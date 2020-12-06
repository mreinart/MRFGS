//
// APRS CWOP Server
//

#include "AprsCwopServer.h"
#include "Configuration.h"

#include <iostream>
#include <math.h>
#include "loguru.hpp"
#include <regex.h>

extern "C" {
#include <telnet.h>
}

using namespace fanet;
using namespace std;

AprsCwopServer *AprsCwopServer::instance_ = nullptr;

void AprsCwopServer::init() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_IF_F(INFO, initialized_, "CWOP already initialized");
    if (!initialized_) {
        Configuration *config = Configuration::getInstance();
        isActive = config->getValue("/configuration/CWOP/active", false);
        stationName = config->getValue("/configuration/stationName", "mFGS");
        server = config->getValue("/configuration/CWOP/server", "glidern4.glidernet.org");
        port = 14580;
        appName = config->getValue("/configuration/CWOP/appName", "mFANgs");
        appVersion = config->getValue("/configuration/CWOP/appVersion", "0.42.17");
        aprsUser = config->getValue("/configuration/CWOP/user", "TTT");
        aprsPass = config->getValue("/configuration/CWOP/pass", "29622");
        LOG_F(INFO, "Init with server %s, port %d, app: %s/%s, user %s",
                server.c_str(), port, appName.c_str(), appVersion.c_str(), aprsUser.c_str());
        string aprsLogFileName = config->getValue("/configuration/CWOP/logfile", "./cwopLog.txt");
        trackFile_.open(aprsLogFileName.c_str(), std::ios_base::app); // append
        trackFile_ << "-- CWOP Log:" << stationName << endl << flush;
        initialized_ = true;
    }
}

// Helper functions -> TODO: move
void encodeAprsAltitude(int alt, char *altStr) {
    // m -> f
    sprintf(altStr, "%06d", alt * 3);
}

void encodeAprsCwopLatitude(float lat, char *latStr) {
	char cHemishpere = {(lat < 0) ? 'S' : 'N'};
	int degrees = floor(lat);
	float minutes = (lat - floor(lat)) * 60;
	sprintf(latStr, "%02d%05.2f%c", degrees, minutes, cHemishpere);
}

void encodeArpsCwopLongitude(float lon, char *lonStr) {
	char cHemishpere = {(lon < 0) ? 'W' : 'E'};
    int degrees = floor(lon);
    float minutes = (lon - floor(lon)) * 60;
	sprintf(lonStr, "%03d%05.2f%c", degrees, minutes, cHemishpere);
}


void AprsCwopServer::encodeWeatherAPRS(WeatherStation *ws, char *aprsStr) {
    //
    // http://www.wxqa.com/faq.html
    //
    // 7. How is the weather data coded into the data packet?
    // When you look at examples of APRS position weather packets here, or here, the part after
    // the longitude "E" or "W" carries the weather data as symbols followed by numbers.
    // - The underscore "_" followed by 3 numbers represents wind direction in degrees from true north. This is the direction that the wind is blowing from.
    // - The slash "/" followed by 3 numbers represents the average wind speed in miles per hour.
    // - The letter "g" followed by 3 numbers represents the peak instaneous value of wind in miles per hour.
    // - The letter "t" followed by 3 characters (numbers and minus sign) represents the temperature in degrees F.
    // - The letter "r" followed by 3 numbers represents the amount of rain in hundredths of inches that fell the past hour.
    // - The letter "p" followed by 3 numbers represents the amount of rain in hundredths of inches that fell in the past 24 hours. Only these two precipitation values are accepted by MADIS.
    // - The letter "P" followed by 3 numbers represents the amount of rain in hundredths of inches that fell since local midnight.
    // - The letter "b" followed by 5 numbers represents the barometric pressure in tenths of a millibar.
    // - The letter "h" followed by 2 numbers represents the relative humidity in percent, where "h00" implies 100% RH.
    //
    // The first four fields (wind direction, wind speed, temperature and gust) are required, in that order,
    // and if a particular measurement is not present, the three numbers should be replaced by "..." to indicate no data available.
    // Solar radiation data can also be coded into the data packet.
    //
    // terminated by cr/lf. The APRS "packet" is a weather position packet with a very specific header:
    //
    // EW9876>APRS,TCPIP*:rest of packet
    //
    // The first "callsign" is the CWOP station identifier for that station as shown.
    // The next item is TCPIP followed by an asterisk. All capital letters must be used for the packet header.
    // The rest of the packet follows the format specified in item#7 above.
    // For example,
    // @060151z3316.04N/09631.96W_120/005g010t021r000p000P000h75b10322
    char result[255];
    memset(result, 0, sizeof(result));
    struct tm *timeinfo = localtime(&ws->lastUpdate);
    if (ws->lastMeasure) {
        timeinfo = localtime(&ws->lastMeasure->timestamp);
        char latStr[15];
        encodeAprsCwopLatitude(ws->latitude, latStr);
        char lonStr[15];
        encodeArpsCwopLongitude(ws->longitude, lonStr);
        char directionStr[4] = "...";
        if (ws->lastMeasure->hasWindDir) {
            unsigned int dir = round(ws->lastMeasure->windDir);
            sprintf(directionStr, "%03d", dir);
        }
        char speedStr[4] = "...";
        if (ws->lastMeasure->hasWindSpeed) {
            unsigned int speed = round(ws->lastMeasure->windSpeed / 1.609344); // in miles per hour
            sprintf(speedStr, "%03d", speed);
        }
        char gustsStr[4] = "...";
        if (ws->lastMeasure->hasWindGusts) {
            unsigned int gusts = round(ws->lastMeasure->windGusts / 1.609344); // in miles per hour
            sprintf(gustsStr, "%03d", gusts);
        }
        char tempStr[4] = "...";
        if (ws->lastMeasure->hasTemperature) {
            int temp = round(ws->lastMeasure->temperature * 9 / 5 + 32);
            sprintf(tempStr, "%03d", temp);
        }
        char humidStr[4] = "...";
        if (ws->lastMeasure->hasHumidity) {
            unsigned int humid = round(ws->lastMeasure->humidity);
            sprintf(humidStr, "%02d", humid);
        }
        char commentStr[30];
        sprintf(commentStr, "%s:%s", ws->name.c_str(), ws->id.c_str());
        // EW9876>APRS,TCPIP*:rest of packet
        // @060151z3316.04N/09631.96W_120/005g010t021r000p000P000h75b10322
        // @hhmmddz<lon>/<lat>_<winddir>/<speed>g<gusts>t<temp>r<>p<>P<>h<humid?>b<baro?>
        sprintf(result, "%s>APRS,TCPIP*:@%.2d%.2d%.2dh%s/%s_%s/%sg%st%sr...p...%s\n",
            ws->cwopId.c_str(), timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
            latStr, lonStr, directionStr, speedStr, gustsStr, tempStr,
            commentStr);
        LOG_F(5, "CWOP-APRS-Packet: %s", result);
    }
    strcat(aprsStr, result);
}

void AprsCwopServer::sendAPRSbuffer(const char *trackBuffer) {
	LOG_SCOPE_FUNCTION(5);
    if (socket_ <= 0) {
        LOG_F(INFO, "Trying to re-connect");
        connect();
    }
    trackFile_ << trackBuffer << flush;
    if (socket_ > 0) {
        LOG_F(5, "sending APRS: \n%s", trackBuffer);
        ssize_t ret = 0;
        ret = telnet_send(socket_, trackBuffer);
        if (ret < 0) {
            LOG_F(WARNING, "telnet_send failed");
            socket_ = 0;
        }
    } else {
        LOG_F(WARNING, "Socket not in working state.");
    }
}

void AprsCwopServer::sendMessage(const char *message) {
	LOG_SCOPE_FUNCTION(5);
	sendAPRSbuffer(message);
}

void AprsCwopServer::sendKeepAlive() {
	LOG_SCOPE_FUNCTION(5);
	sendMessage("# keep alive\n");
}

// http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
// example:
// Cambridge>APRS,TCPIP*,qAC,GLIDERN2:/074555h5212.73NI00007.80E&/A=000066 CPU:4.0 RAM:242.7/458.8MB NTP:0.8ms/-28.6ppm +56.2C RF:+38+2.4ppm/+1.7dB
//
void AprsCwopServer::sendReceiverBeacon(GroundStation *groundStation){
    LOG_SCOPE_FUNCTION(5);
    char groundStationBeacon[100];
    memset(groundStationBeacon, 0, sizeof(groundStationBeacon));
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char latStr[15];
    encodeAprsCwopLatitude(groundStation->latitude(), latStr);
    char lonStr[15];
    encodeArpsCwopLongitude(groundStation->longitude(), lonStr);
    char altStr[7];
    encodeAprsAltitude(groundStation->altitude(), altStr);
    // Cambridge>APRS,TCPIP*,qAC,GLIDERN2:/074555h5212.73NI00007.80E&/A=000066
    sprintf(groundStationBeacon, "%s>APRS,TCPIP*:/%.2d%.2d%.2dh%s/%s&/A=%s\n",
            groundStation->name().c_str(),
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            latStr, lonStr, altStr);
    LOG_F(INFO, "GroundStationBeacon: %s", groundStationBeacon);
    sendAPRSbuffer(groundStationBeacon);
}

boolean AprsCwopServer::isConnected()
{
    LOG_SCOPE_FUNCTION(8);
    return (socket_ > 0);
}

void AprsCwopServer::connect()
{
    LOG_F(INFO, "Connecting to %s port %d", server.c_str(), port);
    socket_ = telnet_connect(server.c_str(), port);
    if(socket_ < 0) {
        LOG_F(ERROR, "Failed to connect to server!");
        return;
    } else {
        char buffer[1024];
        sprintf(buffer, "user %s pass %s vers %s %s \n",
                aprsUser.c_str(), aprsPass.c_str(), appName.c_str(), appVersion.c_str());
        telnet_send(socket_, buffer);
    }
}


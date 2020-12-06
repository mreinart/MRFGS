//
// APRS OGN Server
//

#include "AprsOgnManager.h"
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

AprsOgnManager *AprsOgnManager::instance_ = nullptr;

void AprsOgnManager::init() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_IF_F(INFO, initialized_, "APRSHelper already initialized");
    if (!initialized_) {
        Configuration *config = Configuration::getInstance();
        stationName = config->getValue("/configuration/stationName", "mFGS");
        server = config->getValue("/configuration/APRS/server", "glidern4.glidernet.org");
        port = 14580;
        appName = config->getValue("/configuration/APRS/appName", "mFANgs");
        appVersion = config->getValue("/configuration/APRS/appVersion", "0.42.17");
        aprsUser = config->getValue("/configuration/APRS/user", "TTT");
        aprsPass = config->getValue("/configuration/APRS/pass", "29622");
        LOG_F(INFO, "Init with server %s, port %d, app: %s/%s, user %s",
                server.c_str(), port, appName.c_str(), appVersion.c_str(), aprsUser.c_str());
        string aprsLogFileName = config->getValue("/configuration/APRS/logfile", "./aprsLog.txt");
        trackFile_.open(aprsLogFileName.c_str(), std::ios_base::app); // append
        trackFile_ << "-- APRS Log:" << stationName << endl << flush;
    }
}

// Valid messages
// - see https://github.com/glidernet/ogn-aprs-protocol/blob/master/valid_messages/OGNFNT_Fanet.txt
//# The following beacons are example for FANET (Skytraxx) APRS format
//# source: https://github.com/glidernet/ogn-aprs-protocol
//#
//FNT1103CE>OGNFNT,qAS,FNB1103CE:/183727h5057.94N/00801.00Eg355/002/A=001042 !W10! id1E1103CE +03fpm
//FNT1103CE>OGNFNT,qAS,FNB1103CE:/183729h5057.94N/00801.00Eg354/001/A=001042 !W10! id1E1103CE +07fpm
//FNT1103CE>OGNFNT,qAS,FNB1103CE:/183731h5057.94N/00801.00Eg354/001/A=001042 !W10! id1E1103CE +05fpm
//FNT1103CE>OGNFNT,qAS,FNB1103CE:/183734h5057.94N/00801.00Eg354/001/A=001042 !W30! id1E1103CE -10fpm
//FNT1103CE>OGNFNT,qAS,FNB1103CE:/183736h5057.94N/00801.00Eg354/001/A=001042 !W40! id1E1103CE -02fpm
//FNB1103CE>OGNFNT,TCPIP*,qAC,GLIDERN3:/183738h5057.95NI00801.00E&/A=001042
//
// FNT1106F8>OGNFNT,qAS,FNB1106F8:/141757h3653.70N/00524.94Wg194/000/A=002963 !W03! id1E1106F8 -04fpm
// FNT111AD2>OGNFNT,qAS,FNB1106F8:/141759h3653.83N/00525.35Wg308/010/A=003891 !W49! id1E111AD2 +217fpm

void encodeAltitude(int alt, char *altStr) {
	// m -> f
	sprintf(altStr, "%06d", alt * 3);
}

// http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
//
// example:
// ICA4B4E68>APRS,qAS,Letzi:/152339h4726.50N/00814.20E'260/059/A=002253 !W65! id054B4E68 -395fpm -1.5rot 16.5dB 0e -14.3kHz gps1x2 s6.05 h4C rDF0CD1 +4.5dBm
// !W65!
// is a APRS position precision enhancement. "6" is the third decimal digit of latitude minutes, "5" is the added digit of longitude minutes.

void encodeLatitude(float lat, char *latStr, char *daoLatChr) {
	char cHemishpere = {(lat < 0) ? 'S' : 'N'};
	int degrees = floor(lat);
	float minutes = (lat - floor(lat)) * 60;
	sprintf(latStr, "%02d%06.3f%c", degrees, minutes, cHemishpere);
	daoLatChr[0] = latStr[7];
	daoLatChr[1] = '\0';
	if (daoLatChr[0] == 'N' || daoLatChr[0] == 'S') {
	    LOG_F(ERROR, "DAO-LAT %f %d %f %s", lat, degrees, minutes, latStr);
        daoLatChr[0] = '0';
	}
	sprintf(latStr, "%02d%05.2f%c", degrees, minutes, cHemishpere);
}

void encodeLongitude(float lon, char *lonStr, char *daoLonChr) {
	char cHemishpere = {(lon < 0) ? 'W' : 'E'};
    int degrees = floor(lon);
    float minutes = (lon - floor(lon)) * 60;
	sprintf(lonStr, "%03d%06.3f%c", degrees, minutes, cHemishpere);
	daoLonChr[0] = lonStr[8];
	daoLonChr[1] = '\0';
    if (daoLonChr[0] == 'W' || daoLonChr[0] == 'E') {
        LOG_F(ERROR, "DAO-LAT %f %d %f %s", lon, degrees, minutes, lonStr);
        daoLonChr[0] = '0';
    }
	sprintf(lonStr, "%03d%05.2f%c", degrees, minutes, cHemishpere);
}

void encodeAirCraftType(int acType, int adrType, char *acTypeChr) {
    // see https://github.com/wbuczak/ogn-commons-java/blob/master/src/main/java/org/ogn/commons/beacon/AircraftType.java
    // UNKNOWN(0), GLIDER(1), TOW_PLANE(2), HELICOPTER_ROTORCRAFT(3), PARACHUTE(4), DROP_PLANE(5),
    // HANG_GLIDER(6), PARA_GLIDER(7), POWERED_AIRCRAFT(8), JET_AIRCRAFT(9), UFO(10), BALLOON(11), AIRSHIP(12), UAV(13),
    // STATIC_OBJECT(15);
    //
    // https://www.programcreek.com/java-api-examples/?code=glidernet%2Fogn-commons-java%2Fogn-commons-java-master%2Fsrc%2Fmain%2Fjava%2Forg%2Fogn%2Fcommons%2Fbeacon%2Fdescriptor%2FAircraftDescriptorProvider.java#
    //
    //  RANDOM(0) - changing (random) address generated by the device
    //	ICAO(1)
    //	FLARM(2)  FLARM HW
    //	OGN(3)    OGN tracker HW
    //
    // translate FANET type to OGN type
    int ognType = 0;
    switch (acType) {
        case 0: // Other -> UNKNOWN
            ognType = 0;
            break;
        case 1: // Paraglider
            ognType = 7;
            break;
        case 2: // Hangglider
            ognType = 6;
            break;
        case 3: // Balloon
            ognType = 11;
            break;
        case 4: // Glider
            ognType = 1;
            break;
        case 5: // Aircraft -> POWERED_AIRCRAFT
            ognType = 8;
            break;
        case 6: // Helicopter
            ognType = 3;
            break;
        case 7: // UAV
            ognType = 13;
            break;
    }
    // encode OGN type
    u_int8_t type = (ognType & 0x0F) << 2;
    type += (adrType & 0x03);
    sprintf(acTypeChr, "%02X", type);
}

void encodeSymbolCode(int acType, char *scSstr, char *scTstr) {
    scSstr[0] = 'g';
    scTstr[0] = '/';
    scSstr[1] = '\0';
    scTstr[1] = '\0';
    switch (acType) {
        case 0: // Other -> UNKNOWN
            scSstr[0] = '[';
            scTstr[0] = '/';
            break;
        case 1: // Paraglider
            scSstr[0] = 'g'; // TODO - check in APRS catalog
            scTstr[0] = '/';
            break;
        case 2: // Hangglider
            scSstr[0] = 'g';
            scTstr[0] = '/';
            break;
        case 3: // Balloon
            scSstr[0] = 'O';
            scTstr[0] = '/';
            break;
        case 4: // Glider
            scSstr[0] = '^';
            scTstr[0] = 'G';
            break;
        case 5: // Aircraft -> POWERED_AIRCRAFT
            sprintf(scSstr, "%s", "'");
            scTstr[0] = '/';
            break;
        case 6: // Helicopter
            scSstr[0] = 'X';
            scTstr[0] = '/';
            break;
        case 7: // UAV
            scSstr[0] = '^';
            scTstr[0] = 'D';
            break;
    }
}

void AprsOgnManager::encodeTrackAPRS(Track *track, char *trackStr) {
    // http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
	// TODO - encode varialble parts like FNT type - station name
	char result[1025];
    memset(result, 0, sizeof(result));
	struct tm *timeinfo = localtime(&track->timestamp);
	// FLRxxxxxx>APRS,qAS,GroundStation:112233h4910.17N/00808.42Eg000/000/A=000505 id1Exxxxxx
	// callsign           groundstation time   lat      lon       srs spd   alt    idYY  -
	//
	// 	 YY = STttttaa tttt = AIrCraft type - aa = address typs
	//   1E = 00011100 S=0 T=0 tttt=0111=7=Paraglider aa=00
	//
	char latStr[15];
	char daoLatChr[2];
	encodeLatitude(track->latitude, latStr, daoLatChr);
	char lonStr[15];
	char daoLonChr[2];
	encodeLongitude(track->longitude, lonStr, daoLonChr);
	char altStr[7];
	encodeAltitude(track->altitude, altStr);
	char courseStr[4];
    unsigned int course = round(track->heading);
    sprintf(courseStr, "%03d", course);
    char speedStr[4];
    unsigned int speed = round(track->speed / 1.609344); // in miles per hour
    sprintf(speedStr, "%03d", speed);

    char acTypeStr[3];
    encodeAirCraftType(track->type, 0x02, acTypeStr);  // address type 2 = FLARM HW
    char scSstr[2];
    char scTstr[2];
    encodeSymbolCode(track->type, scSstr, scTstr);
	sprintf(result, "%s%s>APRS,qAS,%s:/%.2d%.2d%.2dh%s%s%s%s%s/%s/A=%s !W%s%s! id%s%s\r\n",
			track->origin,
			track->callsign, this->stationName.c_str(),
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
			latStr, scTstr, lonStr, scSstr,
			courseStr, speedStr, altStr,
			daoLatChr, daoLonChr,
            acTypeStr, track->callsign);
	LOG_F(5, "APRS-Packet: %s", result);
	strcat(trackStr, result);
}

void AprsOgnManager::sendAPRSbuffer(const char *trackBuffer) {
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
            LOG_F(ERROR, "telnet_send failed");
            socket_ = 0;
        }
    } else {
        LOG_F(ERROR, "Socket not in working state.");
    }
}

void AprsOgnManager::sendTrack(Track *track) {
	LOG_SCOPE_FUNCTION(5);
	char aprsTrack[200];
	encodeTrackAPRS(track, aprsTrack);
	sendAPRSbuffer(aprsTrack);
}

void AprsOgnManager::sendMessage(const char *message) {
	LOG_SCOPE_FUNCTION(5);
	sendAPRSbuffer(message);
}

void AprsOgnManager::sendKeepAlive() {
	LOG_SCOPE_FUNCTION(5);
	sendMessage("# keep alive\n");
}

void AprsOgnManager::sendTrackList(list<Track *> trackList) {
	LOG_SCOPE_FUNCTION(INFO);
	if (!trackList.empty()) {
		char aprsBuffer[20000];
        memset(aprsBuffer, 0, sizeof(aprsBuffer));
		for (list<Track *>::iterator ti = trackList.begin(); ti != trackList.end(); ti++) {
			Track *tr = *ti;
			LOG_F(1, "Track: %s", tr->toString().c_str());
			if (tr->tracking) {
                encodeTrackAPRS(tr, aprsBuffer);
			} else {
			    LOG_F(6, " - NOT tracking");
			}
		}
		sendAPRSbuffer(aprsBuffer);
	}
};

// http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
// example:
// Cambridge>APRS,TCPIP*,qAC,GLIDERN2:/074555h5212.73NI00007.80E&/A=000066 CPU:4.0 RAM:242.7/458.8MB NTP:0.8ms/-28.6ppm +56.2C RF:+38+2.4ppm/+1.7dB
//
void AprsOgnManager::sendReceiverBeacon(GroundStation *groundStation){
    LOG_SCOPE_FUNCTION(5);
    char groundStationBeacon[100];
    memset(groundStationBeacon, 0, sizeof(groundStationBeacon));
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char latStr[15];
    char daoLatChr[2];
    encodeLatitude(groundStation->latitude(), latStr, daoLatChr);
    char lonStr[15];
    char daoLonChr[2];
    encodeLongitude(groundStation->longitude(), lonStr, daoLonChr);
    char altStr[7];
    encodeAltitude(groundStation->altitude(), altStr);
    // Cambridge>APRS,TCPIP*,qAC,GLIDERN2:/074555h5212.73NI00007.80E&/A=000066
    //sprintf(groundStationBeacon, "%s>APRS,TCPIP*,qAC,%s:/%.2d%.2d%.2dh%s/%s&/A=%s\n",
    sprintf(groundStationBeacon, "%s>APRS,TCPIP*,qAC,%s:/%.2d%.2d%.2dh%s/%s&/A=%s\n",
            groundStation->name().c_str(),
            groundStation->name().c_str(),
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            latStr, lonStr, altStr);
    LOG_F(INFO, "GroundStationBeacon: %s", groundStationBeacon);
    sendAPRSbuffer(groundStationBeacon);
}


//----- APRS receive and parse

#define MAX_ERROR_MSG 0x1000

// Compile the regular expression described by "regex_text" into "r".
static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
        char error_message[MAX_ERROR_MSG];
        regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                regex_text, error_message);
        return 1;
    }
    return 0;
}

// Match the string in "to_match" against the compiled regular expression in "r".
static int match_regex (regex_t * r, const char * to_match)
{
    // "P" is a pointer into the string which points to the end of the previous match.
    const char * p = to_match;
    // "N_matches" is the maximum number of matches allowed.
    const int n_matches = 10;
    // "M" contains the matches found.
    regmatch_t m[n_matches];

    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            printf ("No more matches.\n");
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) {
                printf ("$& is ");
            }
            else {
                printf ("$%d is ", i);
            }
            printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
                    to_match + start, start, finish);
        }
        p += m[0].rm_eo;
    }
    return 0;
}

static int match_regex (regex_t * r, const char * to_match, char *found)
{
    // "P" is a pointer into the string which points to the end of the previous match.
    const char * p = to_match;
    // "N_matches" is the maximum number of matches allowed.
    const int n_matches = 1;
    // "M" contains the matches found.
    regmatch_t m[n_matches];

    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            strncpy(found, (to_match + start), (finish - start));
            return 0;
        }
        p += m[0].rm_eo;
    }
    return 0;
}

Track *AprsOgnManager::parseAprsTrack(const char *aprsLine) {

    regex_t r;
    regex_t r_aprs_packet;
    regex_t r_aprs_packet_callsign;
    regex_t r_aprs_packet_time;
    regex_t r_aprs_packet_latitude;
    regex_t r_aprs_packet_longitude;
    regex_t r_aprs_packet_crs_spd;
    regex_t r_aprs_packet_altitude;
    regex_t r_aprs_packet_dao;
    regex_t r_aprs_packet_id;
    const char * regex_text;
    const char * regex_text_aprs_packet;
    const char * regex_text_aprs_packet_callsign;
    const char * regex_text_aprs_packet_time;
    const char * regex_text_aprs_packet_latitude;
    const char * regex_text_aprs_packet_longitude;
    const char * regex_text_aprs_packet_crs_spd;
    const char * regex_text_aprs_packet_altitude;
    const char * regex_text_aprs_packet_dao;
    const char * regex_text_aprs_id;

    char aprs_str[132];
    regex_text_aprs_packet = "((ICA|FNT|FLR)[0-9A-F]{6}>.*:/[0-9]{6}h[0-9]{4}\\.[0-9]{2}N.[0-9]{5}\\.[0-9]{2}E.[0-9]{3}/[0-9]{3}/A=[0-9]{6} !W[0-9]{2}! id[0-9A-Fa-f]{2}[0-9A-F]{6})";//
    // ((ICA|FNT|FLR)[0-9A-F]{6}>.*\\\\:/[0-9]{6}h[0-9]{4}\\\\.[0-9]{2}(N|S).[0-9]{5}\\\\.[0-9]{2}(W|E).[0-9]{3}/[0-9]{3}/A=[0-9]{6} !W[0-9]{2}! id[0-9]{2}[0-9A-F]{6} .*)\\\\n";
    compile_regex (&r_aprs_packet, regex_text_aprs_packet);
    int res = match_regex(&r_aprs_packet, aprsLine, aprs_str);
    if (res == 0) {
        printf("- Found APRS position packet\n");
        LOG_F(5, "APRS Position packet found");
    } else {
        LOG_F(9, "Not an APRS Position packet");
        return nullptr;
    }
    regfree (&r_aprs_packet);

    char callsign[11];
    memset(callsign, 0, sizeof(callsign));
    regex_text_aprs_packet_callsign = "((ICA|FNT|FLR)[0-9A-F]{6})";
    compile_regex (&r_aprs_packet_callsign, regex_text_aprs_packet_callsign);
    res = match_regex(&r_aprs_packet_callsign, aprsLine, callsign);
    regfree (&r_aprs_packet_callsign);

    char time_str[10];
    memset(time_str, 0, sizeof(time_str));
    regex_text_aprs_packet_time = "[0-9]{6}h";
    compile_regex (&r_aprs_packet_time, regex_text_aprs_packet_time);
    res = match_regex(&r_aprs_packet_time, aprsLine, time_str);
    regfree (&r_aprs_packet_time);

    char latitude_str[12];
    memset(latitude_str, 0, sizeof(latitude_str));
    regex_text_aprs_packet_latitude = "[0-9]{4}\\.[0-9]{2}(N|S)";
    compile_regex (&r_aprs_packet_latitude, regex_text_aprs_packet_latitude);
    res = match_regex(&r_aprs_packet_latitude, aprsLine, latitude_str);
    regfree (&r_aprs_packet_latitude);

    char longitude_str[12];
    memset(longitude_str, 0, sizeof(longitude_str));
    regex_text_aprs_packet_longitude = "[0-9]{5}\\.[0-9]{2}(W|E)";
    compile_regex (&r_aprs_packet_longitude, regex_text_aprs_packet_longitude);
    res = match_regex(&r_aprs_packet_longitude, aprsLine, longitude_str);
    regfree (&r_aprs_packet_longitude);

    char crs_spd_str[11];
    memset(crs_spd_str, 0, sizeof(crs_spd_str));
    regex_text_aprs_packet_crs_spd = "[0-9]{3}/[0-9]{3}/A=";
    compile_regex (&r_aprs_packet_crs_spd, regex_text_aprs_packet_crs_spd);
    res = match_regex(&r_aprs_packet_crs_spd, aprsLine, crs_spd_str);
    regfree (&r_aprs_packet_crs_spd);

    char altitude_str[10];
    memset(altitude_str, 0, sizeof(altitude_str));
    regex_text_aprs_packet_altitude = "A=[0-9]{6}";
    compile_regex (&r_aprs_packet_altitude, regex_text_aprs_packet_altitude);
    res = match_regex(&r_aprs_packet_altitude, aprsLine, altitude_str);
    regfree (&r_aprs_packet_altitude);

    char dao_ext[10];
    memset(dao_ext, 0, sizeof(dao_ext));
    regex_text_aprs_packet_dao = "!W[0-9]{2}!";
    compile_regex (&r_aprs_packet_dao, regex_text_aprs_packet_dao);
    res = match_regex(&r_aprs_packet_dao, aprsLine, dao_ext);
    regfree (&r_aprs_packet_dao);

    char id_text[5];
    memset(id_text, 0, sizeof(id_text));
    regex_text_aprs_id = "id[0-9]{2}";
    compile_regex (&r_aprs_packet_id, regex_text_aprs_id);
    res = match_regex(&r_aprs_packet_id, aprsLine, id_text);
    regfree (&r_aprs_packet_id);

    unsigned long hex_value = std::strtoul(callsign+3, 0, 16);
    u_int8_t manufacturerId = (hex_value & 0x00ff0000) >> 16;
    u_int16_t uniqueId = hex_value & 0x0000ffff;

    char lat_degrees_str[3];
    memset(lat_degrees_str, 0, sizeof(lat_degrees_str));
    strncpy(lat_degrees_str, latitude_str, 2);
    char lat_minutes_str[7];
    memset(lat_minutes_str, 0, sizeof(lat_minutes_str));
    strncpy(lat_minutes_str, latitude_str+2, 5);
    char lon_degrees_str[4];
    memset(lon_degrees_str, 0, sizeof(lon_degrees_str));
    strncpy(lon_degrees_str, longitude_str, 3);
    char lon_minutes_str[7];
    memset(lon_minutes_str, 0, sizeof(lon_minutes_str));
    strncpy(lon_minutes_str, longitude_str+3, 5);
    if (dao_ext[0] == '!') {
        lat_minutes_str[5] = dao_ext[2];
        lon_minutes_str[5] = dao_ext[3];
    }
    float lat_minutes = atof(lat_minutes_str);
    float lat = atof(lat_degrees_str);
    lat += (lat_minutes * 100 / 60) / 100;
    float lon_minutes = atof(lon_minutes_str);
    float lon = atof(lon_degrees_str);
    lon += (lon_minutes * 100 / 60) / 100;

    char course_str[4];
    strncpy(course_str, crs_spd_str, 3);
    int course = atoi(course_str);
    char speed_str[4];
    strncpy(speed_str, crs_spd_str+4, 3);
    int speed = atoi(speed_str);

    char alt_str[7];
    memset(alt_str, 0, sizeof(alt_str));
    strncpy(alt_str, altitude_str+2, 6);
    int alt = atoi(alt_str) / 3; // ft -> m

    id_text[0] = '0';
    id_text[1] = '0';
    int ognType = atoi(id_text);
    ognType &= 0x3E;
    ognType >>= 2;
    int acType = 0;
    // UNKNOWN(0), GLIDER(1), TOW_PLANE(2), HELICOPTER_ROTORCRAFT(3), PARACHUTE(4), DROP_PLANE(5),
    // HANG_GLIDER(6), PARA_GLIDER(7), POWERED_AIRCRAFT(8), JET_AIRCRAFT(9), UFO(10), BALLOON(11), AIRSHIP(12), UAV(13),
    // STATIC_OBJECT(15);
    switch (ognType) {
        case 0: // Other -> UNKNOWN
            acType = 0;
            break;
        case 1:
            acType = 4;
            break;
        case 2:
            acType = 5;
            break;
        case 3:
            acType = 6;
            break;
        case 4:
            acType = 0;
            break;
        case 5:
            ognType = 5;
            break;
        case 6:
            acType = 2;
            break;
        case 7:
            acType = 1;
            break;
        case 8:
            acType = 5;
            break;
        case 9:
            acType = 5;
            break;
        case 10:
            acType = 0;
            break;
        case 11:
            acType = 3;
            break;
        case 12:
            acType = 3;
            break;
        default:
            acType = 0;
            break;
    }

    time_t t = time(0);
    tm *now = localtime(&t);
    string *timeString = new string(time_str);
    int hrs = atoi(timeString->substr(0, 2).c_str());
    int min = atoi(timeString->substr(2, 2).c_str());
    int sec = atoi(timeString->substr(4, 2).c_str());
    tm trackTime;
    trackTime.tm_year = now->tm_year;
    trackTime.tm_mon = now->tm_mon;
    trackTime.tm_mday = now->tm_mday;
    trackTime.tm_hour = hrs;
    trackTime.tm_min  = min;
    trackTime.tm_sec  = sec;

    // only propagate air tracks
    if (manufacturerId > 0x20) { // Groundstations have 0xD0uuuu etc.
        LOG_F(2, "GroundTrack received: %s", callsign);
        return nullptr;
    }
    AirTrack * track = new AirTrack();
    track->id = callsign;
    strcpy(track->callsign, callsign+3);
    track->timestamp = mktime(&trackTime);
    hex_value = std::strtoul(track->callsign, 0, 16);
    track->manufacturerId = manufacturerId;
    track->uniqueId = uniqueId;
    track->type = acType;
    track->latitude = lat;
    track->longitude = lon;
    track->altitude = alt;
    track->heading = course;
    track->speed = speed;
    LOG_F(1, "AirTrack received: %s", track->toString().c_str());
    return track;
}

boolean AprsOgnManager::isConnected()
{
    LOG_SCOPE_FUNCTION(8);
    return (socket_ > 0);
}

void AprsOgnManager::connect()
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

void AprsOgnManager::recveiverLoop()
{
    LOG_SCOPE_FUNCTION(5);
    char buffer[1024];
    while(telnet_receive(socket_, buffer, 1024))
    {
        if (buffer[0] == '#') {
            LOG_F(9, "APRS comment received: %s", buffer);
        } else {
            LOG_F(5, "APRS received: %s", buffer);
            AirTrack *aprsTrack = (AirTrack*)parseAprsTrack(buffer);
            if (aprsTrack) {
                sendTrack(aprsTrack);
            }
        }
    }
}

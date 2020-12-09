//
// WeatherStation
//

#include "Configuration.h"
#include "WeatherStation.h"
#include "AprsCwopServer.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidcsv.h>
#include <iostream>
#include <string>
#include "loguru.hpp"
#include <httplib.h>

using namespace fanet;
using namespace rapidjson;
using namespace std;

void WeatherStation::init() {
    LOG_SCOPE_FUNCTION(5);
    dfDbActive_ = Configuration::getInstance()->getValue("/configuration/WeatherDB/active", false);
    dbHost_ = Configuration::getInstance()->getValue("/configuration/WeatherDB/updateHost", "www.duddefliecher.de");
    updateUrl_ = Configuration::getInstance()->getValue("/configuration/WeatherDB/updateUrl", "/m/add_fanet_weather.php");
    readUrl_ = Configuration::getInstance()->getValue("/configuration/WeatherDB/readUrl", "/m/get_fanet_weather_csv.php");

    windyActive_ = Configuration::getInstance()->getValue("/configuration/WindyUpdate/active", false);
    windyHost_ = Configuration::getInstance()->getValue("/configuration/WindyUpdate/windyHost", "stations.windy.com");
    windyUpdateUrl_ = Configuration::getInstance()->getValue("/configuration/WindyUpdate/updateUrl","/pws/update/")
            + Configuration::getInstance()->getValue("/configuration/WindyUpdate/updateApiKey","<API-KEY-for-Station>>");

    cwopActive_ = Configuration::getInstance()->getValue("/configuration/CWOP/active", false);
    initialized_ = true;
}

// Push data do DF website via PHP URL
// example: http://www.duddefliecher.de/m/add_fanet_weather.php
//              ?station=FD0003&tms=2020-02-23 18:19:40&direction=123&speed=17.42&temp=22&humid=32&gusts=33.1
//

void WeatherStation::updateDfDb() {
    LOG_SCOPE_FUNCTION(6);
    struct tm *timeinfo;
    timeinfo = localtime(&this->lastMeasure->timestamp);
    char timeBuf[25];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    httplib::SSLClient client(dbHost_.c_str());
    httplib::Headers headers;
    headers.emplace("Accept-Encoding", "gzip, deflate");
    httplib::Params params;
    params.emplace("station", this->id);
    params.emplace("tms", timeBuf);
    params.emplace("received_by", GroundStation::getInstance()->fanetId().c_str());
    char paramStr[25];
    sprintf(paramStr, "%f", this->lastMeasure->windDir);
    params.emplace("direction", paramStr);
    sprintf(paramStr, "%f", this->lastMeasure->windSpeed);
    params.emplace("speed", paramStr);
    sprintf(paramStr, "%f", this->lastMeasure->windGusts);
    params.emplace("gusts", paramStr);
    sprintf(paramStr, "%f", this->lastMeasure->temperature);
    params.emplace("temp", paramStr);
    sprintf(paramStr, "%f", this->lastMeasure->humidity);
    params.emplace("humid", paramStr);
    char uri[255];
    sprintf(uri, "%s?station=%s&tms=%s&received_by=%s&direction=%f&speed=%f&gusts=%f&temp=%f&humid=%f",
            updateUrl_.c_str(), this->id.c_str(), timeBuf, GroundStation::getInstance()->fanetId().c_str(),
            this->lastMeasure->windDir, this->lastMeasure->windSpeed, this->lastMeasure->windGusts,
            this->lastMeasure->temperature, this->lastMeasure->humidity);
    auto res = client.Get(uri, headers);    // Post does not seem to work - while it works from Postman
    //auto res = client.Post(updateUrl_.c_str(), headers, params);    // Post does not seem to work - while it works from Postman
    if (res != nullptr)
        LOG_F(5, "URI: %s - %s", uri, res->body.c_str());
    else
        LOG_F(ERROR, "updating DF-DB with %s", uri);
}

void WeatherStation::readDfDb() {
    LOG_SCOPE_FUNCTION(6);
    try {
        httplib::SSLClient client(dbHost_);
        char request[100];
        sprintf(request, "%s?station=%s", readUrl_.c_str(), this->id.c_str());
        auto res = client.Get(request);
        if (res && res->status == 200) {
            LOG_F(9, "CSV %s", res->body.c_str());
            std::string csvStr = res->body;
            std::stringstream sstream(csvStr);
            rapidcsv::Document doc(sstream);
            WeatherMeasure *wm = new WeatherMeasure();
            wm->weatherStationId = this->id;
            wm->timestamp = doc.GetCell<uint32_t>("tms", this->id);
            wm->windDir = doc.GetCell<float>("direction", this->id);
            wm->hasWindDir = true;
            wm->windSpeed = doc.GetCell<float>("speed", this->id);
            wm->hasWindSpeed = true;
            wm->windGusts = doc.GetCell<float>("gusts", this->id);
            wm->hasWindGusts = true;
            wm->temperature = doc.GetCell<float>("temp", this->id);
            wm->hasTemperature = true;
            wm->humidity = doc.GetCell<float>("humid", this->id);
            wm->hasHumidity = true;
            this->lastMeasure = wm;
            LOG_F(5, "WeatherMeasure: %s", wm->toString().c_str());
        } else {
            LOG_F(9, "no data");
        }
    } catch (const std::exception &e) {
        LOG_F(WARNING, "Issue with update of %s : %s", this->id.c_str(), e.what());
    } catch (...) {
        LOG_F(WARNING, "Issue with update of %s", this->id.c_str());
    }
}

// https://stations.windy.com
//     /pws/update/<API-KEY>?station=1&winddir=123&windspeedmph=11&windgustmph=22&tempf=66&rainin=0
//
const char *windyUpdateUriPatternWindTempHumid = "%s?station=%s&winddir=%f&wind=%f&gust=%f&temp=%f&humidity=%f";
const char *windyUpdateUriPatternWindTemp      = "%s?station=%s&winddir=%f&wind=%f&gust=%f&temp=%f";

void WeatherStation::updateWindy() {
    LOG_SCOPE_FUNCTION(6);
    httplib::SSLClient client(windyHost_.c_str());
    httplib::Headers headers;
    char uri[500];
    memset(uri, 0, 500);
    if (lastMeasure->hasHumidity) {
        sprintf(uri, windyUpdateUriPatternWindTempHumid, windyUpdateUrl_.c_str(),
                windyId.c_str(),
                lastMeasure->windDir,
                (lastMeasure->windSpeed * 5 / 18), // m/s
                (lastMeasure->windGusts * 5 / 18), // m/s
                lastMeasure->temperature,
                lastMeasure->humidity);
    } else {
        sprintf(uri, windyUpdateUriPatternWindTemp, windyUpdateUrl_.c_str(),
                windyId.c_str(),
                lastMeasure->windDir,
                (lastMeasure->windSpeed * 5 / 18), // m/s
                (lastMeasure->windGusts * 5 / 18), // m/s
                lastMeasure->temperature);
    }
    auto res = client.Get(uri, headers);
    LOG_F(9, "%s", res->body.c_str());
}

void WeatherStation::update() {
    LOG_SCOPE_FUNCTION(6);
    if (!initialized_) {
        LOG_F(INFO, "NOT initialized: %s", this->id.c_str());
        return;
    }
    if (this->lastMeasure == nullptr) {
        LOG_F(1, "No Measure %s", this->id.c_str());
        return;
    }
    this->lastUpdate = this->lastMeasure->timestamp;
    // Update weather DB table
    if (dfDbActive_ && this->pushDb) {
        this->updateDfDb();
    }
    // Update Windy service
    if (windyActive_ && this->pushWindy) {
        this->updateWindy();
    }
    // Update APRS CWOP
    if (AprsCwopServer::getInstance()->isActive && this->pushCwop) {
        char aprsMessage[200] = "";
        AprsCwopServer::getInstance()->encodeWeatherAPRS(this, aprsMessage);
        LOG_F(1, "WeatherMeasure-APRS: %s", aprsMessage);
        AprsCwopServer::getInstance()->sendMessage(aprsMessage);
    }
}

void WeatherStation::update(WeatherMeasure *wm) {
    LOG_SCOPE_FUNCTION(5);
    this->update();
    this->lastMeasure = wm;
}

string WeatherStation::toJson() {
	LOG_SCOPE_FUNCTION(7);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);

    writer.StartObject();
    {
        writer.Key("id");
        writer.String(this->id.c_str());
        writer.Key("name");
        writer.String(this->name.c_str());
        writer.Key("shortName");
        writer.String(this->shortName.c_str());
        writer.Key("uniqueId");
        writer.Uint(this->uniqueId);
        writer.Key("manufacturerId");
        writer.Uint(this->manufacturerId);
        writer.Key("latitude");
        writer.Double(this->latitude);
        writer.Key("longitude");
        writer.Double(this->longitude);
        writer.Key("altitude");
        writer.Double(this->altitude);

        if (this->lastMeasure) {
            writer.Key("lastMeasure");
            {
                writer.StartObject();
                writer.Key("windDir");
                writer.Double(this->lastMeasure->windDir);
                writer.Key("windSpeed");
                writer.Double(this->lastMeasure->windSpeed);
                writer.Key("windGusts");
                writer.Double(this->lastMeasure->windGusts);
                writer.Key("temperature");
                writer.Double(this->lastMeasure->temperature);
                writer.EndObject();
            }
        }
    }
    writer.EndObject();

    cout << sb.GetString() << endl;
    return sb.GetString();
}

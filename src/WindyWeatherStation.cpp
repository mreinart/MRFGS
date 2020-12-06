//
// WindyWeatherStation
//

#include "WindyWeatherStation.h"

#include <iostream>
#include "rapidjson/document.h"
#include <httplib.h>
#include "loguru.hpp"

#include "Configuration.h"

using namespace std;
using namespace fanet;
using namespace rapidjson;

void WindyWeatherStation::init() {
    LOG_SCOPE_FUNCTION(6);
    windyHost_ = Configuration::getInstance()->getValue("/configuration/WindyStatus/windyHost", "stations.windy.com");
    windyUrl_ = Configuration::getInstance()->getValue("/configuration/WindyStatus/statusUrl","/pws/update/")
                + Configuration::getInstance()->getValue("/configuration/WindyStatus/statusApiKey","<API-KEY>");
    initialized_ = true;
}

// https://stations.windy.com/pws/station/open/<API-KEY>/<stationId>
// <API-Key> e.g.: ey....ek
// <stationId> - e.g.: f064faa9

std::string WindyWeatherStation::get_windy_station_json(const char *stationId)
{
    LOG_SCOPE_FUNCTION(8);
    httplib::SSLClient client(windyHost_.c_str());
    httplib::Headers headers;
    headers.emplace("Accept-Encoding", "gzip, deflate");
//    httplib::Params params;
    char windyStationUri[200];
    sprintf(windyStationUri, "%s/%s", windyUrl_.c_str(), stationId);
    auto res = client.Get(windyStationUri);//, headers, params);
    if (res && res->status == 200) {
        LOG_F(9, "WindyJSON: %s", res->body.c_str());
        return res->body;
    } else {
        LOG_F(9, "no data");
    }
    return "";
}

void WindyWeatherStation::update() {
	LOG_SCOPE_FUNCTION(6);
    if (!initialized_) {
        LOG_F(ERROR, "NOT initialized: %s", this->id.c_str());
        return;
    }
	this->lastUpdate = time(0);
	try {
		std::string jsonStr = get_windy_station_json(this->id.c_str());
		if (jsonStr.length() == 0) {
			LOG_F(ERROR, "Issue with update of %s - no data", this->id.c_str());
			return;
		}
        Document d;
        d.Parse(jsonStr.c_str());
        WeatherMeasure *wm = new WeatherMeasure();
        wm->weatherStationId = this->id;
        wm->timestamp = time(0);  // toto - get from update array
        const Value& data = d["data"];
        const Value& tmsArry = data["ts"];
        assert(tmsArry.IsArray());
        if (tmsArry[tmsArry.Size()-1].IsNumber()) {
            uint64_t tms_ms = tmsArry[tmsArry.Size() - 1].GetUint64();
            wm->timestamp = tms_ms / 1000;
        }
        const Value& windDirArray = data["windDir"];
        assert(windDirArray.IsArray());
        if (windDirArray[windDirArray.Size()-1].IsNumber()) {
            wm->windDir = windDirArray[windDirArray.Size() - 1].GetFloat();
            wm->hasWindDir = true;
        }
        const Value& windSpeedArray = data["wind"];
        assert(windSpeedArray.IsArray());
        if (windSpeedArray[windSpeedArray.Size()-1].IsNumber()) {
            wm->windSpeed = windSpeedArray[windSpeedArray.Size() - 1].GetFloat();
            wm->hasWindSpeed = true;
        }
        const Value& windGustArray = data["gust"];
        assert(windGustArray.IsArray());
        if (windGustArray[windGustArray.Size()-1].IsNumber()) {
            wm->windGusts = windGustArray[windGustArray.Size() - 1].GetFloat();
            wm->hasWindGusts = true;
        }
        const Value& temperatureArray = data["gust"];
        assert(temperatureArray.IsArray());
        if (temperatureArray[temperatureArray.Size()-1].IsNumber()) {
            wm->temperature = windGustArray[temperatureArray.Size()-1].GetFloat();
            wm->hasTemperature = true;
        }
        this->lastMeasure = wm;
		LOG_F(5, "WeatherMeasure: %s", wm->toString().c_str());
	} catch (const std::exception &e) {
		LOG_F(ERROR, "Issue with update of %s : %s", this->id.c_str(), e.what());
	} catch (...) {
		LOG_F(ERROR, "Issue with update of %s", this->id.c_str());
    }
}

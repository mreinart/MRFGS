//
// WeatherStationManager
//

#include "Configuration.h"
#include "WeatherStationManager.h"
#include "FanetWeatherStation.h"
#include "HolfuyWeatherStation.h"
#include "WindyWeatherStation.h"
#include "AprsCwopServer.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <thread>
#include "loguru.hpp"

using namespace fanet;
using namespace rapidjson;
using namespace std;

WeatherStationManager *WeatherStationManager::instance_ = 0;

void WeatherStationManager::init() {
	LOG_SCOPE_FUNCTION(5);
	Document jsonDoc;
    jsonDoc.Parse(Configuration::getInstance()->getJson().c_str());
    Value *wsArray = Pointer("/configuration/weatherStations").Get(jsonDoc);
    if (wsArray && wsArray->IsArray()) {
        for (auto &wsv : wsArray->GetArray()) {
            if (wsv["active"].GetBool()) {
                WeatherStation *ws;
                if (strcmp(wsv["type"].GetString(), "FanetWeatherStation") == 0) {
                    FanetWeatherStation *fanetWs;
                    fanetWs = new FanetWeatherStation(
                            wsv["id"].GetString(),
                            wsv["name"].GetString(),
                            wsv["shortName"].GetString(),
                            strtol(wsv["manufacturerId"].GetString(), NULL, 16),
                            strtol(wsv["uniqueId"].GetString(), NULL, 16),
                            wsv["latitude"].GetDouble(),
                            wsv["longitude"].GetDouble(),
                            wsv["altitude"].GetDouble());
                    fanetWs->windyId = "";
                    fanetWs->pushWindy = false;
                    if (wsv.HasMember("windyId")) {
                        fanetWs->windyId = wsv["windyId"].GetString();
                        if (wsv.HasMember("windyPush")) {
                            fanetWs->pushWindy = wsv["windyPush"].GetBool();
                        }
                    }
                    fanetWs->pushDb = false;
                    fanetWs->pushDb = wsv["dbPush"].GetBool();
                    fanetWs->pushCwop = false;
                    fanetWs->pushCwop = wsv["cwopPush"].GetBool();
                    fanetWs->cwopId   = wsv["cwopId"].GetString();
                    fanetWs->init();
                    if (fanetWs->pushDb)
                        fanetWeatherStationMap_[fanetWs->id] = fanetWs;
                    else
                        weatherStationMap_[fanetWs->id] = fanetWs;
                } else if (strcmp(wsv["type"].GetString(), "HolfuyWeatherStation") == 0) {
                    ws = new HolfuyWeatherStation(
                            wsv["id"].GetString(),
                            wsv["name"].GetString(),
                            wsv["shortName"].GetString(),
                            strtol(wsv["manufacturerId"].GetString(), NULL, 16),
                            strtol(wsv["uniqueId"].GetString(), NULL, 16),
                            wsv["latitude"].GetDouble(),
                            wsv["longitude"].GetDouble(),
                            wsv["altitude"].GetDouble());
                    ws->pushWindy = false;
                    if (wsv.HasMember("windyId")) {
                        ws->windyId = wsv["windyId"].GetString();
                        if (wsv.HasMember("windyPush")) {
                            ws->pushWindy = wsv["windyPush"].GetBool();
                        }
                    }
                    ws->pushDb = false;
                    ws->pushDb = wsv["dbPush"].GetBool();
                    ws->pushCwop = false;
                    ws->pushCwop = wsv["cwopPush"].GetBool();
                    ws->cwopId   = wsv["cwopId"].GetString();
                    ws->init();
                    weatherStationMap_[ws->id] = ws;
                } else if (strcmp(wsv["type"].GetString(), "WindyWeatherStation") == 0) {
                    ws = new WindyWeatherStation(
                            wsv["id"].GetString(),
                            wsv["name"].GetString(),
                            wsv["shortName"].GetString(),
                            strtol(wsv["manufacturerId"].GetString(), NULL, 16),
                            strtol(wsv["uniqueId"].GetString(), NULL, 16),
                            wsv["latitude"].GetDouble(),
                            wsv["longitude"].GetDouble(),
                            wsv["altitude"].GetDouble());
                    ws->pushWindy = false;
                    ws->pushDb = false;
                    ws->pushDb = wsv["dbPush"].GetBool();
                    ws->pushCwop = false;
                    ws->pushCwop = wsv["cwopPush"].GetBool();
                    ws->cwopId   = wsv["cwopId"].GetString();
                    ws->init();
                    weatherStationMap_[ws->id] = ws;
                } else {
                    ws = new WeatherStation(wsv["id"].GetString());
                    ws->id = wsv["id"].GetString();
                    ws->name = wsv["name"].GetString();
                    ws->shortName = wsv["shortName"].GetString();
                    ws->manufacturerId = strtol(wsv["manufacturerId"].GetString(), NULL, 16);
                    ws->uniqueId = strtol(wsv["uniqueId"].GetString(), NULL, 16);
                    ws->latitude = wsv["latitude"].GetDouble();
                    ws->longitude = wsv["longitude"].GetDouble();
                    ws->altitude = wsv["altitude"].GetDouble();
                    ws->pushWindy = false;
                    ws->pushDb = false;
                    ws->pushDb = wsv["dbPush"].GetBool();
                    ws->pushCwop = false;
                    ws->pushCwop = wsv["cwopPush"].GetBool();
                    ws->cwopId   = wsv["cwopId"].GetString();
                    ws->init();
                    weatherStationMap_[ws->id] = ws;
                }
            }
        }
    }
}

void WeatherStationManager::addFanetWeatherStation(WeatherStation *weatherStation) {
    LOG_F(1, "Adding WeatherStation %s", weatherStation->toJson().c_str());
    weatherStationMap_[weatherStation->id] = weatherStation;
}

void WeatherStationManager::run() {
	VLOG_SCOPE_F(1, "active WeatherStations");
    for (map<string, WeatherStation *>::iterator wsIter = weatherStationMap_.begin();
         wsIter != weatherStationMap_.end(); wsIter++) {
		LOG_F(INFO, " - %s : %s", wsIter->first.c_str(), wsIter->second->name.c_str());
    }
    unsigned int interval = Configuration::getInstance()->getValue("/configuration/features/weatherPolling/interval",
																   120);
	LOG_F(INFO, "updating WeatherStations - interval %d [sec]", interval);
    while (!finished_) {
		VLOG_SCOPE_F(5, "WeatherStationManager - updating WeatherStations");
        for (map<string, WeatherStation *>::iterator iter = weatherStationMap_.begin();
            iter != weatherStationMap_.end(); iter++) {
			LOG_F(2, "updating WS %s: ", iter->first.c_str());
            iter->second->update();
        }
		LOG_F(7, "WeatherStationManager - going to sleep for %d [sec]", interval);
        std::this_thread::sleep_for(chrono::seconds(interval));
    }
	LOG_F(INFO, "WeatherStationManager - END");
}

FanetWeatherStation *WeatherStationManager::getByFanetId(u_int8_t manufacturerId, u_int16_t uniqueId) {
    LOG_F(5, "Get Weatherstation for ID %02X %04X", manufacturerId, uniqueId);
    FanetWeatherStation *fanetWeatherStation = nullptr;
    for (map<string, FanetWeatherStation *>::iterator wsIter = fanetWeatherStationMap_.begin();
         wsIter != fanetWeatherStationMap_.end(); wsIter++) {
        LOG_F(9, " - %s : %s", wsIter->first.c_str(), wsIter->second->name.c_str());
        if ((wsIter->second->manufacturerId == manufacturerId) &&
            (wsIter->second->uniqueId       == uniqueId) ) {
            LOG_F(3, "found %s %s", wsIter->first.c_str(), wsIter->second->name.c_str());
            fanetWeatherStation = wsIter->second;
        }
    }
    return fanetWeatherStation;
}

FanetWeatherStation *WeatherStationManager::getById(string id) {
    LOG_F(5, "Get Weatherstation for ID %s", id.c_str());
    FanetWeatherStation *fanetWeatherStation = nullptr;
    for (map<string, FanetWeatherStation *>::iterator wsIter = fanetWeatherStationMap_.begin();
         wsIter != fanetWeatherStationMap_.end(); wsIter++) {
        LOG_F(9, " - %s : %s", wsIter->first.c_str(), wsIter->second->name.c_str());
        if (wsIter->second->id.compare(id.c_str()) == 0) {
            LOG_F(3, "found %s %s", wsIter->first.c_str(), wsIter->second->name.c_str());
            fanetWeatherStation = wsIter->second;
        }
    }
    return fanetWeatherStation;
}

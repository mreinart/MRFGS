//
// Created by mr on 23.10.19.
//

#ifndef FANET_GS_WEATHERSTATION_H
#define FANET_GS_WEATHERSTATION_H

#include <map>
#include <string>
#include "WeatherMeasure.h"
#include "../include/TDequeConcurrent.h"

using namespace std;

namespace fanet {

    class WeatherStation {
    protected:
        bool initialized_;
        string accessToken;
        string accessUrl;

        bool dfDbActive_;
        std::string dbHost_;
        std::string updateUrl_;
        std::string readUrl_;

        bool windyActive_;
        std::string windyHost_;
        std::string windyUpdateUrl_;

        bool cwopActive_;

    public:
        WeatherStation(string id) : initialized_(false), accessToken(""), accessUrl(""),
                                    id(id), name(""), shortName(""),
                                    manufacturerId(0xFB), uniqueId(0x0000),
                                    latitude(0.0), longitude(0.0), altitude(0.0),
                                    lastUpdate(0), lastMeasure(NULL) {}

        WeatherStation(string id, string name, string shortName)
                : initialized_(false), accessToken(""), accessUrl(""),
                  id(id), name(name), shortName(shortName),
                  manufacturerId(0xFB), uniqueId(0x0000),
                  latitude(0.0), longitude(0.0), altitude(0.0),
                  lastUpdate(0), lastMeasure(NULL) {}

        WeatherStation(string id, string name, string shortName, float latitude, float longitude, float altitude)
                : initialized_(false), accessToken(""), accessUrl(""),
                  id(id), name(name), shortName(shortName),
                  manufacturerId(0xFB), uniqueId(0x0000),
                  latitude(0.0), longitude(0.0), altitude(0.0),
                  lastUpdate(0), lastMeasure(NULL) {}

        WeatherStation(string id, string name, string shortName,
                       u_int8_t manufacturerId, u_int16_t uniqueId,
                       float latitude, float longitude, float altitude)
                : initialized_(false), accessToken(""), accessUrl(""),
                  id(id), name(name), shortName(shortName),
                  manufacturerId(manufacturerId), uniqueId(uniqueId),
                  latitude(latitude), longitude(longitude), altitude(altitude),
                  lastUpdate(0), lastMeasure(NULL) {}

        WeatherStation(string id, string name, string shortName,
                       u_int8_t manufacturerId, u_int16_t uniqueId,
                       string accessToken, string accessUrl,
                       float latitude, float longitude, float altitude)
                : initialized_(false), accessToken(accessToken), accessUrl(accessUrl),
                  id(id), name(name), shortName(shortName),
                  manufacturerId(manufacturerId), uniqueId(uniqueId),
                  latitude(latitude), longitude(longitude), altitude(altitude),
                  lastUpdate(0), lastMeasure(NULL) {}

        string id;
        string name;
        string shortName;
        string cwopId;
        u_int8_t manufacturerId;
        u_int16_t uniqueId;
        float latitude;
        float longitude;
        float altitude;
        time_t lastUpdate;

        bool pushDb;
        bool pushCwop;
        bool pushWindy;
        string windyId;

        WeatherMeasure *lastMeasure;

        string toAprsString();

        virtual string toJson();

        virtual void init();
        virtual void update();
        virtual void update(WeatherMeasure *wm);
        virtual void updateDfDb();
        virtual void readDfDb();
        virtual void updateWindy();
    };

}; // namespace fanet

#endif //FANET_GS_WEATHERSTATION_H

//
// Created by mr on 2020-04-26
//

#ifndef FANET_GS_WEATHERSTATIONMGR_H
#define FANET_GS_WEATHERSTATIONMGR_H

#include <map>
#include <string>
#include "WeatherMeasure.h"
#include "../include/TDequeConcurrent.h"
#include "WeatherStation.h"
#include "FanetWeatherStation.h"

using namespace std;

namespace fanet {

    class WeatherStationManager {
    private:
        static WeatherStationManager *instance_;
        bool initialized_;
        bool finished_;
        map<string, WeatherStation *> weatherStationMap_;
        map<string, FanetWeatherStation *> fanetWeatherStationMap_;

        WeatherStationManager() : initialized_(false), finished_(false), weatherStations(&weatherStationMap_) {};

    public:

        static WeatherStationManager *getInstance() {
            if (instance_ == 0) {
                instance_ = new WeatherStationManager();
            }
            return instance_;
        }

//        TDequeConcurrent<WeatherMeasure> weatherMeasureQueue;
        TDequeConcurrent<WeatherMeasure*> fanetWeatherQueue;

        void init();

        void run();
        void stop() { finished_ = true; };
        map<string, WeatherStation *>* weatherStations;
        map<string, WeatherStation *>* fanetWeatherStations;
        FanetWeatherStation *getByFanetId(u_int8_t manufacturerId, u_int16_t uniqueId);
        FanetWeatherStation *getById(string id);
        void addFanetWeatherStation(WeatherStation *weatherStation);
    };

}; // namespace fanet

#endif //FANET_GS_WEATHERSTATIONMGR_H

//
// WindyWeatherStation
//

#ifndef FANET_GS_WindyWeatherStation_H
#define FANET_GS_WindyWeatherStation_H

#include "WeatherStation.h"

#include <utility>

namespace fanet {

    class WindyWeatherStation : public WeatherStation {

        std::string windyHost_;
        std::string windyUrl_;

        std::string get_windy_station_json(const char *stationId);

    public:
        WindyWeatherStation(string id, string name, string shortName,
                         u_int8_t manufacturerId, u_int16_t uniqueId,
                         string accessToken, string accessUrl,
                         float latitude, float longitude, float altitude)
                : WeatherStation(id, name, shortName, manufacturerId, uniqueId,
                                 accessToken, accessUrl, latitude, longitude, altitude) {}

        WindyWeatherStation(string id, string name, string shortName,
                         u_int8_t manufacturerId, u_int16_t uniqueId,
                         float latitude, float longitude, float altitude)
                : WeatherStation(id, name, shortName, manufacturerId, uniqueId,
                                 "", "", latitude, longitude, altitude) {}

        void init();
        void update();
    };

} // namespace fanet

#endif //FANET_GS_WindyWeatherStation_H

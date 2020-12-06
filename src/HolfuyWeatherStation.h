//
// HolfuyWeatherStation
//

#ifndef FANET_GS_HOLFUYWEATHERSTATION_H
#define FANET_GS_HOLFUYWEATHERSTATION_H

#include "WeatherStation.h"

namespace fanet {
    class HolfuyWeatherStation : public WeatherStation {

        std::string get_holfuy_json(const char *holfuy_id);

    public:
        HolfuyWeatherStation(string id, string name, string shortName,
                             u_int8_t manufacturerId, u_int16_t uniqueId,
                             float latitude, float longitude, float altitude)
                : WeatherStation(id, name, shortName, manufacturerId, uniqueId,
                                 latitude, longitude, altitude) {};

        void update();
    };

} // namespace fanet

#endif //FANET_GS_HOLFUYWEATHERSTATION_H

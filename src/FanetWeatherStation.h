//
// FanetWeatherStation
//

#ifndef FANET_GS_FANETWEATHERSTATION_H
#define FANET_GS_FANETWEATHERSTATION_H

#include "WeatherStation.h"
#include "Configuration.h"

namespace fanet {
    class FanetWeatherStation : public WeatherStation {

    public:
        FanetWeatherStation(string id, string name, string shortName,
                             u_int8_t manufacturerId, u_int16_t uniqueId,
                             float latitude, float longitude, float altitude)
                : WeatherStation(id, name, shortName, manufacturerId, uniqueId,
                                 latitude, longitude, altitude) {}

        void update();
    };

} // namespace fanet

#endif //FANET_GS_FANETWEATHERSTATION_H

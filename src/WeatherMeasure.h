//
// WeatherMeasure
//

#ifndef FANET_GS_WEATHERMEASURE_H
#define FANET_GS_WEATHERMEASURE_H

#include <string>

using namespace std;
namespace fanet {

    class WeatherMeasure {
    public:
        WeatherMeasure()
                : weatherStationId(""), timestamp(0),
                  manufacturerId(0xFD), uniqueId(0x0000),
                  windDir(0.0), windSpeed(0.0), windGusts(0.0),
                  temperature(0.0), humidity(0.0),
                  hasTemperature(false), hasHumidity(false),
                  hasWindDir(false), hasWindSpeed(false), hasWindGusts(false) {}

        WeatherMeasure(string weatherStationId, time_t timestamp,
                       u_int8_t manufacturerId, u_int16_t uniqueId,
                       float windDir, float windSpeed, float windGusts,
                       float temperature, float humidity)
                : weatherStationId(weatherStationId), timestamp(timestamp),
                  manufacturerId(manufacturerId), uniqueId(uniqueId),
                  windDir(windDir), windSpeed(windSpeed), windGusts(windGusts),
                  temperature(temperature), humidity(humidity),
                  hasTemperature(true), hasHumidity(true),
                  hasWindDir(true), hasWindSpeed(true), hasWindGusts(true) {}

        string weatherStationId;
        time_t timestamp;
        u_int8_t manufacturerId;
        u_int16_t uniqueId;
        float windDir;
        float windSpeed;
        float windGusts;
        float temperature;
        float humidity;

        bool hasTemperature;
        bool hasHumidity;
        bool hasWindDir;
        bool hasWindSpeed;
        bool hasWindGusts;

        string toString();
        string toJson();

        static WeatherMeasure fromRtl433Type145Json(string rtl433Type145Json);
        static WeatherMeasure fromRtl433Type145Json(const char* rtl433Type145Json);

        };

} // namespace fanet

#endif //FANET_GS_WEATHERMEASURE_H

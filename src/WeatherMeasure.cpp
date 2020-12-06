//
// WeatherMeasure
//

#include "WeatherMeasure.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <sstream>

using namespace fanet;
using namespace std;
using namespace rapidjson;

string WeatherMeasure::toString() {
	struct tm *timeinfo;
	timeinfo = localtime(&this->timestamp);
	char timeBuf[80];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
	stringstream ss;
	ss << "Weather: "
	   << this->weatherStationId
	   << "/" << timeBuf << "(" << this->timestamp << ")"
	   << "/" << this->windDir
	   << "/" << this->windSpeed
	   << "/" << this->windGusts;
	return string(ss.str());
}

string WeatherMeasure::toJson() {
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();
    {
        writer.Key("ws-id");
        writer.String(this->weatherStationId.c_str());
        writer.Key("timestamp");
        writer.Int(this->timestamp);
        writer.Key("windDir");
        writer.Double(this->windDir);
        writer.Key("windSpeed");
        writer.Double(this->windSpeed);
        writer.Key("windGusts");
        writer.Double(this->windGusts);
        writer.Key("temperature");
        writer.Double(this->temperature);
    }
    writer.EndObject();
    cout << sb.GetString() << endl;
    return sb.GetString();
}

/*
 {"time" : "2020-01-30 19:52:11",
 "model" : "WS2032",
 "id" : 43501,
 "temperature_C" : 7.700,
 "humidity" : 84,
 "direction_deg" : 180.000,
 "wind_avg_km_h" : 4.644,
 "wind_max_km_h" : 12.384,
 "maybe_flags" : 0,
 "maybe_rain" : 256,
 "mic" : "CRC"}
 */

WeatherMeasure WeatherMeasure::fromRtl433Type145Json(string rtl433Type145Json) {
    return fromRtl433Type145Json(rtl433Type145Json.c_str());
}

WeatherMeasure WeatherMeasure::fromRtl433Type145Json(const char* rtl433Type145Json) {
    Document d;
    d.Parse(rtl433Type145Json);
    WeatherMeasure wm;
    Value &value = d["id"];
    char weatherStationId[20];
    sprintf(weatherStationId, "%04X", value.GetInt());
    wm.weatherStationId = weatherStationId;
    tm tm{};
    value = d["time"];
    string str = value.GetString();
    strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    wm.timestamp = mktime(&tm);
    value = d["temperature_C"];
    wm.temperature = value.GetFloat();
    wm.hasTemperature = true;
    value = d["humidity"];
    wm.humidity = value.GetFloat();
    wm.hasHumidity = true;
    value = d["direction_deg"];
    wm.windDir = value.GetFloat();
    wm.hasWindDir = true;
    value = d["wind_avg_km_h"];
    wm.windSpeed = value.GetFloat();
    wm.hasWindSpeed = true;
    value = d["wind_max_km_h"];
    wm.windGusts = value.GetFloat();
    wm.hasWindGusts = true;
    return wm;
}

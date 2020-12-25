//
// HolfuyWeatherStation
//

#include "HolfuyWeatherStation.h"
#include "AprsCwopServer.h"

#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "loguru.hpp"

using namespace std;
using namespace fanet;
using namespace rapidjson;

#include <httplib.h>

const char *holfuyHost     = "holfuy.com";
const char *holfuyPugetUri = "/puget/mjso.php?k=%s";

std::string HolfuyWeatherStation::get_holfuy_json(const char *holfuy_id)
{
	LOG_SCOPE_FUNCTION(8);
	httplib::SSLClient client(holfuyHost);
	httplib::Headers headers;
	headers.emplace("Accept-Encoding", "gzip, deflate");
    char uri[100];
    sprintf(uri, holfuyPugetUri, holfuy_id);
	auto res = client.Get(uri, headers);
	if (res && res->status == 200) {
		LOG_F(9, "CSV %s", res->body.c_str());
		return res->body;
	} else {
		LOG_F(9, "no data");
	}
    return "";
}

void HolfuyWeatherStation::update() {
	LOG_SCOPE_FUNCTION(6);
    if (!initialized_) {
        LOG_F(ERROR, "NOT initialized: %s", this->id.c_str());
        return;
    }
	this->lastUpdate = time(0);
	time_t now = time(0);
	struct tm *tm = localtime(&now);

	std::string jsonStr = get_holfuy_json(this->id.c_str());
	if (jsonStr.length() == 0) {
		LOG_F(ERROR, "Issue with update of %s - no data", this->id.c_str());
		return;
	}
	try {
        if ((jsonStr.find("no_conn") == string::npos) && (jsonStr.find("low_batt") == string::npos)) {
			Document d;
			d.Parse(jsonStr.c_str());
			WeatherMeasure *wm = new WeatherMeasure();
			wm->weatherStationId = this->id;
			wm->timestamp = time(0);
			Value &floatValue = d["dir"];
			wm->windDir = floatValue.GetFloat();
			wm->hasWindDir = true;
			floatValue = d["speed"];
			wm->windSpeed = floatValue.GetFloat();
            wm->hasWindSpeed = true;
			floatValue = d["gust"];
			wm->windGusts = floatValue.GetFloat();
            wm->hasWindGusts = true;
			floatValue = d["temperature"];
			wm->temperature = floatValue.GetFloat();
			wm->hasTemperature = true;
			this->lastMeasure = wm;
			LOG_F(5, "WeatherMeasure: %s", wm->toString().c_str());
		}
	} catch (const std::exception &e) {
		LOG_F(ERROR, "Issue with update of %s : %s", this->id.c_str(), e.what());
	} catch (...) {
		LOG_F(ERROR, "Issue with update of %s", this->id.c_str());
	}
	WeatherStation::update();
}

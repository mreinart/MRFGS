//
// FanetWeatherStation
//

#include "FanetWeatherStation.h"

#include "loguru.hpp"

using namespace std;
using namespace fanet;

void FanetWeatherStation::update() {
	LOG_SCOPE_FUNCTION(6);
	if (!this->pushDb) {
	    readDfDb();
	}
	if (!lastMeasure) {
        LOG_F(9, "No measure yet for WeatherStation %s", this->id.c_str());
        return;
    }
	WeatherStation::update();
    if (this->pushDb) {
        this->lastMeasure = nullptr;
    }
}

/*
 * FANET Ground Station - Intrenet-Access-Only flavor
 *
 *
 */

#include <sys/fcntl.h>
#include <syslog.h>
#include <unistd.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/pointer.h>
#include <iostream>
#include <string>

#include <list>
#include <map>
#include "../include/TDequeConcurrent.h"

#include "AprsOgnManager.h"
#include "Configuration.h"
#include "WeatherMeasure.h"
#include "WeatherStation.h"
#include "WeatherStationManager.h"
#include "FanetWeatherStation.h"
#include "Packet.h"
#include "Track.h"
#include "EntityName.h"
#include "Landmark.h"
#include "GroundStation.h"
#include "Message.h"

#include <chrono>
#include "../include/httplib.h"
#include <curl/curl.h>
#include "loguru.hpp"
#include "tgbot/tgbot.h"

extern "C" {
#include "fanet_GS/fanet_struct.h"
#include "fanet_GS/fanet_mac.h"
#include "fanet_GS/fanet_terminal.h"
#include "fanet_GS/fanet_t0_ack.h"
#include "fanet_GS/fanet_t1_tracking.h"
#include "fanet_GS/fanet_t2_name.h"
#include "fanet_GS/fanet_t3_messenger.h"
#include "fanet_GS/fanet_t4_service.h"
#include "fanet_GS/fanet_t5_landmark.h"
#include "fanet_GS/fanet_t7_tracking.h"
#include "fanet_GS/fanet_t8_hwinfo.h"
#include "fanet_GS/fanet_t9_thermal.h"
}

#include <CLI/CLI.hpp>

using namespace fanet;
using namespace rapidjson;
using namespace httplib;

bool finished = false;

//---------------------------------------------------------------------

void weatherStationManagerRunner (int pauseSecs) {
    loguru::set_thread_name("Weather-get");
    LOG_F(INFO, "WeatherStationManager - RUN");
    WeatherStationManager::getInstance()->init();
    WeatherStationManager::getInstance()->run();
    LOG_F(INFO, "WeatherStationManager - END");
};

void groundStationManagerRunner(int pauseSecs) {
    loguru::set_thread_name("GS-RUN");
	LOG_F(INFO, "GroundStationManager - RUN");
 	GroundStation::getInstance()->run();
	LOG_F(INFO, "GroundStationManager - END");
};

const int sll[] = { LOG_NOTICE, LOG_NOTICE, LOG_NOTICE, LOG_INFO, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG };

void log_to_rsyslog(void*, const loguru::Message& message)
{
    int syslog_level = sll[message.verbosity];
    syslog (syslog_level, "%s - %d - %s", message.filename, message.line, message.message);
}

int main(int argc, char *argv[]) {

    CLI::App app("FANET Groundstation");
    // add version output
    app.set_version_flag("--version", string("0.0.1"));

    string configFile = "./config/MRFGS.json";
    CLI::Option *send_opt = app.add_option("-c,--config", configFile, "Configuration file - default: './config/MRFGS.json'");

    CLI11_PARSE(app, argc, argv);

    cout << "-- ino FANET Groundstation --" << endl;
	loguru::init(argc, argv);
    loguru::add_callback("rsyslog", log_to_rsyslog, nullptr, loguru::Verbosity_MAX);

    loguru::add_file("./log/everything.log", loguru::Truncate, 7);
	char log_path[PATH_MAX];
	loguru::suggest_log_path("./log/", log_path, sizeof(log_path));
	loguru::add_file(log_path, loguru::Truncate, loguru::Verbosity_INFO);

	LOG_F(2, "FANET Groundstation");

	Configuration *config = Configuration::getInstance();
	config->init(configFile);
	LOG_F(INFO, "FANET Groundstation: %s", Configuration::getInstance()->getStringValue("/configuration/stationName").c_str());

    int power = Configuration::getInstance()->getValue("/configuration/features/fanetRadio/power", 15);

    Message startMsg;
    startMsg.initFromText(Configuration::getInstance()->getStringValue("/configuration/stationId"),
                          "FFFFFF", false,
                          "FANET-GS " + Configuration::getInstance()->getStringValue("/configuration/stationName") + " - Start");

    AprsOgnManager::getInstance()->connect();

    std::thread threadWeatherStationManager(weatherStationManagerRunner, 60);

    EntityNameManager::getInstance()->init();

    std::thread groundStationManagerThread(groundStationManagerRunner, 300);

    groundStationManagerThread.join();

	std::cout << "finished!" << std::endl;

	return 0;
}

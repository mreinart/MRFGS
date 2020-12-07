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
//#include "fanet_GS/fanet_radio.h"
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

void forwardTracksToInternet(bool doTrackPush, bool pushToAprs, bool pushToKtrax, bool pushToTelegram) {
    VLOG_SCOPE_F(1, "TrackConsumer - trying to consume...");
    TDequeConcurrent<Track*> *trackQueuePtr = &GroundStation::getInstance()->trackQueue;
    list<Track*> trackList;
    if (!trackQueuePtr->empty()) {
        LOG_SCOPE_FUNCTION(1);
        while (!trackQueuePtr->empty()) {
            auto tr = trackQueuePtr->pop_front();
            LOG_F(INFO, "APRS - consuming (%lu) : %s", trackQueuePtr->size(), tr->toString().c_str());
            GroundStation::getInstance()->addTrack(tr);
            trackList.emplace_back(tr);
        }
        if (doTrackPush && pushToAprs) {
            LOG_F(INFO, "TrackConsumer - push to APRS");
            AprsOgnManager::getInstance()->sendTrackList(trackList);
        } else {
            LOG_F(INFO, "TrackConsumer - APRS - NOT sending");
        }
    } else {
        LOG_F(INFO, "TrackConsumer - Nothing to send");
    }
}

void queuesConsumer() {
    LOG_F(INFO, "queuesConsumer - Start");
    loguru::set_thread_name("Queue-Cons");

    bool doTrackPush = Configuration::getInstance()->getValue("/configuration/features/trackPushing/active", false);
    LOG_IF_F(INFO, doTrackPush, "Feature: Push Tracks");
    bool doTrackPushAprs = Configuration::getInstance()->getValue("/configuration/APRS/active", false);
    LOG_IF_F(INFO, doTrackPushAprs, "Feature: Push Tracks to APRS");
    bool doTrackPushKtrax = Configuration::getInstance()->getValue("/configuration/KTRAX/active", false);
    LOG_IF_F(INFO, doTrackPushKtrax, "Feature: Push Tracks to KTRAX");
    bool doTrackPushTelegram = Configuration::getInstance()->getValue("/configuration/Telegram/active", false);
    LOG_IF_F(INFO, doTrackPushTelegram, "Feature: Push Tracks to Telegram");
    int intervalTrackPush = Configuration::getInstance()->getValue("/configuration/features/trackPushing/interval", 5000);

    bool doWeatherPush = Configuration::getInstance()->getValue("/configuration/features/weatherPushing/active", false);
    int weatherPushInterval = Configuration::getInstance()->getValue("/configuration/features/weatherPushing/interval", 120);
    int pauseSecsBetweenStations = Configuration::getInstance()->getValue("/configuration/features/weatherPushing/pause", 5);
    int maxAgeSeconds = Configuration::getInstance()->getValue("/configuration/features/weatherPushing/maxAge", 300);

    TDequeConcurrent<Packet*>* packetQueuePtr = &GroundStation::getInstance()->packetQueue;
    TDequeConcurrent<EntityName*> *nameQueuePtr = &GroundStation::getInstance()->nameQueue;

    LOG_F(INFO, "WeatherMeasureConsumer - RUN - interval %d / %d [sec]", weatherPushInterval, pauseSecsBetweenStations);
    while (!finished) {
        LOG_F(1, "Pushing Packets to MQTT and Telegram");
        if (!packetQueuePtr->empty()) {
            while (!packetQueuePtr->empty()) {
                auto packet = packetQueuePtr->pop_front();
                LOG_F(1, "Packet - consuming (%lu) : %s", packetQueuePtr->size(), packet->toString().c_str());
                GroundStation::getInstance()->sendPacketToDfDb(packet);
            }
        }
        LOG_F(1, "Pushing Names to DB");
        if (!nameQueuePtr->empty()) {
            while (!nameQueuePtr->empty()) {
                auto nameEntity = nameQueuePtr->pop_front();
                LOG_F(1, "Name - consuming (%lu) : %s", nameQueuePtr->size(), nameEntity->toString().c_str());
                Device *device = new Device(nameEntity->id);
                device->name = nameEntity->name;
                device->manufacturerId = nameEntity->manufacturerId;
                device->uniqueId = nameEntity->uniqueId;
                device->timestamp = time(0);
                GroundStation::getInstance()->sendDeviceToDfDb(device);
            }
        }
        LOG_F(1, "Pushing FANET track data to Inet");
        forwardTracksToInternet(doTrackPush, doTrackPushAprs, doTrackPushKtrax, doTrackPushTelegram);
        LOG_F(7, "TrackConsumer - going to sleep for %d [msec]", intervalTrackPush);
        std::this_thread::sleep_for(chrono::milliseconds(intervalTrackPush));

    } // while
    LOG_F(INFO, "queuesConsumer - END");
}

void weatherStationManagerRunner (int pauseSecs) {
    loguru::set_thread_name("Weather-get");
    LOG_F(INFO, "WeatherStationManager - RUN");
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
	loguru::g_stderr_verbosity = 3;
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
    // Initialize FANET base mechanisms

    Message startMsg;
    startMsg.initFromText(Configuration::getInstance()->getStringValue("/configuration/stationId"),
                          "FFFFFF", false,
                          "FANET-GS " + Configuration::getInstance()->getStringValue("/configuration/stationName") + " - Start");

    AprsOgnManager::getInstance()->connect();

    WeatherStationManager::getInstance()->init();
    std::thread threadWeatherStationManager(weatherStationManagerRunner, 60);

    std::thread threadQueuesConsumer1(queuesConsumer);

    EntityNameManager::getInstance()->init();

    std::thread groundStationManagerThread(groundStationManagerRunner, 300);

    groundStationManagerThread.join();

	std::cout << "finished!" << std::endl;

	return 0;
}

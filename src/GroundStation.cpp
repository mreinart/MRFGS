//
// GroundStation
//

#include "GroundStation.h"
#include "AprsOgnManager.h"
#include "Configuration.h"
#include "Packet.h"
//TTT#include "Vehicle.h"

#include "loguru.hpp"
#include <sstream>
#include <thread>
#include "time.h"

#include <rapidjson/pointer.h>

using namespace fanet;
using namespace std;

#include <httplib.h>

extern "C" {
#include "fanet_GS/fanet_struct.h"
#include "fanet_GS/fanet_radio.h"
#include "fanet_GS/fanet_mac.h"
#include "fanet_GS/fanet_global.h"
#include "fanet_GS/fanet_terminal.h"
#include "fanet_GS/fanet_t0_ack.h"
#include "fanet_GS/fanet_t1_tracking.h"
#include "fanet_GS/fanet_t2_name.h"
#include "fanet_GS/fanet_t3_messenger.h"
#include "fanet_GS/fanet_t4_service.h"
#include "fanet_GS/fanet_t7_tracking.h"
}

// Push data do website via PHP URL
//
void GroundStation::sendTrackToDfDb(Track *track) {
    LOG_SCOPE_FUNCTION(6);
    struct tm *timeinfo;
    timeinfo = localtime(&track->timestamp);
    char timeBuf[25];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    httplib::SSLClient client(trackDbHost_.c_str());
    httplib::Headers headers;
    headers.emplace("Accept-Encoding", "gzip, deflate");
    httplib::Params params;
    char paramStr[25];
    sprintf(paramStr, "%s", track->callsign);
    params.emplace("device", paramStr);
    params.emplace("tms", timeBuf);
    sprintf(paramStr, "%s", this->fanetId().c_str());
    params.emplace("received_by", paramStr);
    sprintf(paramStr, "%d", track->type);
    params.emplace("type", paramStr);
    sprintf(paramStr, "%f", track->latitude);
    params.emplace("lat", paramStr);
    sprintf(paramStr, "%f", track->longitude);
    params.emplace("lon", paramStr);
    sprintf(paramStr, "%d", track->altitude);
    params.emplace("alt", paramStr);
    sprintf(paramStr, "%f", track->heading);
    params.emplace("course", paramStr);
    sprintf(paramStr, "%f", track->speed);
    params.emplace("speed", paramStr);
    sprintf(paramStr, "%2.1f", track->climbRate);
    params.emplace("vspd", paramStr);
    sprintf(paramStr, "%2.1f", track->turnRate);
    params.emplace("trate", paramStr);
    sprintf(paramStr, "%2.1f", track->distance);
    params.emplace("dist", paramStr);
    sprintf(paramStr, "%d", track->tracking);
    params.emplace("track", paramStr);
    char uri[255];
    //?device=110B0D&tms=2020-03-13 20:06:44&received_by=FD7777&course=321&speed=42&vspd=22&trate=88&lat=49.21&lon=8.321&alt=333&type=7
    sprintf(uri, "%s?device=%s&tms=%s&received_by=%s&course=%f&speed=%f&vspd=%2.1f&trate=%2.1f&lat=%f&lon=%f1&alt=%d&dist=%f&type=%d&track=%d",
            updateUrlTrack_.c_str(), track->callsign, timeBuf, this->fanetId().c_str(),
            track->heading, track->speed, track->climbRate, track->turnRate,
            track->latitude, track->longitude, track->altitude, track->distance, track->type, track->tracking);
    LOG_F(5, "Pushing track of device %s to DF-DB from %s", track->callsign, this->fanetId().c_str());
    auto res = client.Get(uri, headers);
    // Post oddly not working from here - while it is from Postman
    if (res != nullptr)
        LOG_F(9, "%s", res->body.c_str());
}

// Push data do DF website via PHP URL
// example: http://www.duddefliecher.de/m/add_fanet_device.php
//              ?station=FD0003&tms=2020-02-23 18:19:40&name=
void GroundStation::sendDeviceToDfDb(Device *device) {
    LOG_SCOPE_FUNCTION(6);
    struct tm *timeinfo;
    time_t tms = device->timestamp;
    timeinfo = localtime(&tms);
    char timeBuf[25];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    httplib::SSLClient client(trackDbHost_.c_str());
    httplib::Headers headers;
    headers.emplace("Accept-Encoding", "gzip, deflate");
    httplib::Params params;
    char paramStr[25];
    sprintf(paramStr, "%s", device->id.c_str());
    params.emplace("device", paramStr);
    params.emplace("tms", timeBuf);
    sprintf(paramStr, "%s", this->fanetId().c_str());
    params.emplace("received_by", paramStr);
    sprintf(paramStr, "%d", device->type);
    params.emplace("type", paramStr);
    sprintf(paramStr, "%s", device->name.c_str());
    params.emplace("name", paramStr);
    char uri[255];
    //?device=110B0D&tms=2020-03-13 20:06:44&received_by=FD7777&name=ABCD&type=7
    sprintf(uri, "%s?device=%s&tms=%s&received_by=%s&type=%d&name=%s",
            updateUrlDevice_.c_str(), device->id.c_str(), timeBuf, this->fanetId().c_str(), device->type, device->name.c_str());
    LOG_F(5, "Pushing Name of device %s %s to DF-DB from %s", device->id.c_str(), device->name.c_str(), this->fanetId().c_str());
    auto res = client.Get(uri, headers);
    // Post oddly not working from here - while it is from Postman
    if (res != nullptr)
        LOG_F(9, "%s", res->body.c_str());
}

void GroundStation::sendPacketToDfDb(Packet *packet) {
    LOG_SCOPE_FUNCTION(6);
    if (!pushPackets_) {
        return;
    }
    struct tm *timeinfo;
    time_t tms = packet->radioData.timestamp;
    timeinfo = localtime(&tms);
    char timeBuf[25];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    httplib::SSLClient client(packetDbHost_.c_str());
    httplib::Headers headers;
    headers.emplace("Accept-Encoding", "gzip, deflate");
    httplib::Params params;
    char paramStr[25];
    char devIdStr[10];
    sprintf(devIdStr, "%02X%04X", packet->fanetMAC.s_manufactur_id, packet->fanetMAC.s_unique_id);
    params.emplace("device", devIdStr);
    params.emplace("tms", timeBuf);
    sprintf(paramStr, "%s", this->fanetId().c_str());
    params.emplace("received_by", paramStr);
    sprintf(paramStr, "%d", packet->fanetMAC.type);
    params.emplace("type", paramStr);
    sprintf(paramStr, "%s", "ABCD");//packet->toJson().c_str());
    params.emplace("hexstr", paramStr);
    char uri[10000];
    //?device=110B0D&tms=2020-03-13 20:06:44&received_by=FD7777&hexstr={<JSON>}&type=x
    sprintf(uri, "%s?device=%s&tms=%s&received_by=%s&type=%d&hexstr=%s",
            updateUrlPacket_.c_str(), devIdStr, timeBuf, this->fanetId().c_str(), packet->fanetMAC.type, packet->toJson().c_str());
    LOG_F(5, "Pushing packet from %s to DF-DB from %s", devIdStr, this->fanetId().c_str());
    auto res = client.Get(uri, headers);
    // Post oddly not working from here - while it is from Postman
    if (res != nullptr)
        LOG_F(5, "%s", res->body.c_str());
}

GroundStation *GroundStation::instance_ = nullptr;

void GroundStation::init() {
	LOG_SCOPE_FUNCTION(INFO);
    if (!initialized_) {
        initialized_ = true;
        Configuration *config = Configuration::getInstance();
        fanetId_ = config->getValue("/configuration/stationId", "UNK_ID");
        manufacturerId = strtol(config->getValue("/configuration/manufacturerId", "0xFC").c_str(), NULL, 16);
        uniqueId = strtol(config->getValue("/configuration/uniqueId", "0x0042").c_str(), NULL, 16);
        name_ = config->getValue("/configuration/stationName", "UNK");

        lat_ = config->getValue("/configuration/position/latitude", (float) 42.42);
        lon_ = config->getValue("/configuration/position/longitude", (float) 17.17);
        alt_ = config->getValue("/configuration/position/altitude", 123);
        LOG_F(INFO, "FANET Groundstation: %s - Position: %f / %f / %d", name_.c_str(), lat_, lon_, alt_);
        set_position(lat_, lon_, double(alt_));

        sendHwInfo = config->getValue("/configuration/hwinfo/active", false);
        fwInfo.deviceType = config->getValue("/configuration/hwinfo/type", 0x42);
        fwInfo.year = config->getValue("/configuration/hwinfo/year", 2020);
        fwInfo.month = config->getValue("/configuration/hwinfo/month", 4);
        fwInfo.day = config->getValue("/configuration/hwinfo/day", 1);
        fwInfo.add1 = config->getValue("/configuration/hwinfo/add1", 0x98);
        fwInfo.add2 = config->getValue("/configuration/hwinfo/add2", 0x76);
        LOG_F(INFO, "FANET Groundstation - hwinfo: %04d-%02d-%02d", fwInfo.year, fwInfo.month, fwInfo.day);
        fwInfo.experimental = config->getValue("/configuration/hwinfo/experimental", true);
        LOG_IF_F(INFO, fwInfo.experimental, "-- experimental");

        pushTracks_ = Configuration::getInstance()->getValue("/configuration/features/trackPushing/active", false);
        pushTracks_ = pushTracks_ &&
                      Configuration::getInstance()->getValue("/configuration/TrackDB/active", false);
        trackDbHost_ = Configuration::getInstance()->getValue("/configuration/TrackDB/updateHost",
                                                              "www.duddefliecher.de");
        updateUrlTrack_ = Configuration::getInstance()->getValue("/configuration/TrackDB/updateUrlTrack",
                                                            "/m/add_fanet_track.php");
        updateUrlDevice_ = Configuration::getInstance()->getValue("/configuration/TrackDB/updateUrlDevice",
                                                                 "/m/add_fanet_device.php");
        LOG_IF_F(INFO, pushTracks_, "-- Pushing tracks");
        pushPackets_ = Configuration::getInstance()->getValue("/configuration/PacketDB/active", false);
        packetDbHost_ = Configuration::getInstance()->getValue("/configuration/PacketDB/updateHost",
                                                              "www.duddefliecher.de");
        updateUrlPacket_ = Configuration::getInstance()->getValue("/configuration/PacketDB/updateUrlPacket",
                                                                  "/m/add_fanet_packet.php");
        LOG_IF_F(INFO, pushPackets_, "-- Pushing packets");
    } else {
        LOG_F(INFO, "FANET Groundstation already initialized");
    }
}

string GroundStation::fanetId() {
    return fanetId_;
}

string GroundStation::name() {
    return name_;
}

void GroundStation::addName(EntityName *entityName) {
    LOG_F(5, "addName %s", entityName->toString().c_str());
    int count = deviceMap.count(entityName->id);
    if (deviceMap.count(entityName->id) > 0) {
        deviceMap.at(entityName->id)->name = entityName->name;
    } else {
        LOG_F(INFO, "New Device %s", entityName->id.c_str());
        Device *newDevice = new Device(entityName->id);
        newDevice->manufacturerId = entityName->manufacturerId;
        newDevice->uniqueId = entityName->uniqueId;
        newDevice->timestamp = time(0);
        //type
        newDevice->firstPosition = nullptr;
        newDevice->lastPosition = nullptr;
        deviceMap.insert(pair<string, Device *>(newDevice->id, newDevice));
    }
}

void GroundStation::addTrack(Track *track) {
	LOG_F(5, "addTrack %s", track->id.c_str());
	int count = deviceMap.count(track->callsign);
	if (deviceMap.count(track->callsign) > 0) {
		deviceMap.at(track->callsign)->lastPosition = track;
	} else {
		LOG_F(INFO, "New Device %s", track->callsign);
		Device *newDevice = new Device(track->callsign);
        newDevice->timestamp = time(0);
        newDevice->firstPosition = track;
		newDevice->lastPosition = track;
		deviceMap.insert(pair<string, Device *>(newDevice->id, newDevice));
	}
    if (pushTracks_) {
        LOG_F(6, "Pushing track;");
        sendTrackToDfDb(track);
    }
}

string GroundStation::getStatusInfo() {
	LOG_SCOPE_FUNCTION(INFO);
	stringstream ss;
	ss << "Groundstation "
	   << name_ << endl
	   << " #Devices: " << deviceMap.size() << endl;
	int i = 0;
	for (map<string, Device *>::iterator iter = deviceMap.begin(); iter != deviceMap.end(); iter++) {
		ss << "-- device[" << ++i << "] - ID: " << iter->second->id << " / " << iter->second->name << endl;
		ss << "  - first position : " << iter->second->firstPosition->toString() << endl;
		ss << "  - last position  : " << iter->second->lastPosition->toString() << endl;
	}
	ss << "--" << endl;
	return string(ss.str());
}

void GroundStation::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Groundstation %s - RUN", name_.c_str());
    unsigned int interval = Configuration::getInstance()->getValue(
            "/configuration/features/summaryPushing/interval", 300);
    while (!finished_) {
        this->runIteration();
        this_thread::sleep_for(chrono::seconds(interval));
    }
    LOG_F(INFO, "Groundstation - END");
}

void GroundStation::runIteration() {
    LOG_SCOPE_FUNCTION(7);
    AprsOgnManager::getInstance()->sendReceiverBeacon(this);
}





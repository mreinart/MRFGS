//
// FANET GroundStation
//

#include "FanetGroundStation.h"
#include "AprsOgnManager.h"
#include "Configuration.h"
#include "Packet.h"
#include "Vehicle.h"

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

void FanetGroundStation::init() {
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
        LOG_F(INFO, "FANET Groundstation - hwinfo: %04d-02d-%02d", fwInfo.year, fwInfo.month, fwInfo.day);
        fwInfo.experimental = config->getValue("/configuration/hwinfo/experimental", true);
        LOG_IF_F(INFO, fwInfo.experimental, "-- experimental");

        pushTracks_ = Configuration::getInstance()->getValue("/configuration/features/trackPushing/active", false);
        pushTracks_ = pushTracks_ &&
                      Configuration::getInstance()->getValue("/configuration/TrackDB/active", false);
        relayTracksLegacy2Fanet_ = Configuration::getInstance()->getValue(
                "/configuration/features/trackRelaying/active", false);
        relayTracksExcludeFanetPlus_ = Configuration::getInstance()->getValue(
                "/configuration/features/trackRelaying/excludeFanetPlus", true);
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

        Document jsonDoc;
        jsonDoc.Parse(Configuration::getInstance()->getJson().c_str());

        Value *vehArray = Pointer("/configuration/vehicles").Get(jsonDoc);
        if (vehArray && vehArray->IsArray()) {
            for (auto &veh : vehArray->GetArray()) {
                if (veh["active"].GetBool()) {
                    Vehicle *vehicle;
                    vehicle = new Vehicle(veh["id"].GetString());
                    vehicle->name = veh["name"].GetString();
                    vehicle->manufacturerId = strtol(veh["manufacturerId"].GetString(), NULL, 16);
                    vehicle->uniqueId = strtol(veh["uniqueId"].GetString(), NULL, 16);
                    vehicle->type = veh["type"].GetInt();
                    vehicle->pullDb = false;
                    vehicle->pullDb = veh["dbPull"].GetBool();
                    vehicle->maxAgeSeconds = veh["maxAgeSeconds"].GetInt();
                    this->vehicleMap[vehicle->id] = vehicle;
                    vehicle->init();
                }
            }
        }
    } else {
        LOG_F(INFO, "FANET Groundstation already initialized");
    }
}

void FanetGroundStation::sendTrackToFANET(Track *track) {
    LOG_F(INFO, "Send Track to FANET: %s", track->toString().c_str());
    AirTrack *airTrack = dynamic_cast<AirTrack*>(track);
    if (airTrack != NULL) {
        sendAirTrackToFANET(airTrack);
    }
    GroundTrack *gndTrack = dynamic_cast<GroundTrack*>(track);
    if (gndTrack != NULL) {
        sendGndTrackToFANET(gndTrack);
    }
}

void FanetGroundStation::sendAirTrackToFANET(AirTrack *track) {
    LOG_F(INFO, "Send AirTrack to FANET: %s", track->toString().c_str());
    std::unique_lock<std::mutex> lock{radioMutex};

    sRadioData _radiodata;
    sFanetMAC _fanet_mac;
    sAirTracking _tx_tracking;
    sRawMessage _tx_message;

    _tx_tracking.aircraft_type = track->type;
    _tx_tracking.longitude = track->longitude;
    _tx_tracking.latitude = track->latitude;
    _tx_tracking.altitude = track->altitude;

    _tx_tracking.heading = track->heading;
    _tx_tracking.speed = track->speed;

    _tx_tracking.climb = 0; //track->climbRate;
    _tx_tracking.turn_rate = 0;
    _tx_tracking.turn_rate_on = false;

    _tx_message.m_length = 0;
    type_1_tracking_coder(&_tx_message, &_tx_tracking);

    _fanet_mac.type = 1;    // Air Tracking
    _fanet_mac.s_manufactur_id = track->manufacturerId;
    _fanet_mac.s_unique_id = track->uniqueId;
    _fanet_mac.e_header = 0;
    _fanet_mac.forward = 0;
    _fanet_mac.ack = 0;
    _fanet_mac.cast = 0;
    _fanet_mac.signature_bit = 0;

    fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_message);
    write_tx_data(&_radiodata, &_tx_message);

    lock.unlock();

    sAirTracking _rx_tracking;
    type_1_tracking_decoder(&_tx_message, &_fanet_mac, &_rx_tracking);
    terminal_message_1(true, false, &_radiodata, &_fanet_mac, &_rx_tracking);
}

void FanetGroundStation::sendGndTrackToFANET(GroundTrack *track) {
    LOG_F(INFO, "Send GndTrack to FANET: %s", track->toString().c_str());
    std::unique_lock<std::mutex> lock{radioMutex};

    sRadioData _radiodata;
    sFanetMAC _fanet_mac;
    sGroundTracking _tx_tracking;
    sRawMessage _tx_message;

    _tx_tracking.ground_type = track->type;
    _tx_tracking.longitude = track->longitude;
    _tx_tracking.latitude = track->latitude;

    _tx_message.m_length = 0;
    type_7_tracking_coder(&_tx_message, &_tx_tracking);

    _fanet_mac.type = 7;    // Ground Tracking
    _fanet_mac.s_manufactur_id = track->manufacturerId;
    _fanet_mac.s_unique_id = track->uniqueId;
    _fanet_mac.e_header = 0;
    _fanet_mac.forward = 1;
    _fanet_mac.ack = 0;
    _fanet_mac.cast = 0;
    _fanet_mac.signature_bit = 0;

    fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_message);
    write_tx_data(&_radiodata, &_tx_message);

    lock.unlock();

    sGroundTracking _rx_tracking;
    type_7_tracking_decoder(&_tx_message, &_fanet_mac, &_rx_tracking);
    terminal_message_7(true, false, &_radiodata, &_fanet_mac, &_rx_tracking);
}

void FanetGroundStation::addTrack(Track *track) {
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
    if (relayTracksLegacy2Fanet_) {
        if (track->manufacturerId != 0x11) {
            LOG_F(6, "Relay LEG track to FANET");
            sendTrackToFANET(track);
        } else {
            if (!relayTracksExcludeFanetPlus_) {
                LOG_F(6, "Relaying FANET+LEG track to FANET");
                sendTrackToFANET(track);
            } else {
                // Do not relay FANET+ devices which are sending LEG and FANET in parallel
            }
        }
    }
}

void FanetGroundStation::run() {
	LOG_SCOPE_FUNCTION(INFO);
	LOG_F(INFO, "FanetGroundstation %s - RUN", name_.c_str());
    AprsOgnManager *arpsHelper = AprsOgnManager::getInstance();
    unsigned int interval = Configuration::getInstance()->getValue(
            "/configuration/features/summaryPushing/interval", 300);
	while (!finished_) {
        arpsHelper->sendReceiverBeacon(this);
		this_thread::sleep_for(chrono::seconds(interval));
	}
	LOG_F(INFO, "FanetGroundstation - END");
}





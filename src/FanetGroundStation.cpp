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
	GroundStation::init();
    if (!initializedFanet_) {
        initializedFanet_ = true;
        Configuration *config = Configuration::getInstance();
        relayTracksLegacy2Fanet_ = Configuration::getInstance()->getValue(
                "/configuration/features/trackRelaying/active", false);
        relayTracksExcludeFanetPlus_ = Configuration::getInstance()->getValue(
                "/configuration/features/trackRelaying/excludeFanetPlus", true);

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
    LOG_SCOPE_FUNCTION(5);
    GroundStation::addTrack(track);
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

void FanetGroundStation::runIteration() {
    LOG_SCOPE_FUNCTION(5);
    GroundStation::runIteration();
    AprsOgnManager::getInstance()->sendReceiverBeacon(this);
}




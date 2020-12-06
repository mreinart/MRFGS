//
// Landmark / LandmarkManager
//

#include "Landmark.h"

#include "Configuration.h"
#include "GroundStation.h"
#include "loguru.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <algorithm>

extern "C" {
#include "fanet_GS/fanet_struct.h"
#include "fanet_GS/fanet_radio.h"
#include "fanet_GS/fanet_mac.h"
#include "fanet_GS/fanet_terminal.h"
#include "fanet_GS/fanet_t5_landmark.h"
}

#include "../include/TDequeConcurrent.h"

using namespace std;
using namespace fanet;
using namespace rapidjson;

//extern std::mutex _mutexRadio;

LandmarkManager *LandmarkManager::instance_ = 0;

string Landmark::toString() {
    stringstream ss;
    ss << "ID: "
       << this->id
       << "/type: "
       << this->type
       << " - " << this->text
       << "/C:" << this->coordinates.size();
    list<fanet::Coordinate2D>::iterator iter;
    for (iter = this->coordinates.begin(); iter != this->coordinates.end(); iter++) {
        ss << "(" << iter->lat << "/" << iter->lon << ")";
    }
    ss << "/D:" << this->diameter;
    return string(ss.str());
}

void encode_abs_coord_2_buffer(char _buffer[], int _bufptr, float _latitude, float _longitude) {
    signed int _lat_int;
    signed int _lon_int;
    if (_latitude > 90)
        _latitude = 0;
    if (_latitude < -90)
        _latitude = 0;
    if (_longitude > 180)
        _longitude = 0;
    if (_longitude < -180)
        _longitude = 0;
    _lat_int = round(_latitude * 93206);
    _lon_int = round(_longitude * 46603);

    _buffer[_bufptr + 0] = (_lat_int & 0x000000FF);
    _buffer[_bufptr + 1] = (_lat_int & 0x0000FF00) >> 8;
    _buffer[_bufptr + 2] = (_lat_int & 0x00FF0000) >> 16;

    _buffer[_bufptr + 3] = (_lon_int & 0x000000FF);
    _buffer[_bufptr + 4] = (_lon_int & 0x0000FF00) >> 8;
    _buffer[_bufptr + 5] = (_lon_int & 0x00FF0000) >> 16;
}

uint16_t fns_coord2buf_compressed(float ref_deg) {
    const float deg_round = roundf(ref_deg);
    const bool deg_odd = ((int)deg_round) & 1;
    const float decimal = ref_deg - deg_round;
    int dec_int = (int)(decimal*32767.0f);
    auto clamped_value = std::min( std::max(dec_int, -16383), 16383 );
    return ((clamped_value & 0x7FFF) | (!!deg_odd<<15));
}

void encode_rel_coord_2_buffer(char _buffer[], int _bufptr, float _latitude, float _longitude) {
    uint16_t _delta_lat = fns_coord2buf_compressed(_latitude);
    _buffer[_bufptr + 0] = (_delta_lat & 0x000000FF);
    _buffer[_bufptr + 1] = (_delta_lat & 0x0000FF00) >> 8;

    uint16_t _delta_lon = fns_coord2buf_compressed(_longitude);
    _buffer[_bufptr + 2] = (_delta_lon & 0x000000FF);
    _buffer[_bufptr + 3] = (_delta_lon & 0x0000FF00) >> 8;
}

void Landmark::encode_raw_message(sRawMessage *_rawMessage) {
    LOG_SCOPE_FUNCTION(6);
    memset(_rawMessage, 0, sizeof(sRawMessage));
    _rawMessage->message[_rawMessage->m_length++] = ((this->timeToLive & 0x0F) << 4) | (this->type & 0x0F);
    _rawMessage->message[_rawMessage->m_length++] = this->layer & 0x0F;
    if (this->windDep) {
        _rawMessage->message[_rawMessage->m_length-1] |= 0x10; // set bit 4 in Byte 1
        _rawMessage->message[_rawMessage->m_length++] = this->windBits; // fill wind in byte 2
    }
    if (this->type == 0) {
        encode_abs_coord_2_buffer(_rawMessage->message, _rawMessage->m_length, this->coordinates.begin()->lat, this->coordinates.begin()->lon);
        _rawMessage->m_length += 6;
        sprintf(_rawMessage->message+_rawMessage->m_length, "%s", this->text.c_str());
        _rawMessage->m_length += strlen(this->text.c_str());
    } else if (((this->type >= 1) && (this->type <= 4)) || (this->type == 8)) {
        if (this->type == 8) {
            // Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once)
            int8_t alt_min = this->altMin;//((altMin / 25) - 109);
            int8_t alt_max = this->altMax;//((altMax / 25) - 109);
            _rawMessage->message[_rawMessage->m_length++] = alt_min; //alt_min & 0xFF;
            _rawMessage->message[_rawMessage->m_length++] = alt_max; //alt_max & 0xFF;
        }
        list<fanet::Coordinate2D>::iterator coord;
        for (coord = this->coordinates.begin(); coord != this->coordinates.end(); coord++) {
            if (coord == this->coordinates.begin()) {
                encode_abs_coord_2_buffer(_rawMessage->message, _rawMessage->m_length, coord->lat, coord->lon);
                _rawMessage->m_length += 6;
            } else {
                encode_rel_coord_2_buffer(_rawMessage->message, _rawMessage->m_length, coord->lat, coord->lon);
                _rawMessage->m_length += 4;
            }
        }
    } else if ((this->type == 5) || (this->type == 6) || (this->type == 9)) {
        encode_abs_coord_2_buffer(_rawMessage->message, _rawMessage->m_length, this->coordinates.begin()->lat, this->coordinates.begin()->lon);
        _rawMessage->m_length += 6;
        unsigned int dia = this->diameter / 50;
        if (dia > 0x7F) { // 7 Bits for diameter
            _rawMessage->message[_rawMessage->m_length] = 0x80 + dia / 8; // bit 7=1: 8x diameter x 50m
        } else {
            _rawMessage->message[_rawMessage->m_length] = dia; // bit 7=0: 1x diameter x 50m
        }
        _rawMessage->m_length += 1;
        if (this->type == 9) {
            // Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once)
            int8_t alt_min = this->altMin;//((altMin / 25) - 109);
            int8_t alt_max = this->altMax;//((altMax / 25) - 109);
            _rawMessage->message[_rawMessage->m_length++] = alt_min; //alt_min & 0xFF;
            _rawMessage->message[_rawMessage->m_length++] = alt_max; //alt_max & 0xFF;
        }
    }
    _rawMessage->m_pointer = _rawMessage->m_length;
}

void LandmarkManager::sendRawMessage(sRawMessage *_tx_message) {
    LOG_SCOPE_FUNCTION(5);
    sRadioData _radiodata;
    sFanetMAC _fanet_mac;
    sLandmark _tx_landmark;

    _fanet_mac.type = 5;    // Landmark
    _fanet_mac.s_manufactur_id = GroundStation::getInstance()->manufacturerId;
    _fanet_mac.s_unique_id = GroundStation::getInstance()->uniqueId;
    _fanet_mac.e_header = 0;
    _fanet_mac.forward = 0;
    _fanet_mac.ack = 0;
    _fanet_mac.cast = 0;
    _fanet_mac.signature_bit = 0;

    std::unique_lock<std::mutex> lock{GroundStation::getInstance()->radioMutex};
    terminal_message_raw(0, 0, &_radiodata, &_fanet_mac, _tx_message);
    fanet_mac_coder(&_radiodata, &_fanet_mac, _tx_message);
    write_tx_data(&_radiodata, _tx_message);
//    terminal_message_raw(0, 0, &_radiodata, &_fanet_mac, _tx_message);
    lock.unlock();
//    sLandmark _rx_landmark;
//    type_5_landmark_decoder(_tx_message, &_rx_landmark);
//    terminal_message_raw(0, 0, &_radiodata, &_fanet_mac, _tx_message);
//    terminal_message_5(true, false, &_radiodata, &_fanet_mac, &_rx_landmark);
}

void LandmarkManager::sendLandmark(Landmark *landmark) {
    LOG_SCOPE_FUNCTION(5);

    if (landmark->type == 0) {
        sRawMessage _tx_message0;
        memset(_tx_message0.message, 0, sizeof(_tx_message0.message));
        landmark->encode_raw_message(&_tx_message0);
        sendRawMessage(&_tx_message0);
    } else {
        sRawMessage _tx_messageX;
        memset(_tx_messageX.message, 0, sizeof(_tx_messageX.message));
        landmark->encode_raw_message(&_tx_messageX);
        sendRawMessage(&_tx_messageX);
        if (landmark->text.length() > 0) {
            sRawMessage _tx_message0;
            memset(_tx_message0.message, 0, sizeof(_tx_message0.message));
            int save_type = landmark->type;
            landmark->type = 0;
            landmark->encode_raw_message(&_tx_message0);
            sendRawMessage(&_tx_message0);
            landmark->type = save_type;
        }
    }
}

void LandmarkManager::reinit() {
    LOG_SCOPE_FUNCTION(INFO);
    initialized_ = false;
    init();
}

void LandmarkManager::init() {
    LOG_SCOPE_FUNCTION(INFO);
    if (!initialized_){
        LOG_F(INFO, "LandmarkManager - getting Landmarks");
        Document jsonDoc;
        jsonDoc.Parse(Configuration::getInstance()->getJson().c_str());

        Value *wsArray = Pointer("/configuration/landmarks").Get(jsonDoc);
        if (wsArray->IsArray()) {
            for (auto &wsv : wsArray->GetArray()) {
                if (wsv["active"].GetBool()) {
                    Landmark landmark = Landmark();
                    landmark.type = wsv["type"].GetUint();
                    if (wsv.HasMember("layer")) {
                        landmark.layer = wsv["layer"].GetUint();
                    }
                    if (wsv.HasMember("timeToLive")) {
                        landmark.timeToLive = wsv["timeToLive"].GetUint();
                    }
                    landmark.id = wsv["id"].GetString();
                    landmark.text = wsv["text"].GetString();
                    if (wsv.HasMember("coordinates")) {
                        for (auto &coord : wsv["coordinates"].GetArray()) {
                            Coordinate2D c2d;
                            c2d.lat = coord["lat"].GetDouble();
                            c2d.lon = coord["lon"].GetDouble();
                            landmark.coordinates.emplace_back(c2d);
                        }
                    }
                    if (wsv.HasMember("geometry")) {
                        auto &geometryMembers = wsv["geometry"];
                        auto &type = geometryMembers["type"];
                        string typeStr = type.GetString();
                        if (geometryMembers.HasMember("coordinates")) {
                            auto &coordinates = geometryMembers["coordinates"];
                            if (0 == typeStr.compare("Point")) {
                                Coordinate2D c2d;
                                c2d.lon = coordinates[0].GetDouble();
                                c2d.lat = coordinates[1].GetDouble();
                                landmark.coordinates.emplace_back(c2d);
                            } else if (0 == typeStr.compare("LineString")) {
                                for (auto &coord : coordinates.GetArray()) {
                                    Coordinate2D c2d;
                                    c2d.lon = coord[0].GetDouble();
                                    c2d.lat = coord[1].GetDouble();
                                    landmark.coordinates.emplace_back(c2d);
                                }
                            } else { // Polygon
                                for (auto &coord : coordinates.GetArray()) {
                                    for (auto &c : coord.GetArray()) {
                                        Coordinate2D c2d;
                                        c2d.lon = c[0].GetDouble();
                                        c2d.lat = c[1].GetDouble();
                                        landmark.coordinates.emplace_back(c2d);
                                    }
                                }
                            }
                        }
                    }
                    if (wsv.HasMember("windDepend")) {
                        landmark.windDep = wsv["windDepend"].GetBool();
                    }
                    if (wsv.HasMember("windSectors")) {
                        landmark.windBits = 0x00;
                        auto &windSectors = wsv["windSectors"];
                        for (auto &ws : windSectors.GetArray()) {
                            string windSector = ws.GetString();
                            if (0 == windSector.compare("N")) {
                                landmark.windBits |= 0x01;
                            } else if (0 == windSector.compare("NE")) {
                                landmark.windBits |= 0x02;
                            } else if (0 == windSector.compare("E")) {
                                landmark.windBits |= 0x04;
                            } else if (0 == windSector.compare("SE")) {
                                landmark.windBits |= 0x08;
                            } else if (0 == windSector.compare("S")) {
                                landmark.windBits |= 0x10;
                            } else if (0 == windSector.compare("SW")) {
                                landmark.windBits |= 0x20;
                            } else if (0 == windSector.compare("W")) {
                                landmark.windBits |= 0x40;
                            } else if (0 == windSector.compare("NW")) {
                                landmark.windBits |= 0x80;
                            } else {
                                LOG_F(ERROR, "Unknown Windsector code: '%s'", windSector.c_str());
                            }
                        }
                    }
                    if (wsv.HasMember("windBits")) {
                        landmark.windBits = wsv["windBits"].GetInt();
                    }
                    if (wsv.HasMember("diameter")) {
                        landmark.diameter = wsv["diameter"].GetUint();
                    }
                    if (wsv.HasMember("altMin")) {
                        landmark.altMin = wsv["altMin"].GetInt();
                    }
                    if (wsv.HasMember("altMax")) {
                        landmark.altMax = wsv["altMax"].GetInt();
                    }
                    landmarkList.emplace_back(landmark);
                    LOG_F(INFO, "%s", landmark.toString().c_str());
                }
            }
        }
        LOG_F(INFO, "# names: %lu", landmarkList.size());
        initialized_ = true;
    } else {
        LOG_F(INFO, "already initialized");
    }
}

void LandmarkManager::run() {
    LOG_F(INFO, "LandmarkManager - RUN");
    while (!finished_) {
        unsigned int interval = Configuration::getInstance()->getValue(
                "/configuration/features/landmarkPushing/interval", 600);
        unsigned int pauseSecsBetweenLandmarks = Configuration::getInstance()->getValue(
                "/configuration/features/landmarkPushing/pause", 10);
        list<fanet::Landmark>::iterator iter;// = EntityNameManager::getInstance()->nameList;
        for (iter = LandmarkManager::getInstance()->landmarkList.begin();
             iter != LandmarkManager::getInstance()->landmarkList.end(); iter++) {
            LOG_F(1, "Landmark: %s", iter->toString().c_str());
            Landmark *lmPtr = &(*iter);
            sendLandmark(lmPtr);
            std::this_thread::yield();
            LOG_F(7, "LandmarkManager - going to sleep for %d [sec]", pauseSecsBetweenLandmarks);
            std::this_thread::sleep_for(chrono::seconds(pauseSecsBetweenLandmarks));
        }
        this_thread::sleep_for(chrono::seconds(interval));
    }
    LOG_F(INFO, "LandmarkManager - END");
}


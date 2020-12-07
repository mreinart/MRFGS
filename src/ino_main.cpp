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

void receiveFanetWeather(
        sRadioData radiodata,
        sFanetMAC fanet_mac,
        sWeather rx_weather_data) {
    LOG_SCOPE_FUNCTION(5);
    LOG_F(1, "Received Weather %02X%04X - Wind: %f %f %f",
            fanet_mac.s_manufactur_id,
            fanet_mac.s_unique_id,
            rx_weather_data.wind_heading,
            rx_weather_data.wind_speed,
            rx_weather_data.wind_gusts);

    WeatherMeasure *weatherMeasure = new WeatherMeasure();
    char wsId[15];
    sprintf(wsId, "%02X%04X", fanet_mac.s_manufactur_id, fanet_mac.s_unique_id);
    weatherMeasure->weatherStationId = wsId;
    weatherMeasure->timestamp = rx_weather_data.time;
    weatherMeasure->manufacturerId = fanet_mac.s_manufactur_id;
    weatherMeasure->uniqueId = fanet_mac.s_unique_id;
    weatherMeasure->hasWindDir = true;
    weatherMeasure->windDir = rx_weather_data.wind_heading;
    weatherMeasure->hasWindSpeed = true;
    weatherMeasure->windSpeed = rx_weather_data.wind_speed;
    weatherMeasure->windGusts = rx_weather_data.wind_gusts;
    if (weatherMeasure->windGusts > 0.0) {
        weatherMeasure->hasWindGusts = true;
    }
    weatherMeasure->temperature = rx_weather_data.temperature;
    if (weatherMeasure->temperature > 0.0) {
        weatherMeasure->hasTemperature = true;
    }
    weatherMeasure->humidity = rx_weather_data.humidity;
    if (weatherMeasure->humidity > 0.0) {
        weatherMeasure->hasHumidity = true;
    }
    WeatherStationManager::getInstance()->fanetWeatherQueue.emplace_back(weatherMeasure);
    LOG_F(2, "WeatherMeasure: %s", weatherMeasure->toString().c_str());
}
/*
void sendWeather(WeatherStation *ws) {
    LOG_SCOPE_FUNCTION(5);
    std::mutex *_mutexRadio = &GroundStation::getInstance()->radioMutex;
    if (ws->lastMeasure) {
        std::unique_lock<std::mutex> lock{*_mutexRadio};

        sRadioData _radiodata;
        sFanetMAC _fanet_mac;
        sWeather _tx_weather_data;
        sRawMessage _tx_message;

        _tx_weather_data.e_header = false;
        strcpy(_tx_weather_data.id_station, ws->id.c_str());
        strcpy(_tx_weather_data.name, ws->name.c_str());
        strcpy(_tx_weather_data.short_name, ws->shortName.c_str());
        _tx_weather_data.time = ws->lastMeasure->timestamp;

        _tx_weather_data.longitude = ws->longitude;
        _tx_weather_data.latitude = ws->latitude;
        _tx_weather_data.altitude = ws->altitude;

        _tx_weather_data.wind = true;
        _tx_weather_data.wind_heading = ws->lastMeasure->windDir;
        _tx_weather_data.wind_speed = ws->lastMeasure->windSpeed;
        _tx_weather_data.wind_gusts = ws->lastMeasure->windGusts;

        _tx_weather_data.temp = true;
        _tx_weather_data.temperature = ws->lastMeasure->temperature;

        _tx_weather_data.humid = false;
        _tx_weather_data.humidity = 0.0;
        _tx_weather_data.barom = false;
        _tx_weather_data.barometric = 0.0;

        _tx_message.m_length = 0;
        type_4_service_coder(&_tx_message, &_tx_weather_data);

        _fanet_mac.type = 4;
        _fanet_mac.s_manufactur_id = ws->manufacturerId;
        _fanet_mac.s_unique_id = ws->uniqueId;
        _fanet_mac.e_header = 0;
        _fanet_mac.forward = 0;
        _fanet_mac.ack = 0;
        _fanet_mac.cast = 0;
        _fanet_mac.signature_bit = 0;

        fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_message);
        write_tx_data(&_radiodata, &_tx_message);

        sRawMessage _rx_payload;
        fanet_mac_decoder(&_fanet_mac, &_tx_message, &_rx_payload);
        sWeather _rx_weather_data;
        type_4_service_decoder(&_rx_payload, &_rx_weather_data);
        terminal_message_4(true, false, &_radiodata, &_fanet_mac, &_rx_weather_data);
        lock.unlock();
    } else {
        cerr << "-- no WeatherMeasure for WS: " << ws->id << "/" << ws->name << endl;
    }
}

void receivepacket(bool requireValidBit, bool requireValidCRC) {
	sFanetMAC _fanet_mac;
	sRawMessage _rx_radio;
	sRawMessage _rx_payload;
	sRadioData _radiodata;
    TDequeConcurrent<Track*> *trackQueuePtr = &GroundStation::getInstance()->trackQueue;
    TDequeConcurrent<EntityName*> *nameQueuePtr = &GroundStation::getInstance()->nameQueue;
    std::mutex *_mutexRadio = &GroundStation::getInstance()->radioMutex;

    std::unique_lock<std::mutex> lock{*_mutexRadio};
	bool radioRxData = read_rx_data(&_rx_radio, &_radiodata);   // receive packet

	if (radioRxData) { //read_rx_data(&_rx_radio, &_radiodata)) {
		_fanet_mac.valid_bit = 1;

		if (_radiodata.crc_err)
			terminal_message_crc_err(0, 0, &_radiodata, &_fanet_mac);

		fanet_mac_decoder(&_fanet_mac, &_rx_radio, &_rx_payload);
		if (!_fanet_mac.valid_bit) {
			terminal_message_mac_err(0, 0, &_radiodata, &_fanet_mac);
		}
		terminal_message_raw(0, 0, &_radiodata, &_fanet_mac, &_rx_radio);
        if (_fanet_mac.type == 9) {
        }
        Packet *packet = new Packet();
        packet->radioData = _radiodata;
        packet->rawMessage = _rx_radio;
        packet->fanetMAC = _fanet_mac;
        LOG_F(2, "RAW: %s", packet->toString().c_str());
        GroundStation::getInstance()->packetQueue.emplace_back(packet);

		if (_fanet_mac.ack) {
            LOG_F(1, "send - ACK: %02X:%04X->%02X:%04X",
                    _fanet_mac.s_manufactur_id, _fanet_mac.s_unique_id,
                    _fanet_mac.d_manufactur_id, _fanet_mac.d_unique_id);
			send_ack(&_fanet_mac);
		}
		lock.unlock();

		if (requireValidBit && (_fanet_mac.valid_bit == 0)) {
            LOG_F(2, "invalid packet ignored - valid_bit");
            return;
        }
        if (requireValidCRC && (_radiodata.crc_err == 1)) {
            LOG_F(2, "invalid packet ignored - CRC_ERR");
            return;
        }
        // Dispatch activities
			switch (_fanet_mac.type) {
				case 0:
					type_0_ack_receiver(&_radiodata, &_fanet_mac, &_rx_payload);
                    LOG_F(2, "receive - ACK: %02X:%04X->%02X:%04X",
                            _fanet_mac.s_manufactur_id, _fanet_mac.s_unique_id,
                            _fanet_mac.d_manufactur_id, _fanet_mac.d_unique_id);
                    break;
				case 1: {
					AirTrack *airTrack = new AirTrack();
					sAirTracking _rx_tracking;
					type_1_tracking_decoder(&_rx_payload, &_fanet_mac, &_rx_tracking);
                    terminal_message_1(0, 0, &_radiodata, &_fanet_mac, &_rx_tracking);
                    airTrack->initFromFanetPacket(&_rx_tracking);
                    trackQueuePtr->emplace_back(airTrack);
					LOG_F(1, "recv - %s", airTrack->toString().c_str());
					break;
				}
				case 2: {
					type_2_name_receiver(&_radiodata, &_fanet_mac, &_rx_payload);
					char nameStr[256];
					memset(nameStr, 0, 256);
					strncpy(nameStr, _rx_payload.message, _rx_payload.m_length);
                    char idStr[10];
                    sprintf(idStr, "%02X%04X", _fanet_mac.s_manufactur_id, _fanet_mac.s_unique_id);
                    LOG_F(1, "recv - Id: '%s' - Name: '%s'", idStr, nameStr);
                    EntityName *nameEntity = new EntityName(idStr, nameStr, nameStr, _fanet_mac.s_manufactur_id, _fanet_mac.s_unique_id);
                    nameQueuePtr->emplace_back(nameEntity);
					break;
				}
				case 3: {
					type_3_message_receiver(&_radiodata, &_fanet_mac, &_rx_payload);
                    sMessage _rx_message;
                    type_3_message_decoder(&_rx_payload, &_rx_message);

                    char msgStr[256];
                    memset(msgStr, 0, 256);
                    strncpy(msgStr, _rx_payload.message+1, _rx_payload.m_length-1);
                    char idStr[20];
                    sprintf(idStr, "%02X:%04X->%02X:%04X",
                            _fanet_mac.s_manufactur_id, _fanet_mac.s_unique_id,
                            _fanet_mac.d_manufactur_id, _fanet_mac.d_unique_id);
                    LOG_F(1, "recv - Message: '%s' '%s'", idStr, msgStr);
					break;
				}
				case 4:
					type_4_service_receiver(&_radiodata, &_fanet_mac, &_rx_payload);
                    LOG_F(2, "recv - Service");
                    // decode FANET weather and handle as SkyTraxx weather measurement
                    sWeather _rx_weather_data;
                    type_4_service_decoder(&_rx_payload, &_rx_weather_data);
                    _rx_weather_data.time = time(0); // TBD
                    if (!_rx_weather_data.gateway)
                        receiveFanetWeather(_radiodata, _fanet_mac, _rx_weather_data);
					break;
                case 5:
                    LOG_F(2, "recv - Landmark");
                    type_5_landmark_receiver(&_radiodata, &_fanet_mac, &_rx_payload);
                    break;
				case 7: {
					GroundTrack *groundTrack = new GroundTrack();
					sGroundTracking _rx_ground_tracking;
					type_7_tracking_decoder(&_rx_payload, &_fanet_mac, &_rx_ground_tracking);
                    terminal_message_7(0, 0, &_radiodata, &_fanet_mac, &_rx_ground_tracking);
					groundTrack->initFromFanetPacket(&_rx_ground_tracking);
                    trackQueuePtr->emplace_back(groundTrack);
					LOG_F(1, "recv - %s", groundTrack->toString().c_str());
					break;
				}
                case 8: {
                    LOG_F(1, "recv - HW-Info");
                    sHardwareInfo _hwinfo;
                    type_8_hwinfo_decoder(&_rx_payload, &_fanet_mac, &_hwinfo);
                    terminal_message_8(false, true, &_radiodata, &_fanet_mac, &_hwinfo);
                    break;
                }
                case 9: {
                    LOG_F(INFO, "recv - Thermal-Info");
                    sThermalInfo _thermalinfo;
                    type_9_thermal_decoder(&_rx_payload, &_fanet_mac, &_thermalinfo);
                    terminal_message_9(false, true, &_radiodata, &_fanet_mac, &_thermalinfo);
                    break;
                }
                default: {
                    LOG_F(INFO, "recv - UNKNOWN");
                    terminal_message_raw(false, true, &_radiodata, &_fanet_mac, &_rx_payload);
                }
			}
		//}
        memset(&_rx_payload.message, 0, sizeof(_rx_payload.message));
        _rx_payload.m_length = 0;
        _rx_payload.m_pointer = 0;
	}
}

void trackProducer() {
	loguru::set_thread_name("Radio-Receiver");
    int pauseMilliSecs = Configuration::getInstance()->getValue("/configuration/features/fanetRadio/interval", 10);
    LOG_F(INFO, "TrackProducer/Radio-Receiver - RUN - interval: %d", pauseMilliSecs);
    bool requireValidBit = Configuration::getInstance()->getValue("/configuration/features/fanetRadio/requireValidBit", true);
    bool requireValidCRC = Configuration::getInstance()->getValue("/configuration/features/fanetRadio/requireValidCRC", true);
    LOG_IF_F(INFO, requireValidBit,  "Radio-Receiver - require valid bit");
    LOG_IF_F(INFO, !requireValidBit, "Radio-Receiver - ignore valid bit");
    LOG_IF_F(INFO, requireValidCRC,  "Radio-Receiver - require valid CRC");
    LOG_IF_F(INFO, !requireValidCRC, "Radio-Receiver - ignore CRC");
    finished = false;
	while (!finished) {
		receivepacket(requireValidBit, requireValidCRC);
        if (pauseMilliSecs >= 0) {
            std::this_thread::sleep_for(chrono::milliseconds(pauseMilliSecs));
        }
	}
	LOG_F(INFO, "TrackProducer - END");
}
*/
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

void sendFanetWeatherToInternet() {
    VLOG_SCOPE_F(1, "sendFanetWeatherToInternet - trying to consume...");
    TDequeConcurrent<WeatherMeasure*> *fanetWeatherQueuePtr = &WeatherStationManager::getInstance()->fanetWeatherQueue;
    TDequeConcurrent<Track*> *trackQueuePtr = &GroundStation::getInstance()->trackQueue;
    if (!fanetWeatherQueuePtr->empty()) {
        LOG_SCOPE_FUNCTION(INFO);
        while (!fanetWeatherQueuePtr->empty()) {
            auto wm = fanetWeatherQueuePtr->pop_front();
            LOG_F(1, "FanetWeather - consuming (%lu) : %s", trackQueuePtr->size(), wm->toString().c_str());
            FanetWeatherStation *fanetWeatherStation = WeatherStationManager::getInstance()->getByFanetId(wm->manufacturerId, wm->uniqueId);
            if (!fanetWeatherStation) {
                char wsId[15];
                sprintf(wsId, "%02X%04X", wm->manufacturerId, wm->uniqueId);
                fanetWeatherStation = new FanetWeatherStation(wsId, wsId, wsId, wm->manufacturerId, wm->uniqueId, 0.0, 0.0, 0);
                WeatherStationManager::getInstance()->addFanetWeatherStation(fanetWeatherStation);
            }
            if (fanetWeatherStation) {
                fanetWeatherStation->lastMeasure = wm;
                fanetWeatherStation->update();
            }
        }
    } else {
        LOG_F(1, "sendFanetWeatherToInternet - Nothing to send");
    }
}
/*
void sendWeatherToFanet(int pauseSecsBetweenStations, int maxAgeSeconds) {
    LOG_SCOPE_FUNCTION(1);
    map<string, WeatherStation *> * wsMap = WeatherStationManager::getInstance()->weatherStations;
    for (map<string, WeatherStation *>::iterator iter = wsMap->begin(); iter != wsMap->end(); iter++) {
        WeatherStation *ws = iter->second;
        WeatherMeasure *wm = ws->lastMeasure;
        if (wm) {
            time_t now = time(0);
            int ageSeconds = difftime(now, wm->timestamp);
            if (ageSeconds <= maxAgeSeconds) {
                LOG_F(1, "WeatherMeasure: %s - age: %d [sec]", wm->toString().c_str(), ageSeconds);
                sendWeather(iter->second);
            } else {
                LOG_F(5, "WeatherMeasure: %s - age: %d [sec] - outdated - NOT sending", wm->toString().c_str(),
                      ageSeconds);
            }
        } else {
            LOG_F(1, "no measure for station %s", iter->first.c_str());
        }
        LOG_F(7, "WeatherMeasureConsumer - going to sleep for %d [sec]", pauseSecsBetweenStations);
        std::this_thread::sleep_for(chrono::seconds(pauseSecsBetweenStations));
    }
}

void sendHwInfo(
        u_int8_t manufacturerId, u_int16_t uniqueId, uint8_t device_type,
        uint16_t fw_build_year, uint8_t fw_build_month, uint8_t fw_build_day, bool experimantal,
        uint8_t fw_add1, uint8_t fw_add2) {
    LOG_SCOPE_FUNCTION(5);
    LOG_F(1, "sendHwInfo: %02X:%04X : %02d - %04d-%02d-%02d : %02X %02X",
          manufacturerId, uniqueId, device_type, fw_build_year, fw_build_month, fw_build_day, fw_add1, fw_add2);
    std::mutex *_mutexRadio = &GroundStation::getInstance()->radioMutex;

    std::unique_lock<std::mutex> lock{*_mutexRadio};
    sRadioData _radiodata;
    sFanetMAC _fanet_mac;
    sRawMessage _tx_rawMessage;
    sHardwareInfo _tx_hwinfo;

    _tx_rawMessage.m_length = 0;
    _fanet_mac.type = 8;
    _fanet_mac.s_manufactur_id = manufacturerId;
    _fanet_mac.s_unique_id = uniqueId;
    _fanet_mac.e_header = 0;
    _fanet_mac.forward = 0;
    _fanet_mac.cast = 0;
    _fanet_mac.ack = 0;
    _fanet_mac.signature_bit = 0;

    memset((void*)&_tx_hwinfo, 0, sizeof(_tx_hwinfo));

    _tx_hwinfo.s_address_manufactur_id = manufacturerId;
    _tx_hwinfo.s_address_unique_id = uniqueId;
    _tx_hwinfo.device_type = device_type;
    _tx_hwinfo.experimental = experimantal;
    _tx_hwinfo.fw_build_year = fw_build_year;
    _tx_hwinfo.fw_build_month = fw_build_month;
    _tx_hwinfo.fw_build_day = fw_build_day;
    _tx_hwinfo.add1 = fw_add1;
    _tx_hwinfo.add2 = fw_add2;

    type_8_hwinfo_coder(&_tx_rawMessage, &_tx_hwinfo);
    fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_rawMessage);
    write_tx_data(&_radiodata, &_tx_rawMessage);

    lock.unlock();
}

void sendMessage(bool unicast, u_int8_t s_manufacturerId, u_int16_t s_uniqueId, u_int8_t d_manufacturerId, u_int16_t d_uniqueId, string message) {
    LOG_SCOPE_FUNCTION(5);
    LOG_F(1, "sendMessage: %02X:%04X -> %02X:%04X : '%s'",
          s_manufacturerId, s_uniqueId,
          d_manufacturerId, d_uniqueId,
          message.c_str());
    std::mutex *_mutexRadio = &GroundStation::getInstance()->radioMutex;
    std::unique_lock<std::mutex> lock{*_mutexRadio};

    sRadioData _radiodata;
    sFanetMAC _fanet_mac;
    sRawMessage _tx_rawMessage;
    sMessage _t3_message;

    _tx_rawMessage.m_length = 0;
    _fanet_mac.type = 3;
    _fanet_mac.s_manufactur_id = s_manufacturerId;
    _fanet_mac.s_unique_id = s_uniqueId;
    if (unicast) {
        _fanet_mac.d_manufactur_id = d_manufacturerId;
        _fanet_mac.d_unique_id = d_uniqueId;
        _fanet_mac.e_header = true;
        _fanet_mac.forward = true;
        _fanet_mac.cast = true;
        _fanet_mac.ack = true;
    } else {
        _fanet_mac.d_manufactur_id = 0x00;
        _fanet_mac.d_unique_id = 0x0000;
        _fanet_mac.e_header = false;
        _fanet_mac.forward = true;
        _fanet_mac.cast = false;
        _fanet_mac.ack = false;
    }
    _fanet_mac.signature_bit = false;

    memset((void*)&_t3_message, 0, sizeof(_t3_message));
    strcpy(_t3_message.message, message.c_str());
    _t3_message.m_length = strlen(_t3_message.message);

    type_3_message_coder(&_tx_rawMessage, &_t3_message);
    fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_rawMessage);
    write_tx_data(&_radiodata, &_tx_rawMessage);

    lock.unlock();
}

void sendMessageToDevice(u_int8_t s_manufacturerId, u_int16_t s_uniqueId, u_int8_t d_manufacturerId, u_int16_t d_uniqueId, string message) {
    LOG_F(1, "sendMessageToDevice: %02X:%04X -> %02X:%04X : '%s'",
          s_manufacturerId, s_uniqueId,
          d_manufacturerId, d_uniqueId,
          message.c_str());
    sendMessage(true, s_manufacturerId, s_uniqueId, d_manufacturerId, d_uniqueId, message);
}

void sendMessageToAll(u_int8_t s_manufacturerId, u_int16_t s_uniqueId, string message) {
    LOG_F(1, "sendMessageToAll: %02X:%04X -> XX:XXXX : '%s'",
          s_manufacturerId, s_uniqueId,
          message.c_str());
    sendMessage(false, s_manufacturerId, s_uniqueId, 0x00, 0x0000, message);
}
*/
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
        LOG_F(1, "Pushing FANET weather data to Inet");
        sendFanetWeatherToInternet();
        LOG_F(1, "Pushing weather data to FANET");
        sendWeatherToFanet(pauseSecsBetweenStations, maxAgeSeconds);
        LOG_F(7, "TrackConsumer - going to sleep for %d [msec]", intervalTrackPush);
        std::this_thread::sleep_for(chrono::milliseconds(intervalTrackPush));

        if (GroundStation::getInstance()->sendHwInfo) {
            FirmwareInfo *fwInfo = &GroundStation::getInstance()->fwInfo;
            sendHwInfo(GroundStation::getInstance()->manufacturerId, GroundStation::getInstance()->uniqueId,
                       0x42, fwInfo->year, fwInfo->month, fwInfo->day, fwInfo->experimental, fwInfo->add1, fwInfo->add2);
        }
    } // while
    LOG_F(INFO, "queuesConsumer - END");
}

void queuesConsumer2() {
    LOG_F(INFO, "queuesConsumer - 2 - Start");
    loguru::set_thread_name("Queue2-Cons");

    int intervalMessagePush = Configuration::getInstance()->getValue("/configuration/features/messagePushing/interval", 3000);
    TDequeConcurrent<Message*> *messageQueuePtr = &GroundStation::getInstance()->messageToFanetQueue;

    while (!finished) {
        LOG_F(6, "Pushing Messages to FANET");
        if (!messageQueuePtr->empty()) {
            while (!messageQueuePtr->empty()) {
                auto message = messageQueuePtr->pop_front();
                LOG_F(1, "Message - consuming (%lu) : %s", messageQueuePtr->size(), message->toString().c_str());
                sendMessage(message->unicast,
                            message->s_manufacturerId, message->s_uniqueId,
                            message->d_manufacturerId, message->d_uniqueId,
                            message->message);
                std::this_thread::sleep_for(chrono::milliseconds(333));
            }
        }
        std::this_thread::sleep_for(chrono::milliseconds(intervalMessagePush));
    } // while
    LOG_F(INFO, "queuesConsumer - 2 - END");
}

void weatherStationManagerRunner (int pauseSecs) {
    loguru::set_thread_name("Weather-get");
    LOG_F(INFO, "WeatherStationManager - RUN");
    WeatherStationManager::getInstance()->run();
    LOG_F(INFO, "WeatherStationManager - END");
};
/*
void sendName(u_int8_t manufacturerId, u_int16_t uniqueId, string name) {
	LOG_SCOPE_FUNCTION(5);
	LOG_F(5, "sendName: %s", name.c_str());
    std::mutex *_mutexRadio = &GroundStation::getInstance()->radioMutex;
	std::unique_lock<std::mutex> lock{*_mutexRadio};

	sRadioData _radiodata;
	sFanetMAC _fanet_mac;
	sRawMessage _tx_message;
	sName _tx_name;

	_tx_message.m_length = 0;
	_fanet_mac.type = 2;
	_fanet_mac.s_manufactur_id = manufacturerId;
	_fanet_mac.s_unique_id = uniqueId;
	_fanet_mac.e_header = false;
	_fanet_mac.forward = true;
	_fanet_mac.ack = false;
	_fanet_mac.cast = false;
	_fanet_mac.signature_bit = false;

	strcpy(_tx_name.name, name.c_str());
	_tx_name.n_length = strlen(_tx_name.name);

	type_2_name_coder(&_tx_message, &_tx_name);
	fanet_mac_coder(&_radiodata, &_fanet_mac, &_tx_message);
    write_tx_data(&_radiodata, &_tx_message);

	lock.unlock();
}

void nameHandler(int pauseSecs, int pauseSecsBetweenNames) {
	loguru::set_thread_name("Name-push");
	LOG_SCOPE_FUNCTION(INFO);
	LOG_F(INFO, "EntityNameHandler - RUN - interval %d / %d [sec]", pauseSecs, pauseSecsBetweenNames);
	while (!finished) {
		VLOG_SCOPE_F(5, "EntityName - sending names");
		list<fanet::EntityName>::iterator iter;// = EntityNameManager::getInstance()->nameList;
		for (iter = EntityNameManager::getInstance()->nameList.begin(); iter != EntityNameManager::getInstance()->nameList.end(); iter++) {
			LOG_F(1, "EntityName: %s", iter->toString().c_str());
			sendName(iter->manufacturerId, iter->uniqueId, iter->shortName);
			LOG_F(7, "EntityNameHandler - going to sleep for %d [sec]", pauseSecsBetweenNames);
			std::this_thread::sleep_for(chrono::seconds(pauseSecsBetweenNames));
		}
		LOG_F(7, "EntityNameHandler - going to sleep for %d [sec]", pauseSecs);
		std::this_thread::sleep_for(chrono::seconds(pauseSecs));
	}
	LOG_F(INFO, "EntityNameHandler - END");
}
*/
void groundStationManagerRunner(int pauseSecs) {
    loguru::set_thread_name("GS-RUN");
	LOG_F(INFO, "GroundStationManager - RUN");
 	GroundStation::getInstance()->run();
	LOG_F(INFO, "GroundStationManager - END");
};

void landmarkManagerRunner() {
    loguru::set_thread_name("MmMgr");
    LOG_F(INFO, "LandmarkManager - RUN");
    bool doLandmarkPush = Configuration::getInstance()->getValue("/configuration/features/landmarkPushing/active", false);
    LOG_IF_F(INFO, doLandmarkPush, "Feature: Push Landmarks");
    if (doLandmarkPush) {
        LandmarkManager::getInstance()->init();
        LandmarkManager::getInstance()->run();
    }
    LOG_F(INFO, "LandmarkManager - END");
};
/*
void vehiclePositionHandler(int pauseSecs, int pauseSecsBetweenNames) {
    LOG_SCOPE_FUNCTION(INFO);
    loguru::set_thread_name("Veh-Recv");
    sleep(2);
    LOG_F(INFO, "VehiclePositionHandler - RUN - interval %d / %d [sec]", pauseSecs, pauseSecsBetweenNames);
    while (!finished) {
        VLOG_SCOPE_F(5, "VehiclePositionHandler - sending vehicle positions");
        for (map<string, Vehicle *>::iterator iter = GroundStation::getInstance()->vehicleMap.begin();
                iter != GroundStation::getInstance()->vehicleMap.end(); iter++) {
            Vehicle *vehicle = iter->second;
            LOG_F(5, "updating Vehicle: %s", iter->first.c_str());
            vehicle->update();
            if (vehicle->lastPosition != NULL) {
                LOG_F(5, "Vehicle: %s - %s", iter->first.c_str(), vehicle->lastPosition->toString().c_str());
                time_t now = time(0);
                if ((now - vehicle->maxAgeSeconds) <= vehicle->lastPosition->timestamp) {
                    GroundStation::getInstance()->addTrack(iter->second->lastPosition);
                } else {
                    LOG_F(7, "Vehicle - last position too old");
                }
            }
            std::this_thread::sleep_for(chrono::seconds(pauseSecsBetweenNames));
        }
        LOG_F(7, "VehiclePositionHandler - going to sleep for %d [sec]", pauseSecs);
        std::this_thread::sleep_for(chrono::seconds(pauseSecs));
    }
    LOG_F(INFO, "VehiclePositionHandler - END");
}
*/
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

    cout << "-- FANET Groundstation --" << endl;
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
    string fanetLogFileName = config->getValue("/configuration/features/fanetRadio/logfile", "./fanet_packets.txt");
    FILE *fanetLogFile = fopen(fanetLogFileName.c_str(), "a");

    terminal_set_file(fanetLogFile);
    init_fanet_radio(1000, true, power);

    Message startMsg;
    startMsg.initFromText(Configuration::getInstance()->getStringValue("/configuration/stationId"),
                          "FFFFFF", false,
                          "FANET-GS " + Configuration::getInstance()->getStringValue("/configuration/stationName") + " - Start");
    sendMessageToAll(startMsg.s_manufacturerId, startMsg.s_uniqueId, string(startMsg.message));

    AprsOgnManager::getInstance()->connect();

    std::thread threadRadioListener(trackProducer);
    WeatherStationManager::getInstance()->init();
    std::thread threadWeatherStationManager(weatherStationManagerRunner, 60);

    std::thread threadQueuesConsumer1(queuesConsumer);
    std::thread threadQueuesConsumer2(queuesConsumer2);

    EntityNameManager::getInstance()->init();

	std::thread threadNameHandler(nameHandler, 300, 5);

    std::thread groundStationManagerThread(groundStationManagerRunner, 300);

    std::thread landmarkManagerThread(landmarkManagerRunner);

    std::thread threadVehiclePositionHandler(vehiclePositionHandler, 30, 2);

    threadRadioListener.join();

	std::cout << "finished!" << std::endl;

	return 0;
}

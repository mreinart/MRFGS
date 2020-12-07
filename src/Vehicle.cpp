//
// Vehicle
//

#include "Configuration.h"
#include "Track.h"
#include "Vehicle.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidcsv.h>
#include <iostream>
#include <string>
#include "loguru.hpp"
#include <httplib.h>

using namespace fanet;
using namespace rapidjson;
using namespace std;

void Vehicle::init() {
    LOG_SCOPE_FUNCTION(5);
    dfDbActive_ = Configuration::getInstance()->getValue("/configuration/TrackDB/active", false);
    dbHost_ = Configuration::getInstance()->getValue("/configuration/TrackDB/updateHost", "www.duddefliecher.de");
    readUrl_ = Configuration::getInstance()->getValue("/configuration/TrackDB/readUrl", "/m/get_vehicle_pos_recent.php");
    initialized_ = true;
}

void Vehicle::updateDfDb() {
    LOG_SCOPE_FUNCTION(6);
    // TBD
}

void Vehicle::readDfDb() {
    LOG_SCOPE_FUNCTION(6);
    try {
        httplib::SSLClient client(dbHost_);
        char request[100];
        sprintf(request, "%s?device=%s", readUrl_.c_str(), this->id.c_str());
        auto res = client.Get(request);
        if (res && res->status == 200) {
            LOG_F(9, "CSV %s", res->body.c_str());
            std::string csvStr = res->body;
            std::stringstream sstream(csvStr);
            rapidcsv::Document doc(sstream);
            GroundTrack *track = new GroundTrack();
            std::string idStr;
            strcpy(track->callsign, this->id.c_str());
            track->manufacturerId = this->manufacturerId;
            track->uniqueId = this->uniqueId;
            strcpy(track->origin, "GDB");
            track->type = this->type;
            track->tracking = false;
            std::string tmsStr = doc.GetCell<string>("tms", this->id);
            tm tm{};
            strptime(tmsStr.c_str(), "\"%Y-%m-%d %H:%M:%S\"", &tm);
            track->timestamp = mktime(&tm);
            track->latitude = doc.GetCell<float>("lat", this->id);
            track->longitude = doc.GetCell<float>("lon", this->id);
            track->altitude = doc.GetCell<int>("alt", this->id);
            track->heading = doc.GetCell<float>("course", this->id);
            track->speed = doc.GetCell<float>("speed", this->id);
            this->lastPosition = track;
        } else {
            LOG_F(9, "no data");
        }
    } catch (const std::exception &e) {
        LOG_F(WARNING, "Issue with update of %s : %s", this->id.c_str(), e.what());
    } catch (...) {
        LOG_F(WARNING, "Issue with update of %s", this->id.c_str());
    }
}

void Vehicle::update() {
    LOG_SCOPE_FUNCTION(6);
    if (!initialized_) {
        LOG_F(INFO, "NOT initialized: %s", this->id.c_str());
        return;
    }
    if (dfDbActive_ && this->pushDb) {
        this->updateDfDb();
    }
    if (dfDbActive_ && this->pullDb) {
        this->readDfDb();
    }
}

string Vehicle::toJson() {
	LOG_SCOPE_FUNCTION(7);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);

    writer.StartObject();
    {
        writer.Key("id");
        writer.String(this->id.c_str());
        writer.Key("name");
        writer.String(this->name.c_str());
        writer.Key("uniqueId");
        writer.Uint(this->uniqueId);
        writer.Key("manufacturerId");
        writer.Uint(this->manufacturerId);
        if (this->lastPosition) {
            writer.Key("lastPosition");
            {
                writer.StartObject();
                writer.Key("tms");
                writer.Uint(this->lastPosition->timestamp);
                writer.Key("latitude");
                writer.Double(this->lastPosition->latitude);
                writer.Key("longitude");
                writer.Double(this->lastPosition->longitude);
                writer.Key("altitude");
                writer.Double(this->lastPosition->altitude);
                writer.EndObject();
            }
        }
    }
    writer.EndObject();

    cout << sb.GetString() << endl;
    return sb.GetString();
}

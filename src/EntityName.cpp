//
// EntityName
//

#include "EntityName.h"
#include "Configuration.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

using namespace std;
using namespace fanet;
using namespace rapidjson;

#include "loguru.hpp"

EntityNameManager *EntityNameManager::instance_ = 0;

string EntityName::toString() {
    char idStr[8];
    sprintf(idStr, "%02X:%04X", this->manufacturerId, this->uniqueId);
        stringstream ss;
    ss << "Name: "
       << this->id
       << " - " << this->shortName
       << "/" << this->name
       << "/" << idStr;
    return string(ss.str());
}

void EntityNameManager::reinit() {
    LOG_SCOPE_FUNCTION(INFO);
    initialized_ = false;
    init();
}

void EntityNameManager::init() {
	LOG_SCOPE_FUNCTION(INFO);
	if (!initialized_) {
        LOG_F(INFO, "EntityNameManager - getting names");
        Document jsonDoc;
        jsonDoc.Parse(Configuration::getInstance()->getJson().c_str());

        Value *wsArray = Pointer("/configuration/weatherStations").Get(jsonDoc);
        if (wsArray->IsArray()) {
            for (auto &wsv : wsArray->GetArray()) {
                if (wsv["active"].GetBool()) {
                    EntityName nameEntity;
                    nameEntity.id = wsv["id"].GetString();
                    nameEntity.name = wsv["name"].GetString();
                    nameEntity.shortName = wsv["shortName"].GetString();
                    nameEntity.manufacturerId = strtol(wsv["manufacturerId"].GetString(), NULL, 16);
                    nameEntity.uniqueId = strtol(wsv["uniqueId"].GetString(), NULL, 16);
                    if ((nameEntity.shortName.length()) > 0 && (nameEntity.shortName.compare("doNotPublish") != 0)) {
                        nameList.emplace_back(nameEntity);
                        LOG_F(INFO, "WS-Name: %s", nameEntity.toString().c_str());
                    } else {
                        LOG_F(INFO, "Not Publishing: %s", nameEntity.toString().c_str());
                    }
                }
            }
        }
        Value *nameArray = Pointer("/configuration/fanetNames").Get(jsonDoc);
        if (nameArray && nameArray->IsArray()) {
            for (auto &name : nameArray->GetArray()) {
                if (name["active"].GetBool()) {
                    EntityName nameEntity;
                    nameEntity.id = name["id"].GetString();
                    nameEntity.name = name["name"].GetString();
                    nameEntity.shortName = name["shortName"].GetString();
                    nameEntity.manufacturerId = strtol(name["manufacturerId"].GetString(), NULL, 16);
                    nameEntity.uniqueId = strtol(name["uniqueId"].GetString(), NULL, 16);
                    nameList.emplace_back(nameEntity);
                    LOG_F(INFO, "%s", nameEntity.toString().c_str());
                }
            }
        }
        Value *vehArray = Pointer("/configuration/vehicles").Get(jsonDoc);
        if (vehArray && vehArray->IsArray()) {
            for (auto &veh : vehArray->GetArray()) {
                if (veh["active"].GetBool()) {
                    EntityName nameEntity;
                    nameEntity.id = veh["id"].GetString();
                    nameEntity.name = veh["name"].GetString();
                    nameEntity.shortName = nameEntity.name;
                    nameEntity.manufacturerId = strtol(veh["manufacturerId"].GetString(), NULL, 16);
                    nameEntity.uniqueId = strtol(veh["uniqueId"].GetString(), NULL, 16);
                    nameList.emplace_back(nameEntity);
                    LOG_F(INFO, "%s", nameEntity.toString().c_str());
                }
            }
        }
        LOG_F(INFO, "# names: %lu", nameList.size());
    } else {
        LOG_F(INFO, "already initialized");
    }
}

void EntityNameManager::run() {
	LOG_F(INFO, "EntityNameManager - RUN");
}


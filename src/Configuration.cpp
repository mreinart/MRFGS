//
// Configuration
//

#include "Configuration.h"

#include <stdio.h>
#include "rapidjson/pointer.h"
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
#include <fstream>
#include "loguru.hpp"

using namespace fanet;
using namespace rapidjson;
using namespace std;

Configuration *Configuration::instance_ = 0;

void Configuration::init(string token) {
	LOG_F(INFO, "Configuration initialization: %s", token.c_str());

	if (!initialized_) {
        // do initialization
        this->cfgToken_ = token;

        ifstream ifs(token);
        IStreamWrapper isw(ifs);
        jsonDoc.ParseStream(isw);
        if (jsonDoc.HasMember("configuration")) {
            printf("SN = %s\n", jsonDoc["configuration"]["stationName"].GetString());
        }

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        jsonDoc.Accept(writer);
        const char* output = buffer.GetString();
        jsonStr_ = buffer.GetString();
        initialized_ = true;
    }
}

string Configuration::getJson() {
    return jsonStr_;
};

Value *Configuration::getValue(string path) {
    return Pointer(path.c_str()).Get(jsonDoc);
}

bool Configuration::getBoolValue(string path) {
	LOG_SCOPE_FUNCTION(7);
	LOG_F(7, "path: %s", path.c_str());
    return (getValue(path))->GetBool();
}

bool Configuration::getValue(string path, bool defaultValue) {
	LOG_SCOPE_FUNCTION(8);
	LOG_F(5, "path: %s", path.c_str());
    Value *valPtr = getValue(path);
    if (valPtr) {
		LOG_F(5, "found value: %d", valPtr->GetBool());
        return valPtr->GetBool();
    }
	LOG_F(5, "returning default value: %d", defaultValue);
    return defaultValue;
}

int Configuration::getIntValue(string path) {
	LOG_F(5, "path: %s", path.c_str());
    return (getValue(path))->GetInt();
}

int Configuration::getValue(string path, int defaultValue) {
	LOG_F(5, "path: %s", path.c_str());
    Value *valPtr = getValue(path);
    if (valPtr) {
		LOG_F(4, "path '%s' value: '%d'", path.c_str(),valPtr->GetInt());
        return valPtr->GetInt();
    }
	LOG_F(5, "returning default value: %d", defaultValue);
    return defaultValue;
}

float Configuration::getValue(string path, float defaultValue) {
    Value *valPtr = getValue(path);
    if (valPtr) {
        LOG_F(4, "path '%s' value: '%f'", path.c_str(), valPtr->GetFloat());
        return valPtr->GetFloat();
    }
    LOG_F(4, "path '%s' default value: '%f'", path.c_str(), defaultValue);
    return defaultValue;
}

const char *Configuration::getCharPtrValue(string path){
	LOG_F(5, "path: %s", path.c_str());
    return (getValue(path))->GetString();
}

string Configuration::getStringValue(string path) {
	LOG_F(5, "path: %s", path.c_str());
    return (getValue(path))->GetString();
}

string Configuration::getValue(string path, string defaultValue) {
	LOG_SCOPE_FUNCTION(5);
	LOG_F(5, "path: %s", path.c_str());
    Value *valPtr = getValue(path);
    if (valPtr) {
        LOG_F(4, "path '%s' value: '%s'", path.c_str(), valPtr->GetString());
        return valPtr->GetString();
    }
	LOG_F(4, "path '%s' default value: '%s'", path.c_str(), defaultValue.c_str());
    return defaultValue;
}

string Configuration::getValue(string path, const char *defaultValue) {
	LOG_SCOPE_FUNCTION(5);
	LOG_F(5, "path: %s", path.c_str());
    Value *valPtr = getValue(path);
    if (valPtr) {
		return valPtr->GetString();
        return valPtr->GetString();
    }
    LOG_F(4, "path '%s' default value: '%s'", path.c_str(), defaultValue);
    return defaultValue;
}


string Configuration::getValueForKey(string key) {
    // TODO: MAP
    if (jsonDoc.HasMember("configuration")) {
        auto cfg = jsonDoc["configuration"].GetObject();
        if (cfg.HasMember(key.c_str())) {
            return cfg[key.c_str()].GetString();
        }
    }
    if (key == "stationName" ) {
        return "FanGSPalz";
    }
    if (key == "version" ) {
        return "0.1";
    }
    if (key == "configToken" ) {
        return cfgToken_;
    }
    return "";
}

string Configuration::getValueForKey(string group, string key) {
    if (jsonDoc.HasMember(("configuration." + group + "." + key).c_str())) {
        return jsonDoc["configuration"][group.c_str()][key.c_str()].GetString();
    }
    return "";
}

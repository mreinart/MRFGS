//
// Message
//

#include "Message.h"

#include "loguru.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace rapidjson;
using namespace fanet;

string Message::toString() {
	struct tm *timeinfo;
	timeinfo = localtime(&this->timestamp);
	char timeBuf[80];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
	stringstream ss;
	ss << "Message: "
       << timeBuf << "(" << this->timestamp << ")"
	   << this->source
            << " -> " << this->dest;
	if (this->unicast)
	    ss << "(unicast)";
    else
        ss << "(broadcast)";
    ss << " : '" << this->message << "'";
	return string(ss.str());
}

string Message::toJson() {
    struct tm *timeinfo;
    timeinfo = localtime(&this->timestamp);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);

	writer.StartObject();
	{
        writer.Key("type");
        writer.String("Message");
        writer.Key("source");
        writer.String(this->source);
        writer.Key("dest");
        writer.String(this->dest);
        writer.Key("timestamp");
        writer.Uint(this->timestamp);
        writer.Key("dateTime");
        writer.String(timeBuf);
        writer.Key("unicast");
        writer.Bool(this->unicast);

        char xbuf[5];
        writer.Key("s_manufacturerId");
        sprintf(xbuf, "%02X", this->s_manufacturerId);
        writer.String(xbuf);
        writer.Key("s_uniqueId");
        sprintf(xbuf, "%04X", this->s_uniqueId);
        writer.String(xbuf);

        writer.Key("d_manufacturerId");
        sprintf(xbuf, "%02X", this->d_manufacturerId);
        writer.String(xbuf);
        writer.Key("d_uniqueId");
        sprintf(xbuf, "%04X", this->d_uniqueId);
        writer.String(xbuf);

        writer.Key("message");
        writer.String(message);
    }
    writer.EndObject();
    cout << sb.GetString() << endl;
    return sb.GetString();
}

void Message::initFromJson(std::string json) {
    strcpy(this->source, "UNK");
    try {
        Document d;
        d.Parse(json.c_str());
        if (!d.IsObject())
            return;
        Value::ConstMemberIterator itr = d.FindMember("type");
        if (itr == d.MemberEnd()) {
            return;
        } else if (0 != strcmp("Message", itr->value.GetString())) {
            return;
        }
        Value &value = d["source"];
        std::string str = value.GetString();
        strcpy(this->source, str.c_str());
        value = d["dest"];
        str = value.GetString();
        strcpy(dest, str.c_str());
        value = d["timestamp"];
        timestamp = value.GetInt();
        value = d["unicast"];
        unicast = value.GetBool();
        s_manufacturerId = strtol(d["s_manufacturerId"].GetString(), NULL, 16);
        s_uniqueId = strtol(d["s_uniqueId"].GetString(), NULL, 16);
        d_manufacturerId = strtol(d["d_manufacturerId"].GetString(), NULL, 16);
        d_uniqueId = strtol(d["d_uniqueId"].GetString(), NULL, 16);
        value = d["message"];
        strcpy(this->message, value.GetString());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Issue with Message : %s", e.what());
    } catch (...) {
        LOG_F(ERROR, "Issue with Message");
    }
}

void Message::initFromText(std::string sender, std::string receiver, bool unicast, std::string message) {
    strcpy(this->source, sender.c_str());
    strcpy(this->dest,   receiver.c_str());
    timestamp = time(0);
    this->s_manufacturerId = strtol(sender.substr(0,2).c_str(), NULL, 16);
    this->s_uniqueId = strtol(sender.substr(2,4).c_str(), NULL, 16);
    this->unicast = unicast;
    if (this->unicast) {
        this->d_manufacturerId = strtol(receiver.substr(0,2).c_str(), NULL, 16);
        this->d_uniqueId = strtol(receiver.substr(2,4).c_str(), NULL, 16);
    } else {
        this->d_manufacturerId = 0x00;
        this->d_uniqueId = 0x0000;
    }
    strcpy(this->message, message.c_str());
}

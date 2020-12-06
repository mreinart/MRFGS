//
// Packet
//

#include "Packet.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <stdio.h>

using namespace std;
using namespace rapidjson;
using namespace fanet;

string Packet::toString() {
    char str[250];
    char textStr[260];
    memset(textStr, 0, 260);
    if (fanetMAC.type == 2) {
        strncpy(textStr, rawMessage.message + 4, rawMessage.m_length - 4);
        sprintf(str, "Name: %02X:%04X - '%s'",
                fanetMAC.s_manufactur_id, fanetMAC.s_unique_id, textStr);
        return string(str);
    } else if (fanetMAC.type == 3) {
        if (fanetMAC.e_header) {
            strncpy(textStr, rawMessage.message + 9, rawMessage.m_length - 9);
            sprintf(str, "Message: %02X:%04X->%02X:%04X - '%s'",
                    fanetMAC.s_manufactur_id, fanetMAC.s_unique_id,
                    fanetMAC.d_manufactur_id, fanetMAC.d_unique_id,
                    textStr);
        } else {
            strncpy(textStr, rawMessage.message + 5, rawMessage.m_length - 5);
            sprintf(str, "Message: %02X:%04X->XX:XXXX - '%s'",
                    fanetMAC.s_manufactur_id, fanetMAC.s_unique_id, textStr);
        }
        return string(str);
    } else {
        sprintf(str, "[%02X:%04X->%02X:%04X typ:%d len:%d]",
            fanetMAC.s_manufactur_id, fanetMAC.s_unique_id,
            fanetMAC.d_manufactur_id, fanetMAC.d_unique_id,
            fanetMAC.type,
            rawMessage.m_length);
        stringstream ss;
        ss << str;
        char hexStr[1000];
        for (int i = 0; i < rawMessage.m_length; i++) {
            sprintf((hexStr + 3 * i), " %02X", rawMessage.message[i]);
        }
        ss << "[" << hexStr << " ]";
        return string(ss.str());
    }
}

string Packet::toRawString() {
    struct tm *timeinfo;
    timeinfo = localtime((const long int *) &radioData.timestamp);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    stringstream ss;
    //ss << "RAW: ";
    char radioStr[100];
    sprintf(radioStr, "[RSSI: %d/%d %03.1f %s]",
            //timeBuf, //radioData.timestamp,
            radioData.rssi, radioData.prssi, radioData.psnr, radioData.coding_rate);
    ss << radioStr;
    char str[250];
    sprintf(str, "[%02X:%04X->%02X:%04X typ:%d len:%d]",
            fanetMAC.s_manufactur_id, fanetMAC.s_unique_id,
            fanetMAC.d_manufactur_id, fanetMAC.d_unique_id,
            fanetMAC.type,
            rawMessage.m_length
    );
    ss << str;
    char hexStr[1000];
    for (int i = 0; i < rawMessage.m_length; i++) {
        sprintf((hexStr + 3 * i), " %02X", rawMessage.message[i]);
    }
    ss << "[" << hexStr << " ]";
    char textStr[260];
    memset(textStr, 0, 260);
    if (fanetMAC.type == 2) {
        strncpy(textStr, rawMessage.message + 4, rawMessage.m_length - 4);
        ss << "/Name:'" << textStr << "'";
    }
    if (fanetMAC.type == 3) {
        strncpy(textStr, rawMessage.message + 9, rawMessage.m_length - 9);
        ss << "/Msg:'" << textStr << "'";
    }
    return string(ss.str());
}

string Packet::toJson() {
    struct tm *timeinfo;
    timeinfo = localtime((const long int*)&radioData.timestamp);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    char hexStr[1000];
    for (int i = 0; i < rawMessage.m_length; i++)
        sprintf((hexStr + 3 * i), " %02X", rawMessage.message[i]);
    char textStr[260];
    memset(textStr, 0, 260);
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();
    {
        writer.Key("rawMsg");
        writer.StartObject();
        {
            writer.Key("datetime");
            writer.String(timeBuf);
            writer.Key("length");
            writer.Uint(this->rawMessage.m_length);
            writer.Key("hexMsg");
            writer.String(hexStr);
            char xbuf[10];
            writer.Key("source");
            sprintf(xbuf, "%02X:%04X", fanetMAC.s_manufactur_id, fanetMAC.s_unique_id);
            writer.String(xbuf);
            writer.Key("dest");
            sprintf(xbuf, "%02X:%04X", fanetMAC.d_manufactur_id, fanetMAC.d_unique_id);
            writer.String(xbuf);
            writer.Key("type");
            writer.Int(fanetMAC.type);
            if (fanetMAC.type == 2) {
                writer.Key("name");
                strncpy(textStr, rawMessage.message+4, rawMessage.m_length-4);
                writer.String(textStr);
            }
            if (fanetMAC.type == 3) {
                writer.Key("msg");
                strncpy(textStr, rawMessage.message+9, rawMessage.m_length-9);
                writer.String(textStr);
            }
            writer.Key("radioData");
            writer.StartObject();
            {
                writer.Key("timestamp");
                writer.Uint(this->radioData.timestamp);
                writer.Key("RSSI");
                writer.Int(this->radioData.rssi);
                writer.Key("pRSSI");
                writer.Int(this->radioData.prssi);
                writer.Key("pSNR");
                writer.Int(this->radioData.psnr);
                writer.Key("CR");
                writer.String(this->radioData.coding_rate);
                writer.Key("CRCERR");
                writer.Int(this->radioData.crc_err);
            }
            writer.EndObject();
        }
        writer.EndObject();
    }
    writer.EndObject();
    cout << sb.GetString() << endl;
    return sb.GetString();
}

Packet Packet::fromJson(std::string json) {
    Document d;
    d.Parse(json.c_str());
    Packet packet;
    /*
    Value &value = d["id"];
    packet.id = value.GetString();
    value = d["callsign"];
    string str = value.GetString();
    strcpy(packet.callsign, str.c_str());
    value = d["origin"];
    str = value.GetString();
    strcpy(packet.origin, str.c_str());
    value = d["timestamp"];
    packet.timestamp = value.GetInt();
    value = d["tracking"];
    packet.tracking = value.GetBool();
    packet.manufacturerId = strtol(d["manufacturerId"].GetString(), NULL, 16);
    packet.uniqueId = strtol(d["uniqueId"].GetString(), NULL, 16);
    value = d["latitude"];
    packet.latitude = value.GetFloat();
    value = d["longitude"];
    packet.longitude = value.GetFloat();
    value = d["altitude"];
    packet.altitude = value.GetFloat();
    value = d["course"];
    packet.heading = value.GetFloat();
    value = d["speed"];
    packet.speed = value.GetFloat();
     */
    return packet;
}


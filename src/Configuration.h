//
// Configuration
//

#ifndef FANET_GS_CONFIGURATION_H
#define FANET_GS_CONFIGURATION_H

#include <string>
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

namespace fanet {

    class Configuration {

    private:
        static Configuration *instance_;
        bool initialized_;
        string cfgToken_;
        rapidjson::Document jsonDoc;
        string jsonStr_;

        Configuration() : initialized_(false), cfgToken_("") {};

        Configuration(const Configuration &);

        Configuration &operator=(const Configuration &);

    public:

        static Configuration *getInstance() {
            if (instance_ == 0) {
                instance_ = new Configuration();
            }
            return instance_;
        }

        void init(string token);
        string getJson();

        Value *getValue(string path);

        bool getBoolValue(string path);
        bool getValue(string path, bool defaultValue);

        int getIntValue(string path);
        int getValue(string path, int defaultValue);
        float getValue(string path, float defaultValue);

        const char *getCharPtrValue(string path);
        string getStringValue(string path);
        string getValue(string path, string defaultValue);
        string getValue(string path, const char *defaultValue);

        string getValueForKey(string key);

        string getValueForKey(string group, string key);
    };
}

#endif //FANET_GS_CONFIGURATION_H

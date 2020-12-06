//
// EntityName
//

#ifndef DF_FANET_GS_ENTITYNAME_H
#define DF_FANET_GS_ENTITYNAME_H

#include <string>
#include <list>

using namespace std;

namespace fanet {

    class EntityName {
    public:
        EntityName() : id(""), name(""), shortName(""), manufacturerId(0), uniqueId(0) {};
        EntityName(string id, string name, string shortName, u_int8_t manufacturerId, u_int16_t uniqueId)
                : id(id), name(name), shortName(shortName), manufacturerId(manufacturerId), uniqueId(uniqueId) {};

        string id;
        string name;
        string shortName;
        u_int8_t manufacturerId;
        u_int16_t uniqueId;

        string toString();
    };

    class EntityNameManager {

    private:
        static EntityNameManager *instance_;
        bool initialized_;
        bool finished_;

        EntityNameManager() : initialized_(false), finished_(false) {};

    public:
        list <EntityName> nameList;

        static EntityNameManager *getInstance() {
            if (instance_ == 0) {
                instance_ = new EntityNameManager();
            }
            return instance_;
        }

        void init();
        void reinit();

        void run();
        void stop() { finished_ = true; };
    };

}

#endif //DF_FANET_GS_ENTITYNAME_H

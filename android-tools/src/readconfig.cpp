#include "readconfig.h"

std::string readFileConfigurationAndroid() {
    std::string ipWaydroid;
    std::ifstream readfileconf("config.cache"); 
    while (getline(readfileconf,ipWaydroid)) {
        return  ipWaydroid;
    }
    return "";
}

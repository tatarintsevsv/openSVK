#include "configcxx.h"
std::string xmlReadString(std::string path,std::string def){
    char* r = configReadString(path.c_str(),def.c_str());
    std::string res = std::string(r);
    free(r);
    return res;
};
int xmlReadInt(std::string path, int def){
     return configReadInt(path.c_str(), def);
};
std::string xmlReadPath(std::string path,std::string def){
    char* r = configReadPath(path.c_str(),def.c_str());
    std::string res = std::string(r);
    free(r);
    return res;
};



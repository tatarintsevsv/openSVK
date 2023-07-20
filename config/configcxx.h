#ifndef CONFIGCXX_H
#define CONFIGCXX_H
#include <stdbool.h>
#include <string>
#include "config.h"
std::string xmlReadString(std::string path,std::string def="");
int xmlReadInt(std::string path, int def=0);
std::string xmlReadPath(std::string path,std::string def="");

#endif // CONFIGCXX_H

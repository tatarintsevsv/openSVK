#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#define CONFIG_BUFSIZE 1024

//extern xmlDocPtr doc;
extern void configClose();
extern bool configInit(char* filename, char* facility);
extern char* configReadString(char* path,const char* def);
extern int configReadInt(char* path, int def);
extern char* configGetNodeSet(char* XPath);

#endif // CONFIG_H

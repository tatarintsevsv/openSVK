#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#define CONFIG_BUFSIZE 1024

//extern xmlDocPtr doc;
extern void configClose();
extern bool configInit(const char *filename, const char *facility);
extern char* configReadString(const char* path,const char* def);
extern int configReadInt(const char* path, int def);
extern char* configGetNodeSet(const char* XPath);
extern void configSetRoot(const char* path);

#endif // CONFIG_H

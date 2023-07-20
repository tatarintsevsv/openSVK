#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#define CONFIG_BUFSIZE 1024

#ifdef __cplusplus
#define ETYPE extern "C"
#else
#define ETYPE extern
#endif

//extern xmlDocPtr doc;
ETYPE void configClose();
ETYPE bool configInit(const char *filename, const char *facility);
ETYPE char* configReadString(const char* path,const char* def);
ETYPE char* configReadPath(const char* path,const char* def);
ETYPE int configReadInt(const char* path, int def);
ETYPE char* configGetNodeSet(const char* XPath);
ETYPE void configSetRoot(const char* path);

#endif // CONFIG_H

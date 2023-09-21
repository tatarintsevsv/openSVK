#include "config.h"
#include <assert.h>
#include <syslog.h>
#include <wordexp.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/c14n.h>
#include <libxml2/libxml/xmlschemas.h>
#include <libxml2/libxml/xmlschemastypes.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/xmlerror.h>
#include <libxml2/libxml/valid.h>
#include "string.h"
/**
 * @brief укзатель на открытый xml документ (файл конфигурации)
 */
xmlDocPtr doc=NULL;

/**
 * @brief Размер буфера по умолчанию
 */
#define BUFSIZE 1024

/**
 * @brief Узел конфигурации, считающийся корнем при чтении параметров
 */
char configRoot[BUFSIZE]={0};

char* configGetNodeSet(const char* XPath)
{    
    assert(XPath!=NULL);
    xmlXPathContextPtr context;
    char* res=malloc(BUFSIZE);
    memset(res,'\0',BUFSIZE);
    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        syslog(LOG_CRIT,"Ошибка xmlXPathNewContext");
        return res;
    }
    xmlChar *xpath = (xmlChar*)XPath;
    xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
        syslog(LOG_ERR,"Ошибка XPath: %s",xmlGetLastError()->message);
        return res;
    }
    if(result){
        xmlNodeSetPtr nodeset = result->nodesetval;
        switch (result->type) {
        case XPATH_NUMBER:
            sprintf (res, "%lf", result->floatval);
            break;
        case XPATH_BOOLEAN:
            sprintf (res, "%i", result->boolval);
            break;

        case XPATH_NODESET:
            if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
                xmlXPathFreeObject(result);
                return res;
            }
            for(int i=0;i<nodeset->nodeNr;i++){
                xmlNode* node = nodeset->nodeTab[i];
                switch (node->type) {
                case XML_ATTRIBUTE_NODE:{
                    xmlChar* xmlbuff;
                    xmlbuff = xmlNodeGetContent(node);
                    sprintf (res, "%s", (char*)xmlbuff);
                    xmlFree(xmlbuff);
                    break;
                    }
                case XML_ELEMENT_NODE:{
                    xmlBufferPtr bf = xmlBufferCreate();
                    xmlNodeDump(bf,doc,node,1,1);
                    sprintf (res, "%s", (char*)bf->content);
                    xmlBufferFree(bf);
                    break;
                    }
                 default: {//XML_TEXT_NODE
                    xmlBufferPtr bf = xmlBufferCreate();
                    xmlNodeDump(bf,doc,node,1,1);
                    sprintf (res, "%s", (char*)bf->content);
                    xmlBufferFree(bf);
                    break;
                    }
                }
            }
            break;
        case XPATH_STRING:
            sprintf (res, "%s", (char*)result->stringval);
        default:
            break;
        }
    }
    if(result)
        xmlXPathFreeObject (result);
    return res;
}

void configClose()
{
    if(doc!=NULL){
        xmlFreeDoc(doc);
        doc=NULL;
    }
    xmlCleanupParser();
}

void configSetRoot(const char *path){
    strcpy(configRoot,path);
}

bool configInit(const char* filename, const char* facility)
{
    assert(filename!=NULL);
    openlog((facility==NULL)?"SVK_MAIL":facility,LOG_CONS|LOG_PID,LOG_MAIL);
    xmlInitParser();
    doc = xmlReadFile(filename,NULL,0);
    if (doc == NULL){
        syslog(LOG_CRIT,"Ошибка чтения файла конфигурации %s",filename);
        return false;
    }
    return true;
}

char* configReadString(const char* path,const char* def)
{
    assert(path!=NULL);
    if(def==NULL)
        def="";
    char* xpath=malloc(BUFSIZE);
    sprintf (xpath, "%s%s",configRoot, path);
    char* set=NULL;
    set = configGetNodeSet(xpath);
    free(xpath);
    if(strstr(set,"$$$")==set){
        char* r = getenv(&set[3]);
        if(r!=NULL){
            set = realloc(set,strlen(r)+1);
            strcpy(set,r);
        }
    }

    if(set==NULL){
        set = malloc(strlen(def)+1);
        strcpy(set,def);
    }
    return set;
}

char* configReadPath(const char* path,const char* def)
{
    assert(path!=NULL);
    char* res = configReadString(path,def);
    wordexp_t exp_result;
    wordexp(res, &exp_result, 0);
    free(res);
    res = (char*)malloc(strlen(exp_result.we_wordv[0])+1);
    strcpy(res,exp_result.we_wordv[0]);
    wordfree(&exp_result);
    return res;
}

int configReadInt(const char* path, int def){
    assert(path!=NULL);
    char* val=configReadString(path,"");
    int res=def;
    if(strlen(val))
        res=atoi(val);
    free(val);
    return res;

}



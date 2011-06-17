/* error.h
 * Error handling for pngnq 
 */
#include "syslog.h"

/* Error codes */
#define PNGNQ_ERR_NONE 0
#define PNGNQ_ERR_ 0

#define PNGNQ_LOG_ERR(...)(syslog(LOG_ERR,\
    "pngnq - Error in %s near line %d:",__FILE__,__LINE__));\
    syslog(LOG_ERR, __VA_ARGS__); 
    
#define PNGNQ_LOG_WARNING(...)(syslog(LOG_WARNING,"pngnq - warning: "));\
    syslog(LOG_WARNING, __VA_ARGS__);

#define PNGNQ_ERROR(...) (fprintf(stderr,\
    "pngnq - Error in %s near line %d :\n",__FILE__,__LINE__));\
    fprintf(stderr, __VA_ARGS__);\
    PNGNQ_LOG_ERR(__VA_ARGS__)\
    fflush(stderr);

#define PNGNQ_WARNING(...)                                \
    do {                                                  \
        fprintf(stderr, "pngnq - Warning: " __VA_ARGS__); \
        PNGNQ_LOG_WARNING(__VA_ARGS__)                    \
        fflush(stderr);                                   \
    } while (0)

#define PNGNQ_MESSAGE(...) {if(verbose) {fprintf(stderr,__VA_ARGS__);fflush(stderr);}}

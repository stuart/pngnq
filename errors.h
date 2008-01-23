/* error.h
 * Error handling for pngnq 
 */
#include "syslog.h"

/* Error codes */
#define PNGNQ_ERR_NONE 0
#define PNGNQ_ERR_ 0

#define PNGNQ_LOG_ERR(m)(syslog(LOG_ERR,\
    "pngnq - error in %s near line %d: %s\n",__FILE__,__LINE__,(m)));
#define PNGNQ_LOG_WARN(m)(syslog(LOG_WARN,"pngnq - warning: %s\n",(m)));

#define PNGNQ_ERROR(m) (fprintf(stderr,\
    "pngnq - error in %s near line %d: %s\n",__FILE__,__LINE__,(m)));\
                  PNGNQ_LOG_ERR(m)\
                  fflush(stderr);

#define PNGNQ_WARN(m) (fprintf(stderr,"pngnq - warning: %s\n",(m)));\
                  PNGNQ_LOG_WARN(m)\
                  fflush(stderr);


/* error.h
 * Error handling for pngnq 
 */

/* Error codes */
#define PNGNQ_ERR_NONE 0
#define PNGNQ_ERR_ 0




#define PNGNQ_ERROR(m) (fprintf(stderr,"pngnq - error in %s near line %d: %s\n",__FILE__,__LINE__,(m)));\
                  fflush(stderr);

#define  PNGNQ_WARN(m) (fprintf(stderr,"pngnq - warning: %s\n",(m)));\
                  fflush(stderr);

 

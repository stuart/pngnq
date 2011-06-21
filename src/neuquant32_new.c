#include <math.h>
typedef nq_pixel{
    float a,b,g,r;
}

nq_pixel *network;

typedef image_pixel{
    unsigned char a,b,g,r;
}

image_pixel *picture;

float error(nq_pixel a, nq_pixel b){
    
}

void init_network(unsigned char *thepic, unsigned int len, unsigned int colours, double gamma_correction){
    
    if(network = malloc(nq_pixel * netsize) == NULL){
        PNGNQ_ERROR("Out of memory error.");
    } 
     
    
    for (i=0; i<netsize; i++) {
        network[i].b = network[i].g = network[i].r = gamma(i*256/netsize);
              
        /*  Sets alpha values at 0 for dark pixels. */
        if (i < 16) network[i].al = (i*16); else network[i].al = 255; 
        
        freq[i] = 1.0/netsize;  /* 1/netsize */
        bias[i] = 0;
    }
}

void learn(){
    
}


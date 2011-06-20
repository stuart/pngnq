#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix_3.h"

/* Test */
int main(int argc, char** argv){
    matrix_3 m, n, p;
    
    /* Seed random with first argument */
    if(argc > 1){
      srand(atof(argv[1]));
    }
    
    int i;
    
    for(i=0; i<9; i++){
        m[i] = rand()/(RAND_MAX+1.0);
        printf("%d %.10f\n",i, m[i]);
    }
    
    printf("inverting\n") ;
    m3_invert(m,n);
    
    for(i=0; i<9; i++){
        printf("%d %.10f\n",i, n[i]);
    }
    
    printf("check: should be the identity matrix.\n");
    
    m3_multiply(m,n,p);
    for(i=0; i<9; i++){
        printf("%d %.10f\n",i, p[i]);
    } 
    
    matrix_3 m_D65_rgb2XYZ ={ 0.4124564,  0.3575761,  0.1804375,
                              0.2126729,  0.7151522,  0.0721750,
                              0.0193339,  0.1191920,  0.9503041 };
    matrix_3 m_D65_invert;
    m3_invert(m_D65_rgb2XYZ, m_D65_invert);
    for(i=0; i<9; i++){
        printf("%d %.10f\n",i, m_D65_invert[i]);
    };
}
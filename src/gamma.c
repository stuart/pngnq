#include "gamma.h" 

float gamma_table[256];
float gamma_correction;

void init_gamma_table(double gamma){
    int i; 
    gamma_correction = gamma;
    for(i=0; i<256; i++){
        gamma_table[i] = pow(i/255.0, 1.0/gamma_correction);
    }
};

float gamma_correct(unsigned char value){
    return gamma_table[value];
};   

unsigned char inverse_gamma_correct(float value){
    float temp;
    temp = pow(value, gamma_correction);
    
    
} 

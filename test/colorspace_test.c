#include <unistd.h>
#include <stdio.h>

#include "seatest.h"
#include "colorspace.h"

/* Testing accuracy */
#define delta_xyz 0.0001
#define delta_luv 0.0001
#define delta_lab 0.0001


void conversion_test_rgb2XYZ(unsigned char r,unsigned char g,unsigned char b,float X,float Y,float Z){  
    color_rgb rgb; 
    color_XYZ xyz;
    
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(X,xyz.X,delta_xyz);
    assert_float_equal(Y,xyz.Y,delta_xyz);
    assert_float_equal(Z,xyz.Z,delta_xyz);
}

int test_rgb2xyz(){

    printf("Testing rgb to XYZ\n");
    
    /* pure rgb */
    conversion_test_rgb2XYZ(255,0,0,0.4124,0.2126,0.0193);
    conversion_test_rgb2XYZ(0,255,0,0.3576,0.7152,0.1192);
    conversion_test_rgb2XYZ(0,0,255,0.1805,0.0722,0.9503);

    /* black */
    conversion_test_rgb2XYZ(0,0,0,0.0,0.0,0.0);
    
    /* white = D65 whitepoint */
    conversion_test_rgb2XYZ(255,255,255,0.95047,1.00000,1.08883);

    /* Mid grey */
    conversion_test_rgb2XYZ(128,128,128,0.2051,0.2159,0.2350);
    
    /* Test that the function inverts correctly */
    color_rgb rgb, rgb2; 
        color_XYZ xyz;
        for(rgb.r=0;rgb.r<255;rgb.r+=1){
           for(rgb.g=0;rgb.g<255;rgb.g+=1){  
             for(rgb.b=0;rgb.b<255;rgb.b+=1){ 
                rgb2XYZ(&rgb,&xyz);
                XYZ2rgb(&xyz,&rgb2);
                assert_int_equal(rgb.r,rgb2.r);
                assert_int_equal(rgb.g,rgb2.g);
                assert_int_equal(rgb.b,rgb2.b);
             }
          }
          printf("."); fflush(stdout);
        } 
        printf("\n");                 
};

void conversion_test_rgb2LUV(unsigned char r,unsigned char g,unsigned char b,float L,float U,float V){
    color_rgb rgb;
    color_LUV luv;
    
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    
    rgb2LUV(&rgb,&luv);
    assert_float_equal(L,luv.L,delta_luv);
    assert_float_equal(U,luv.U,delta_luv);
    assert_float_equal(V,luv.V,delta_luv);
}

void test_rgb2LUV(){   
    printf("Testing rgb to LUV conversions.\n");
    /* Test some sample colors */
    conversion_test_rgb2LUV(0,0,0,0.0,0.0,0.0);
    conversion_test_rgb2LUV(255,255,255,100.0,0.0,0.0);
    conversion_test_rgb2LUV(255,0,0,53.240789,175.01499,37.756412);
    conversion_test_rgb2LUV(0,255,0,87.734720,-83.077561,107.398533);
    conversion_test_rgb2LUV(0,0,255,32.297009,-9.405405,-130.342344);
    conversion_test_rgb2LUV(128,128,128,53.585013,0.0,0.0);
    
    /* Test that the function inverts correctly */
    color_rgb rgb, rgb2; 
    color_LUV luv;

    for(rgb.r=0;rgb.r<255;rgb.r+=1){
       for(rgb.g=0;rgb.g<255;rgb.g+=1){  
         for(rgb.b=0;rgb.b<255;rgb.b+=1){ 
            rgb2LUV(&rgb,&luv);
            LUV2rgb(&luv,&rgb2);
            assert_int_equal(rgb.r,rgb2.r);
            assert_int_equal(rgb.g,rgb2.g);
            assert_int_equal(rgb.b,rgb2.b);
         }
      }
      printf("."); fflush(stdout);
    } 
    printf("\n");
    
}

void conversion_test_rgb2Lab(unsigned char r,unsigned char g,unsigned char b,float L,float a,float B){
    color_rgb rgb;
    color_Lab lab;
    
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    
    rgb2Lab(&rgb,&lab);
    assert_float_equal(L,lab.L,delta_lab);
    assert_float_equal(a,lab.a,delta_lab);
    assert_float_equal(B,lab.b,delta_lab);
}

void test_rgb2Lab(){   
    printf("Testing rgb to Lab conversions.\n");
    /* Test some sample colors */
    conversion_test_rgb2Lab(0,0,0,0.0,0.0,0.0);
    conversion_test_rgb2Lab(255,255,255,100.0,0.0,0.0);
    conversion_test_rgb2Lab(255,0,0,53.240789,175.01499,37.756412);
    conversion_test_rgb2Lab(0,255,0,87.734720,-83.077561,107.398533);
    conversion_test_rgb2Lab(0,0,255,32.297009,-9.405405,-130.342344);
    conversion_test_rgb2Lab(128,128,128,53.585013,0.0,0.0);
    
    /* Test that the function inverts correctly */
    color_rgb rgb, rgb2; 
    color_Lab lab;

    for(rgb.r=0;rgb.r<255;rgb.r++){
       for(rgb.g=0;rgb.g<255;rgb.g++){  
         for(rgb.b=0;rgb.b<255;rgb.b++){ 
            rgb2Lab(&rgb,&lab);
            Lab2rgb(&lab,&rgb2); 
            assert_int_equal(rgb.r,rgb2.r);
            assert_int_equal(rgb.g,rgb2.g);
            assert_int_equal(rgb.b,rgb2.b);
         }
      }
      printf("."); fflush(stdout);
    } 
    printf("\n");
    
}

void test_using_chrominance(){
    struct chrominance chrm;
    
    // Setup chrominance with Adobe color space
    chrm.r_x = 0.6400;
    chrm.r_y = 0.3300;
    chrm.g_x = 0.2100;
    chrm.g_y = 0.7100;
    chrm.b_x = 0.1500;
    chrm.b_y = 0.0600;
    chrm.w_x = 0.3127;
    chrm.w_y = 0.3290;
    chrm.gamma = 22;
    
    init_colorspace(&chrm);
    conversion_test_rgb2XYZ(0,0,0,0.0,0.0,0.0);
    conversion_test_rgb2XYZ(255,255,255,0.950456, 1.000000, 1.089058);
    conversion_test_rgb2XYZ(255,0,0,0.5766,0.2973,0.027);
    
}

void test_colorspace( void )
{
	test_fixture_start();
	//init_colorspace(NULL);      
	//run_test(test_rgb2xyz);
   // run_test(test_rgb2Lab);
    //run_test(test_rgb2LUV);
    run_test(test_using_chrominance);
	test_fixture_end();       
}



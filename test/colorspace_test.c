#include <unistd.h>
#include <stdio.h>

#include "seatest.h"
#include "colorspace.h"

#define delta_xyz 0.0001

int test_rgb2xyz(){
    color_rgb rgb; 
    color_XYZ xyz;
    printf("Testing rgb to XYZ\n");
    
    rgb.r = 255;
    rgb.g = 0;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.4124,xyz.X,delta_xyz);
    assert_float_equal(0.2126,xyz.Y,delta_xyz);
    assert_float_equal(0.0193,xyz.Z,delta_xyz); 
    
    rgb.r = 0;
    rgb.g = 255;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.3576,xyz.X,delta_xyz);
    assert_float_equal(0.7152,xyz.Y,delta_xyz);
    assert_float_equal(0.1192,xyz.Z,delta_xyz);
    
    rgb.r = 0;
    rgb.g = 0;
    rgb.b = 255;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.1805,xyz.X,delta_xyz);
    assert_float_equal(0.0722,xyz.Y,delta_xyz);
    assert_float_equal(0.9503,xyz.Z,delta_xyz);
    
    rgb.r = 0;
    rgb.g = 0;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.0000,xyz.X,delta_xyz);
    assert_float_equal(0.0000,xyz.Y,delta_xyz);
    assert_float_equal(0.0000,xyz.Z,delta_xyz);
    
    rgb.r = 255;
    rgb.g = 255;
    rgb.b = 255;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.9505,xyz.X,delta_xyz);
    assert_float_equal(1.0000,xyz.Y,delta_xyz);
    assert_float_equal(1.0888,xyz.Z,delta_xyz);
    
    rgb.r = 128;
    rgb.g = 128;
    rgb.b = 128;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.2052,xyz.X,delta_xyz);
    assert_float_equal(0.2159,xyz.Y,delta_xyz);
    assert_float_equal(0.2350,xyz.Z,delta_xyz);
    
    /* Test that the function inverts correctly */
    color_rgb rgb2;
    for(rgb.r=0;rgb.r<252;rgb.r+=1){
       for(rgb.g=0;rgb.g<252;rgb.g+=1){  
         for(rgb.b=0;rgb.b<252;rgb.b+=1){ 
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


void test_colorspace( void )
{
	test_fixture_start();
	init_colorspace(NULL);      
	run_test(test_rgb2xyz);
	test_fixture_end();       
}



#include <unistd.h>
#include <stdio.h>

#include "seatest.h"
#include "colorspace.h"


int test_rgb2xyz(){
    color_rgb rgb; 
    color_XYZ xyz;
    printf("Testing rgb to XYZ\n");
    
    rgb.r = 255;
    rgb.g = 0;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.4124,xyz.X,0.0001);
    assert_float_equal(0.2126,xyz.Y,0.0001);
    assert_float_equal(0.0193,xyz.Z,0.0001); 
    
    rgb.r = 0;
    rgb.g = 255;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.3576,xyz.X,0.0001);
    assert_float_equal(0.7152,xyz.Y,0.0001);
    assert_float_equal(0.1192,xyz.Z,0.0001);
    
    rgb.r = 0;
    rgb.g = 0;
    rgb.b = 255;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.1805,xyz.X,0.0001);
    assert_float_equal(0.0722,xyz.Y,0.0001);
    assert_float_equal(0.9503,xyz.Z,0.0001);
    
    rgb.r = 0;
    rgb.g = 0;
    rgb.b = 0;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.0000,xyz.X,0.0001);
    assert_float_equal(0.0000,xyz.Y,0.0001);
    assert_float_equal(0.0000,xyz.Z,0.0001);
    
    rgb.r = 255;
    rgb.g = 255;
    rgb.b = 255;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.9505,xyz.X,0.0001);
    assert_float_equal(1.0000,xyz.Y,0.0001);
    assert_float_equal(1.0888,xyz.Z,0.0001);
    
    rgb.r = 128;
    rgb.g = 128;
    rgb.b = 128;
    rgb2XYZ(&rgb,&xyz);
    assert_float_equal(0.2052,xyz.X,0.0001);
    assert_float_equal(0.2159,xyz.Y,0.0001);
    assert_float_equal(0.2350,xyz.Z,0.0001);
    
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


//
// put the test into a fixture...
//
void test_fixture_one( void )
{
	test_fixture_start();
	init_colorspace(NULL);      
	run_test(test_rgb2xyz);   
	test_fixture_end();       
}


//
// put the fixture into a suite...
//
void all_tests( void )
{
    test_fixture_one();   
}

//
// run the suite!
//
int main( int argc, char** argv )
{       
    return run_tests(all_tests);
}


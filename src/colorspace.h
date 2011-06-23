#ifndef _COLORSPACE_H_INC
#define _COLORSPACE_H_INC
/* colorspace.h 
   Defines conversions between various colorspaces
*/

typedef struct {
    unsigned char r,g,b,a;
} color_rgba;

/* 8 bit rgb */
typedef struct {
  unsigned char r, g, b;
} color_rgb;

typedef struct {
  float X,Y,Z;
} color_XYZ;

typedef struct {
  float L,U,V;
} color_LUV;

typedef struct {
  float L,a,b;
} color_Lab;

/* Chrominance  from cHRM and gAMA chunks */
struct chrominance{
    float w_x;
    float w_y;
    float r_x;
    float r_y;
    float g_x;
    float g_y;
    float b_x;
    float b_y;
    float gamma;
};

struct chrominance chrm_AdobeRGB = {
    0.3127,
    0.3290,
    0.6400,
    0.3300,
    0.2100,
    0.7100,
    0.1500,
    0.0600,
    2.2 
};

struct chrominance chrm_WideGamut = {
    0.3457,
    0.3585,
    0.7347,
    0.2653,
    0.1152,
    0.8264,
    0.1566,
    0.0177,
    2.2
} 

/* This colorspace is designed for 16bit and may
   not work well in 8bit */
struct chrominance chrm_ProPhoto = {
    0.3457,
    0.3585,
    0.7347,
    0.2653,
    0.1596,
    0.8404,
    0.0366,
    0.0001,
    1.8
}

/* Initialize the color space with a chrominance structure 
   This must be called before any of the conversion functions are used.
   If chrm = NULL a srgb colorspace with D65 whitepoint is assumed. */
void init_colorspace(struct chrominance *chrm);

/* Convert rgb to a CIE XYZ color (CIE 1931 XYZ). 
   Result is stored in xyz.
*/
void rgb2XYZ(const color_rgb *rgb, color_XYZ *xyz);

/* Convert an XYZ color to an LUV color (CIE 1976(L*,u*v*)) 
   Result is stored in luv */
void XYZ2LUV(const color_XYZ *xyz, color_LUV *luv);

/* Convert an LUV color to an XYZ color. Result is stored in xyz */
void LUV2XYZ(const color_LUV *luv, color_XYZ *xyz);

/* Convert an XYZ color to an Lab color (CIE 1976 (L*, a*, b*))
   Result is stored in lab */
void XYZ2Lab(const color_XYZ *xyz, color_Lab *lab);

/* Convert an Lab color to XYZ. Result is stored in xyz */
void Lab2XYZ(const color_Lab *lab, color_XYZ *xyz);

/* Convert rgb to a CIE XYZ color. Result is stored in luv */
void rgb2LUV(const color_rgb *rgb, color_LUV *luv);

/* Convert rgb to a CIE Lab color. Result is stored in lab */
void rgb2Lab(const color_rgb *rgb, color_Lab *lab);

/* Convert CIE Lab to an rgb color. Result is stored in rgb. */
void Lab2rgb(const color_Lab *lab, color_rgb *rgb);

#endif
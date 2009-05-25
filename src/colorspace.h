/* colorspace.h 
   Defines conversions between various colorspaces
*/

typedef struct {
  unsigned char r, g, b;
} color_rgb;

typedef struct {
  float X,Y,Z;
} color_XYZ;

typedef struct {
  float L,U,V;
} color_LUV;


/* Standard white points 
   These are all defined in the CIE XYZ colorspace. 
*/


/* If the whitepoint is passed as NULL d65 is the default */
static color_XYZ d65 = {0.94810,1.0000,1.07305};
static color_XYZ d00 = {0.0, 0.0, 0.0};

/* Convert rgb to a CIE XYZ color, using the supplied white point 
   result is stored in xyz 
*/
void rgb2XYZ(const color_rgb *rgb, color_XYZ *xyz, const color_XYZ *wp);

/* Convert an XYZ color to an LUV color 
   result is stored in luv */
void XYZ2LUV(color_XYZ *xyz, color_LUV *luv, const color_XYZ *wp);

/* Convert rgb to a CIE XYZ color, using the supplied white point 
   result is stored in luv */
void rgb2LUV(const color_rgb *rgb, color_LUV *luv, const color_XYZ *wp);



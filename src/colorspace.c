
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#include "colorspace.h"
#include "matrix_3.h" 
#include "math.h"

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
static const double e = 216.0/24389.0;
static const double k = 24389.0/27.0;

/* Storage for conversion matrices */
static matrix_3 colorspace_m_rgb2XYZ;
static matrix_3 colorspace_m_XYZ2rgb;
static color_XYZ colorspace_wp;
static int colorspace_initialized;

/* Standard D65 conversions */
const matrix_3 m_D65_rgb2XYZ ={ 0.4124564,  0.3575761,  0.1804375,
                                0.2126729,  0.7151522,  0.0721750,
                                0.0193339,  0.1191920,  0.9503041};

const matrix_3 m_D65_XYZ2rgb ={ 3.2404542, -1.5371385, -0.4985314,
                               -0.9692660,  1.8760108,  0.0415560,
                                0.0556434, -0.2040259,  1.0572252};

static color_XYZ d65 = {0.95047, 1.00, 1.08883};
static color_XYZ d00 = {0.0, 0.0, 0.0};
                          	              
/* White point in XYZ space from chrominance data */
void wp_from_chrominance(struct chrominance *chrm, color_XYZ *wp){
   wp->X = chrm->w_x/chrm->w_y;
   wp->Y = 1.0;
   wp->Z = (1 - chrm->w_x - chrm->w_y)/chrm->w_y;
}

/* Get a matrix for conversion from linear rgb to XYZ color space */
void rgb2xyz_matrix(struct chrominance *chrm, matrix_3 M){
    matrix_3 n, s;
    
    color_XYZ *wp = &colorspace_wp;
    
    matrix_3 a = { chrm->r_x/chrm->r_y, chrm->g_x/chrm->g_y, chrm->b_x/chrm->b_y,
                  1.0, 1.0, 1.0,
                  (1-chrm->r_x-chrm->r_y)/chrm->r_y,
                  (1-chrm->g_x-chrm->g_y)/chrm->g_y,   
                  (1-chrm->b_x-chrm->b_y)/chrm->b_y};
    
    m3_invert(a,n);
    
    /* S matrix (diagonal) */
    s[0] = n[0]*wp->X+n[1]*wp->Y+n[2]*wp->Z;
    s[4] = n[3]*wp->X+n[4]*wp->Y+n[5]*wp->Z;
    s[8] = n[6]*wp->X+n[7]*wp->Y+n[8]*wp->Z;
    
    m3_multiply(s,a,M);
}

/* Get a matrix for conversion from XYZ to linear rgb space 
   We simply invert the rgvb2XYZ matrix.
*/
void xyz2rgb_matrix(struct chrominance *chrm, matrix_3 M){
    matrix_3 temp;

    rgb2xyz_matrix(chrm, temp);
    m3_invert(temp,M);
}

/* linearize rgb values into srgb values */
float c_linear(float c_srgb){
   float result;
   if(c_srgb > 0.0405){
         result = pow(((c_srgb + 0.055)/1.055),2.4);
      }else{
         result = c_srgb/12.92;
      }
   return result;  
}

/* de-linearize srgb values */
float c_srgb(float c_linear){
  if(c_linear > 0.0031308){
          return(1.055 * pow(c_linear, 1.0/2.4) - 0.055);
      }else{
          return c_linear * 12.92;
      }
}

/* Convert from rgb to XYZ colorspac//e */
/* Note this assumes a D65 whitepoint */
void rgb2XYZ(const color_rgb *rgb, color_XYZ *xyz)
{
  double r,g,b;
  r = c_linear((float)rgb->r/255.0);
  g = c_linear((float)rgb->g/255.0);
  b = c_linear((float)rgb->b/255.0);

  xyz->X = colorspace_m_rgb2XYZ[0]*r + colorspace_m_rgb2XYZ[1]*g + colorspace_m_rgb2XYZ[2]*b;
  xyz->Y = colorspace_m_rgb2XYZ[3]*r + colorspace_m_rgb2XYZ[4]*g + colorspace_m_rgb2XYZ[5]*b;
  xyz->Z = colorspace_m_rgb2XYZ[6]*r + colorspace_m_rgb2XYZ[7]*g + colorspace_m_rgb2XYZ[8]*b;

}

/* Convert from XYZ space to rgb space */
void XYZ2rgb(const color_XYZ *xyz,color_rgb *rgb){
    double r,g,b;
    
    r = colorspace_m_XYZ2rgb[0]*xyz->X + colorspace_m_XYZ2rgb[1]*xyz->Y + colorspace_m_XYZ2rgb[2]*xyz->Z;
    g = colorspace_m_XYZ2rgb[3]*xyz->X + colorspace_m_XYZ2rgb[4]*xyz->Y + colorspace_m_XYZ2rgb[5]*xyz->Z;
    b = colorspace_m_XYZ2rgb[6]*xyz->X + colorspace_m_XYZ2rgb[7]*xyz->Y + colorspace_m_XYZ2rgb[8]*xyz->Z;
    
    rgb->r = round(c_srgb(r) * 255.0);
    rgb->g = round(c_srgb(g) * 255.0);
    rgb->b = round(c_srgb(b) * 255.0);
}

/* Convert from XYZ to LUV colorspace */
void XYZ2LUV(const color_XYZ *xyz, color_LUV *luv)
{
  float u,v;
  float yref,uref,vref;

  if(xyz->Y == 0.0){
      
  }
  /* Reference white point */ 
  color_XYZ *wp = &colorspace_wp;
  
  uref = 4.0 * wp->X/(wp->X + 15.0 * wp->Y +3.0 * wp->Z); 
  vref = 9.0 * wp->Y/(wp->X + 15.0 * wp->Y +3.0 * wp->Z); 
  yref = xyz->Y/wp->Y;

  /* Calculate LUV */
  u = 4.0 * xyz->X/(xyz->X + 15.0 * xyz->Y + 3.0 * xyz->Z);
  v = 9.0 * xyz->Y/(xyz->X + 15.0 * xyz->Y + 3.0 * xyz->Z);
  
  if(yref > e){
    luv->L = 116.0 * powf(yref,1.0/3.0)-16.0; 
  }else{
    luv->L = k*yref;
  }
  
  if(isnan(u)){
      /* catch the case where X=Y=Z=0 */
      luv->U = 0.0;
  }else{
      luv->U = 13.0*luv->L*(u-uref); 
  }
  
  if(isnan(v)){
      luv->V = 0.0;
  }else{ 
    luv->V = 13.0*luv->L*(v-vref);
  }
}

void LUV2XYZ(const color_LUV *luv, color_XYZ *xyz){
    
    float uref,vref;
    float a,b,c,d;
    
    /* Reference white point */ 
    color_XYZ *wp = &colorspace_wp;
    
    uref = (4.0 * wp->X)/(wp->X + 15.0 * wp->Y + 3.0 * wp->Z);
    vref = (9.0 * wp->Y)/(wp->X + 15.0 * wp->Y + 3.0 * wp->Z);   
    
    if(luv->L > k*e){
        xyz->Y = pow((luv->L+16.0)/116.0,3);
    }else{
        xyz->Y = luv->L/k;
    }

    a = ((52.0 * luv->L)/(luv->U + 13.0 * luv->L * uref)-1.0)/3.0;    
    b = -5.0 * xyz->Y;
    c = -1.0/3.0;
    d = xyz->Y * ((39.0*luv->L)/(luv->V + 13.0 * luv->L * vref)-5.0);
    xyz->X = (d-b)/(a-c);
    xyz->Z = xyz->X * a + b;
    
}

/* Lab f value function */
float f(float t){
    if(t > e){
        return cbrt(t);
    }else{
        return(t*29*29/(3*6*6)+4.0/29.0);
    }           
}

/* Convert from XYZ space to Lab */
void XYZ2Lab(const color_XYZ *xyz, color_Lab *lab){
    /* Reference white point */ 
    color_XYZ *wp = &colorspace_wp;
    
    lab->L = 116.0 * f(xyz->Y/wp->Y) - 16.0;
    lab->a = 500.0 * (f(xyz->X/wp->X) - f(xyz->Y/wp->Y));
    lab->b = 200.0 * (f(xyz->Y/wp->Y) - f(xyz->Z/wp->Z));   
}

float f_inv(float t){
    if(t > 6.0/29.0){
       return(t*t*t);
    }else{
       return(3*6*6*(t-4.0/29.0)/(29*29));
    }
}

/* Convert from lab to XYZ */
void Lab2XYZ(const color_Lab *lab, color_XYZ *xyz){
    
    double x,y,z;
 
    x = (lab->L+16.0)/116.0 + lab->a/500.0;
    z = (lab->L+16.0)/116.0 - lab->b/200.0;
    y = (lab->L+16.0)/116.0;
    
    xyz->X = colorspace_wp.X * f_inv(x);
    xyz->Y = colorspace_wp.Y * f_inv(y);
    xyz->Z = colorspace_wp.Z * f_inv(z);
}

void rgb2LUV(const color_rgb *rgb, color_LUV *luv)
{
  color_XYZ xyz;
  rgb2XYZ(rgb,&xyz);
  XYZ2LUV(&xyz,luv);
}

void LUV2rgb(const color_LUV *luv, color_rgb *rgb)
{
    color_XYZ xyz;
    LUV2XYZ(luv,&xyz);
    XYZ2rgb(&xyz,rgb);
}

void rgb2Lab(const color_rgb *rgb, color_Lab *lab){
    color_XYZ xyz;
    rgb2XYZ(rgb,&xyz);
    XYZ2Lab(&xyz,lab);
}

void Lab2rgb(const color_Lab *lab, color_rgb *rgb){
     color_XYZ xyz;
     Lab2XYZ(lab, &xyz);
     XYZ2rgb(&xyz,rgb);
}


void init_colorspace(struct chrominance *chrm){
  /* Set defaults to D65 whitepoint if no chrominance data. */
  int i;
  if(chrm == NULL){
    colorspace_wp.X = d65.X;
    colorspace_wp.Y = d65.Y;
    colorspace_wp.Z = d65.Z;
    
    for(i=0; i<9; i++){
      colorspace_m_XYZ2rgb[i] = m_D65_XYZ2rgb[i];
      colorspace_m_rgb2XYZ[i] = m_D65_rgb2XYZ[i];
    }

  }else{   
      wp_from_chrominance(chrm,&colorspace_wp);  
      xyz2rgb_matrix(chrm, colorspace_m_XYZ2rgb);  
      rgb2xyz_matrix(chrm, colorspace_m_rgb2XYZ);
  }
  
  colorspace_initialized = 1; 
}

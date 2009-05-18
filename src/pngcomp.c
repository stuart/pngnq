/* pngcomp.c

A simple test program for png image quantisation. 
  
This program compares the pixel colors of two images and prints out 
statistics on the differences between the images. 

Statistics printed include:
mean error
standard deviation of error
maximum error
	     
The error is calculated as the linear distance between two colors in RGBA space.

** Copyright (C) 2006-2007 by Stuart Coyle
** 
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/



#define PNGCOMP_USAGE "usage: pngcomp [-vVh] image1.png image2.png\n\
  options: v - verbose, does nothing as yet.\n\
           V - version, prints version information.\n\
           h - help, prionts this message.\n\
           b - Block size in pixels. This is the length of the block side.\n\
           R - Use RGBA colorspace to calculate errors.\n\
           L - Use LUVA colorspace to calculate errors.\n\
  inputs: image1.png and image2.png are the two images that are to be compared.\n\
          it is required that they be the same size.\n\
\n\
  This program give some basic statistics about the difference between two images.\n\
  It was created as a measure of various color quantization methods.\n\
\n\
  The statistics given include individual pixel differences and also\n\
  block statistics averaged over blocks of pixels. The latter is a better measure\n\
  when images have been dithered.\n\
\n\
  The use of these statistics is limited in that they do not contain a model of human vision."
  
#define MAX_COLOR_VAL 256
#define SQRT_3 1.73205

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

#include "png.h"
#include "config.h"
#include "rwpng.h"
#include "colorspace.h" 

#if HAVE_GETOPT 
  #include <unistd.h>
#else
  #include "../freegetopt/getopt.h"
#endif

typedef struct {
  uch r, g, b, a;
} pixel;

struct statistics {
  char *colorspace;
  double max_error;
  double mean_error;
  double stddev_error;
  ulg  n_pixels;
  ulg correct_pixels;
};

struct blockstats {
  char* colorspace;
  int blocksize;
  double max_error;
  double mean_error;
  double stddev_error;
  ulg n_blocks;
};
 

/* Image information structs */
static mainprog_info image1_info;
static mainprog_info image2_info;

float *imagediff(char* file1_name, char* file2_name);
float *block_imagediff(char* file1_name, char* file2_name,int blocksize);
struct statistics *gather_stats(float *error_data);
struct blockstats *gather_block_stats(float *block_error_data, int blocksize);
void printstats(struct statistics* stats, struct blockstats* bstats);
float LUVerrval(pixel *p1, pixel *p2);
float RGBerrval(pixel *p1, pixel *p2);

/* Error value callback */
typedef float errval_t(pixel *, pixel *);
errval_t* errval;

int main(int argc, char** argv)
{
  int verbose = 0;
  int blocksize = 16;

  char *file1_name = NULL;
  char *file2_name = NULL;

  int c; /* argument count */

  int retval = 0;
  float* err_image = NULL;
  float* block_err_image = NULL;
  errval = RGBerrval;
  char *colorspace = "RGBA";

  /* Parse arguments */
  if(argc==1){
    fprintf(stderr,PNGCOMP_USAGE);
    exit(EXIT_SUCCESS);
  }

  while((c = getopt(argc,argv,"hVvb:RL"))!=-1){
    switch(c){
    case 'v':
      verbose = 1;
      break;
    case 'V':
      fprintf(stderr,"pngcomp %s\n",VERSION);
      rwpng_version_info();
      exit(EXIT_SUCCESS);
      break;
    case 'h':
      fprintf(stderr,PNGCOMP_USAGE);
      exit(EXIT_SUCCESS);
      break;
    case 'b':
      blocksize = atoi(optarg);
      break;
    case 'R':
      errval = RGBerrval;
      colorspace = "RGBA";
      break;
    case 'L':
      errval = LUVerrval;
      colorspace ="LUVA";
      break;
    case '?':      
      if (isprint(optopt))
	fprintf (stderr, "  unknown option `-%c'.\n", optopt);
      else
	fprintf (stderr,
		 "  unknown option character `\\x%x'.\n",
		 optopt);
    default:
      fprintf(stderr,PNGCOMP_USAGE);
      exit(EXIT_FAILURE);
    }
  }


  /* determine input files */
  if(optind == argc){
    fprintf(stderr,"  pngcomp requires two input file names.\n");
    exit(EXIT_FAILURE);
  }
  else{
    file1_name=argv[optind];
    optind++;
    if(optind == argc){
      fprintf(stderr,"  pngcomp requires two file names.\n");
      exit(EXIT_FAILURE);
    }
    else{
      file2_name=argv[optind];
      optind++;
    }
  }

  err_image = imagediff(file1_name,file2_name);
  block_err_image = block_imagediff(file1_name,file2_name,blocksize);
  if(err_image != NULL){
    struct statistics *stats = gather_stats(err_image);
    struct blockstats *bstats = gather_block_stats(err_image,blocksize);
    stats->colorspace = colorspace;
    bstats->colorspace = colorspace;   
    printstats(stats,bstats);
  }

  exit(retval);

}


float *imagediff(char* file1_name, char* file2_name){
  
  FILE *file1 = NULL;
  FILE *file2 = NULL;  
    
  ulg cols, rows;
  ulg row;

  float* error_data = NULL;

  /* Open the image files */
  if((file1 = fopen(file1_name, "rb"))==NULL){
    fprintf(stderr,"  error: cannot open %s for reading.\n",file1_name);
    fflush(stderr);
    return NULL;
  }

  if((file2 = fopen(file2_name, "rb"))==NULL){
    fprintf(stderr,"  error: cannot open %s for reading.\n",file2_name);
    fflush(stderr);
    return NULL;
  }

  /* Read each image */
  rwpng_read_image(file1,&image1_info);
  fclose(file1);
  if (image1_info.retval) {
    fprintf(stderr, "  rwpng_read_image() error\n");
    fflush(stderr);
    return NULL; 
  }

  rwpng_read_image(file2,&image2_info);
  fclose(file2);
  if (image2_info.retval) {
    fprintf(stderr, "  rwpng_read_image() error\n");
    fflush(stderr);
    return NULL; 
  }
 
  /* Can't do images that differ in size */
  /* Is there any point? */
  cols = image1_info.width;
  rows= image1_info.height;

  if(image2_info.width != cols || image2_info.height != rows){
    fprintf(stderr, "  images differ in size. cannot continue. \n");
    return(NULL);
  }
  
   
  if(!image1_info.rgba_data || !image2_info.rgba_data)
    {
      fprintf(stderr,"  no pixel data found.");
      return(NULL);
    }

  error_data = (float *)calloc(cols*rows*sizeof(float),sizeof(float));
  if(error_data == NULL){
    fprintf(stderr,"  cannot allocate error buffer.");
    return(NULL);
  }

  /* Calculate error value for each pixel */
  for(row=0;(ulg)row < rows; ++row){
    int col;
    pixel p1, p2;
    ulg offset = row*cols*4;

    for( col=0;(ulg)col<cols;++col){
      p1.r = image1_info.rgba_data[col*4+offset];
      p1.g = image1_info.rgba_data[col*4+offset+1];
      p1.b = image1_info.rgba_data[col*4+offset+2];
      p1.a = image1_info.rgba_data[col*4+offset+3];

      p2.r = image2_info.rgba_data[col*4+offset];
      p2.g = image2_info.rgba_data[col*4+offset+1];
      p2.b = image2_info.rgba_data[col*4+offset+2];
      p2.a = image2_info.rgba_data[col*4+offset+3];

      error_data[col*row] = errval(&p1,&p2);
      
    }
  }
  
  return error_data;
}

float *block_imagediff(char* file1_name, char* file2_name,int blocksize){
  
  FILE *file1 = NULL;
  FILE *file2 = NULL;  
    
  ulg cols, rows;
  ulg row;

  float* block_error_data = NULL;

  /* Open the image files */
  if((file1 = fopen(file1_name, "rb"))==NULL){
    fprintf(stderr,"  error: cannot open %s for reading.\n",file1_name);
    fflush(stderr);
    return NULL;
  }

  if((file2 = fopen(file2_name, "rb"))==NULL){
    fprintf(stderr,"  error: cannot open %s for reading.\n",file2_name);
    fflush(stderr);
    return NULL;
  }

  /* Read each image */
  rwpng_read_image(file1,&image1_info);
  fclose(file1);
  if (image1_info.retval) {
    fprintf(stderr, "  rwpng_read_image() error\n");
    fflush(stderr);
    return NULL; 
  }

  rwpng_read_image(file2,&image2_info);
  fclose(file2);
  if (image2_info.retval) {
    fprintf(stderr, "  rwpng_read_image() error\n");
    fflush(stderr);
    return NULL; 
  }
 
  /* Can't do images that differ in size */
  /* Is there any point? */
  cols = image1_info.width;
  rows= image1_info.height;

  if(image2_info.width != cols || image2_info.height != rows){
    fprintf(stderr, "  images differ in size. cannot continue. \n");
    return(NULL);
  }
  
   
  if(!image1_info.rgba_data || !image2_info.rgba_data)
    {
      fprintf(stderr,"  no pixel data found.");
      return(NULL);
    }

  block_error_data = (float *)calloc(cols*rows*sizeof(float),sizeof(float));
  if(block_error_data == NULL){
    fprintf(stderr,"  cannot allocate block error buffer.");
    return(NULL);
  }

  /* Do block errors */ 
  for(row=0;(ulg)row+blocksize < rows; row+=blocksize){
    int col;
    pixel p1, p2;
    ulg offset = row*cols*4;
  
    for(col=0;(ulg)col+blocksize<cols; col+= blocksize){
      int blockrow,blockcol;
      for(blockrow=0;blockrow<blocksize;blockrow++){
	offset += blockrow*4;
	for(blockcol=0;blockcol<blocksize;blockcol++){
	  p1.r = image1_info.rgba_data[(col+blockcol)*4+offset];
	  p1.g = image1_info.rgba_data[(col+blockcol)*4+offset+1];
	  p1.b = image1_info.rgba_data[(col+blockcol)*4+offset+2];
	  p1.a = image1_info.rgba_data[(col+blockcol)*4+offset+3];

	  p2.r = image2_info.rgba_data[(col+blockcol)*4+offset];
	  p2.g = image2_info.rgba_data[(col+blockcol)*4+offset+1];
	  p2.b = image2_info.rgba_data[(col+blockcol)*4+offset+2];
	  p2.a = image2_info.rgba_data[(col+blockcol)*4+offset+3];
		
	  block_error_data[col*row] = errval(&p1,&p2);

	} 
	
      }
      block_error_data[col*row] /= (float)(blocksize*blocksize);
    }
  }

  return block_error_data;
}


/* Calculates cartesian distance of pixels in rgba space */
float RGBerrval(pixel *p1, pixel *p2){
  long err;
 
  long err_r = p1->r-p2->r;
  long err_g = p1->g-p2->g;
  long err_b = p1->b-p2->b;
  long err_a = p1->a-p2->a;
  err = err_r*err_r + err_g*err_g + err_b*err_b+ err_a*err_a;
  return sqrt((double)err);
}

/* Calculates cartesian distance of pixels in LUVa space */
float LUVerrval(pixel *p1, pixel *p2){

  color_LUV c1;
  color_LUV c2;
  
  color_rgb r1;
  color_rgb r2;
  
  r1.r = p1->r;
  r1.g = p1->g;
  r1.b = p1->b;
  
  r2.r = p2->r;
  r2.g = p2->g;
  r2.b = p2->b;
  
  rgb2LUV(&r1,&c1,NULL);
  rgb2LUV(&r2,&c2,NULL);
  long err;
 
  long err_L = c1.L-c2.L;
  long err_u = c1.U-c2.U;
  long err_v = c1.V-c2.V;
  long err_a = p1->a-p2->a;
  err = err_L*err_L + err_u*err_u + err_v*err_v+ err_a*err_a;
  return sqrt((double)err);  
}


struct statistics *gather_stats(float *error_data){

  int count;
  struct statistics *stats = malloc(sizeof(struct statistics));
  
  if(stats == NULL){
    fprintf(stderr,"  Cannot allocate statistics struct.");
    return(NULL);
  }
  
 
  stats->max_error = 0.0;
  stats->mean_error = 0.0;
  stats->stddev_error = 0.0;
  stats->n_pixels = image1_info.width*image1_info.height;
  stats->correct_pixels = (ulg)0;

  /* Basic stats */
  for(count=0;count<stats->n_pixels;count++){
    float err = error_data[count];
    if(err > stats->max_error) stats->max_error = err;
    stats->mean_error += err;
    if(err <= 0.0) stats->correct_pixels++;
  }
  stats->mean_error = (float)(stats->mean_error)/(float)(stats->n_pixels);
  
  /* Standard deviation */
 for(count=0;count<stats->n_pixels;count++){
    double err= error_data[count];
    stats->stddev_error += (err-stats->mean_error)*(err-stats->mean_error);
 }
 stats->stddev_error = sqrt(stats->stddev_error)/stats->n_pixels;
 
 return stats;

}


struct blockstats *gather_block_stats(float *block_error_data,int blocksize){

  int count;
  struct blockstats *stats = malloc(sizeof(struct blockstats));
  
  if(stats == NULL){
    fprintf(stderr,"  Cannot allocate block statistics struct.");
    return(NULL);
  }
  
  stats->blocksize = blocksize;
  stats->max_error = 0.0;
  stats->mean_error = 0.0;
  stats->stddev_error = 0.0;
  stats->n_blocks = image1_info.width*image1_info.height/(blocksize*blocksize);
 
  /* Basic stats */
  for(count=0;count<stats->n_blocks;count++){
    float err = block_error_data[count*blocksize*blocksize];
    if(err > stats->max_error) stats->max_error = err;
    stats->mean_error += err;
  }
  stats->mean_error = (float)(stats->mean_error)/(float)(stats->n_blocks);

  /* Standard deviation */
 for(count=0;count<stats->n_blocks;count++){
    double err= block_error_data[count*blocksize*blocksize];
    stats->stddev_error += (err-stats->mean_error)*(err-stats->mean_error);
 }
 stats->stddev_error = sqrt(stats->stddev_error)/(stats->n_blocks);
 
 return stats;

}


void printstats(struct statistics* stats, struct blockstats* bstats){
  printf("%s image color difference statistics.\n",stats->colorspace);
  printf("Mean pixel color error: %f \n",stats->mean_error);
  printf("Maximum pixel color error: %f \n",stats->max_error);
  printf("Standard Deviation of error: %f\n",stats->stddev_error);
  printf("Image Dimensions %ld x %ld \n",image1_info.width,image1_info.height);
  printf("Number of pixels: %ld \n",stats->n_pixels);
  printf("Number of exact pixels: %ld\n",stats->correct_pixels);
  printf("Percentage correct pixels: %f\n",(float)stats->correct_pixels/(float)stats->n_pixels*100.0);
  printf("\n");
  printf("%s image color block difference statistics.\n",bstats->colorspace);
  printf("Blocksize %d x %d = %d pixels\n",bstats->blocksize,bstats->blocksize,
	 bstats->blocksize*bstats->blocksize);
  printf("Mean block color error: %f \n",bstats->mean_error);
  printf("Maximum block color error: %f \n",bstats->max_error);
  printf("Standard Deviation of block error: %f\n",bstats->stddev_error);
  printf("Total number of blocks: %ld\n",bstats->n_blocks);
}


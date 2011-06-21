/* pngnq.c - quantize the colors in an alphamap down to 256 using 
**  the Neuquant algorithm.
**
** Based on Greg Roelf's pngquant which was itself based on Jef Poskanzer's ppmquant.
** Uses Anthony Dekker's Neuquant algorithm extended to handle the alpha channel.
** Rewritten by Kornel Lesiński (2009)

**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
** Copyright (C) 1997, 2000, 2002 by Greg Roelofs; based on an idea by
**                                Stefan Schneider.
** Copyright (C) 2004-2009 by Stuart Coyle
** Copyright (C) Kornel Lesiński (2009)

** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/* NeuQuant Neural-Net Quantization Algorithm
 * ------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 *
 */ 


#define PNGNQ_VERSION VERSION //"0.9 ($Date: 2009-01-25 22:39:19 +0000 (Sun, 25 Jan 2009) $)"

#define FNMAX 1024
#define PNGNQ_USAGE "\
Usage:  pngnq [-fhvV][-d dir][-e ext.][-g gamma][-n colours][-Q dither][-s speed][input files]\n\
Options:\n\
   -n Number of colours the quantized image is to contain. Range: 16 to 256. Defaults to 256.\n\
   -d Directory to put quantized images into.\n\
   -e Specify the new extension for quantized files. Default -nq8.png\n\
   -f Force ovewriting of files.\n\
   -g Image gamma. 1.0 = linear, 2.2 = monitor gamma. Defaults to 1.8.\n\
   -h Print this help.\n\n\
   -Q Quantization: n = no dithering (default), f = floyd-steinberg\n\
   -s Speed/quality: 1 = slow, best quality, 3 = good quality, 10 = fast, lower quality.\n\
   -v Verbose mode. Prints status messages.\n\
   -V Print version number and library versions.\n\
   input files: The png files to be processed. Defaults to standard input if not specified.\n\n\
\
  Quantizes a 32-bit RGBA PNG image to an 8 bit RGBA palette PNG\n\
  using the neuquant algorithm. The output file name is the input file name\n\
  extended with \"-nq8.png\"\n"

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> /* isprint() and features.h */

#if HAVE_GETOPT
#  include <unistd.h>
#else
#  include "../freegetopt/getopt.h"
#endif

#if HAVE_VALGRIND_H
# include <valgrind.h>
#endif

#if defined(WIN32) || defined(MSDOS)	
#  include <fcntl.h>	/* O_BINARY */
#  include <io.h>	/* setmode() */
#  define DIR_SEPARATOR_CHAR		'\\'
#  define DIR_SEPARATOR_STR		"\\"
#endif

#ifndef DIR_SEPARATOR_CHAR
#  define DIR_SEPARATOR_CHAR		'/'
#  define DIR_SEPARATOR_STR		"/"
#endif  

#include "png.h"
#include "neuquant32.h"
#include "rwpng.h"
#include "errors.h"

typedef struct {
  uch r, g, b, a;
} pixel;

/* Image information struct */
static mainprog_info rwpng_info;


static int pngnq(char* filename, char* newext, char* dir,
		 int sample_factor, int n_colors, int verbose,  
		 int using_stdin, int force, int use_floyd, double force_gamma);

int main(int argc, char** argv)
{
  int verbose = 0;
  int force = 0;
  int sample_factor = 0; /* will be set depending on image size */

  char *input_file_name = NULL;
  char *output_file_extension = "-nq8.png";
  char *output_directory = NULL;

  int using_stdin = FALSE;
  int c; /* argument character */

  int errors = 0, file_count =0;
  int retval;
  int n_colours = 256; /* number of colours to quantize to. Default 256 */
  int use_floyd = 0;

  double force_gamma = 0;

  /* Parse arguments */
  while((c = getopt(argc,argv,"hVvfn:s:d:e:g:Q:"))!=-1){
    switch(c){
    case 's':
      sample_factor = atoi(optarg);
      break;
    case 'v':
      verbose = 1;
      break;
    case 'f':
      force = 1;
      break;
    case 'V':
      verbose = 1;
      PNGNQ_MESSAGE("pngnq %s\n",PNGNQ_VERSION);
      rwpng_version_info();
      exit(EXIT_SUCCESS);
      break;
    case 'h':
      fprintf(stderr,PNGNQ_USAGE);
      exit(EXIT_SUCCESS);
      break;
    case 'n':
      n_colours = atoi(optarg);
      if(n_colours > 256){
	      PNGNQ_WARNING("  -n option requested %d colors.\n  PNG indexed images cannot contain more than 256 colours.\n  Setting the number of colours to 256!\n",n_colours);
	      n_colours = 256;
      }else if(n_colours<2){
	      PNGNQ_WARNING("  -n option requested %d colors, which is silly.\n  Setting number of colors to the minimum value of 1!\n",n_colours);
	      n_colours = 1;      
      }
      break;
    case 'g':
      force_gamma = atof(optarg); 
        if (force_gamma <= 0.001 || force_gamma > 10.0) 
        {
            PNGNQ_WARNING("Gamma %s doesn't make sense. Setting to 1.0\n",optarg);
            force_gamma=1.0;
        }
      break; 
    case 'Q':
      if (optarg[0] == 'f') use_floyd = 1;
         else if (optarg[0] == 'n') use_floyd = 0;
            else PNGNQ_WARNING("There's no quantization method %s\n",optarg);
      break;
    case 'd':
      output_directory = optarg;
      break;
    case 'e':
      output_file_extension = optarg;
      break;
    default:
      fprintf(stderr,PNGNQ_USAGE);
      exit(EXIT_FAILURE);
    }
  }


  PNGNQ_MESSAGE("Using quantization method %d", use_floyd);
  

  /* determine input files */
  if(optind == argc){
    using_stdin = TRUE;
    input_file_name = "stdin";
  }
  else{
    input_file_name=argv[optind];
    optind++;
  }
		
  /* Process each input file */
  while(optind<=argc)
  {
   
    PNGNQ_MESSAGE("  quantizing: %s \n",input_file_name);
		
    retval = pngnq(input_file_name, output_file_extension, output_directory,
		   sample_factor, n_colours, verbose, using_stdin,force,use_floyd,force_gamma);

    if(retval){
      errors++;
    }
		       
    input_file_name=argv[optind];
    file_count++;
    optind++;
  }

 
  if (errors)
	 { 
	     PNGNQ_MESSAGE("There were errors quantizing %d file%s out of a total of %d file%s.\n",
             errors, (errors == 1)? "" : "s",file_count, (file_count == 1)? "" : "s");
  }
  else
  {
     PNGNQ_MESSAGE("No errors detected while quantizing %d image%s.\n",
             file_count, (file_count == 1)? "" : "s");
  }
  exit(errors);
}


/* Creates an output file name based on the input file, extension and directory requested */
char *createoutname(char *infilename, char* newext, char *newdir){

  char *outname = NULL;
  int fn_len, ext_len, dir_len = 0;
  char *loc;

  outname = malloc(FNMAX);
  if (!outname){
    PNGNQ_ERROR("  out of memory, cannot allocate output file name\n");
    exit(EXIT_FAILURE);
  }

  fn_len = strlen(infilename); 
  ext_len = strlen(newext)+1; /* include the terminating NULL*/
 
  if(newdir){
    dir_len = strlen(newdir);
    if(dir_len+fn_len+ext_len > FNMAX){
      PNGNQ_WARNING("  directory name too long, ignoring -d option\n");
      dir_len = 0;
      newdir=NULL;
    }
  }

  if(newdir){
    /* find the last directory separator */
    loc = strrchr(infilename, DIR_SEPARATOR_CHAR);
    if(loc){
      /* strip original directory from filename */
      infilename = loc+1;           
      fn_len = strlen(infilename);
    }
	 
    /* add a separator to newdir if needed */
    if(newdir && newdir[dir_len-1] != DIR_SEPARATOR_CHAR){
      strncpy(newdir+dir_len,DIR_SEPARATOR_STR,1);
      dir_len++;
    }
    /* copy new directory name to output */
    strncpy(outname,newdir,dir_len);
  }

  if (fn_len > FNMAX-ext_len-dir_len) {
    PNGNQ_WARNING("  base filename [%s] will be truncated\n", infilename);
    fn_len = FNMAX-ext_len;
  }

  /* copy filename to output */
  strncpy(outname+dir_len,infilename,fn_len);

  /* Add extension */
  if (strncmp(outname+dir_len+fn_len-4, ".png", 4) == 0) 
    strncpy(outname+dir_len+fn_len-4, newext, ext_len);
  else
    strncpy(outname+dir_len+fn_len, newext, ext_len);
  
  return(outname);
}


static void remap_floyd(int cols, int rows, unsigned char map[MAXNETSIZE][4], unsigned int* remap,  uch **row_pointers, int quantization_method)
{    
    uch *outrow = NULL; /* Output image pixels */

    int i,row;
    #define CLAMP(a) ((a)>=0 ? ((a)<=255 ? (a) : 255)  : 0)      

    /* Do each image row */
    for ( row = 0; (ulg)row < rows; ++row ) {
        int offset, nextoffset;
        outrow = rwpng_info.interlaced? row_pointers[row] :
        rwpng_info.indexed_data;
    
        int rederr=0;
        int blueerr=0;
        int greenerr=0;
        int alphaerr=0;
        
        offset = row*cols*4;
        nextoffset = offset; if (row+1<rows) nextoffset += cols*4;        
        int increment = 4; 
        
        if (0)//row&1)
        {
            offset += cols*4 - 4;
            nextoffset += cols*4 - 4;
            increment = -4;
        }
        
        for( i=0;i<cols;i++, offset+=increment, nextoffset+=increment)
        {
            int idx;
            unsigned int floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr;
            
            idx = inxsearch(CLAMP(rwpng_info.rgba_data[offset+3] - alphaerr),
                            CLAMP(rwpng_info.rgba_data[offset+2] - blueerr),
                            CLAMP(rwpng_info.rgba_data[offset+1] - greenerr),
                            CLAMP(rwpng_info.rgba_data[offset]   - rederr  ));                
                                    
            outrow[increment > 0 ? i : cols-i-1] = remap[idx];            
            
            int alpha = MAX(map[idx][3],rwpng_info.rgba_data[offset+3]);
            int colorimp = 255 - ((255-alpha) * (255-alpha) / 255);         
                
            int thisrederr=(map[idx][0] -   rwpng_info.rgba_data[offset]) * colorimp   / 255; 
            int thisblueerr=(map[idx][1] - rwpng_info.rgba_data[offset+1]) * colorimp  / 255; 
            int thisgreenerr=(map[idx][2] -  rwpng_info.rgba_data[offset+2]) * colorimp  / 255;
            int thisalphaerr=map[idx][3] - rwpng_info.rgba_data[offset+3];         
            
            rederr += thisrederr;
            greenerr += thisblueerr;
            blueerr +=  thisgreenerr;
            alphaerr += thisalphaerr;
            
            unsigned int thiserr = (thisrederr*thisrederr + thisblueerr*thisblueerr + thisgreenerr*thisgreenerr + thisalphaerr*thisalphaerr)*2;
             floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr;
            
            int L = 10;
            while (rederr*rederr > L*L || greenerr*greenerr > L*L || blueerr*blueerr > L*L || alphaerr*alphaerr > L*L ||
                   floyderr > thiserr || floyderr > L*L*2)
            {            
                rederr /=2;greenerr /=2;blueerr /=2;alphaerr /=2;
                floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr; 
            }
            
            if (i>0)
            {
                rwpng_info.rgba_data[nextoffset-increment+3]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+3] - alphaerr*3/16);
                rwpng_info.rgba_data[nextoffset-increment+2]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+2] - blueerr*3/16 );
                rwpng_info.rgba_data[nextoffset-increment+1]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+1] - greenerr*3/16);
                rwpng_info.rgba_data[nextoffset-increment]  =CLAMP(rwpng_info.rgba_data[nextoffset-increment]   - rederr*3/16  );           
            }
            if (i+1<cols)
            {
                rwpng_info.rgba_data[nextoffset+increment+3]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+3] - alphaerr/16); 
                rwpng_info.rgba_data[nextoffset+increment+2]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+2] - blueerr/16 ); 
                rwpng_info.rgba_data[nextoffset+increment+1]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+1] - greenerr/16);
                rwpng_info.rgba_data[nextoffset+increment]  =CLAMP(rwpng_info.rgba_data[nextoffset+increment]   - rederr/16  );           
            }
            rwpng_info.rgba_data[nextoffset+3]=CLAMP(rwpng_info.rgba_data[nextoffset+3] - alphaerr*5/16); 
            rwpng_info.rgba_data[nextoffset+2]=CLAMP(rwpng_info.rgba_data[nextoffset+2] - blueerr*5/16 ); 
            rwpng_info.rgba_data[nextoffset+1]=CLAMP(rwpng_info.rgba_data[nextoffset+1] - greenerr*5/16);
            rwpng_info.rgba_data[nextoffset]  =CLAMP(rwpng_info.rgba_data[nextoffset]   - rederr*5/16  );                   
        }
        
        rederr = rederr*7/16; greenerr =greenerr*7/16; blueerr =blueerr*7/16; alphaerr =alphaerr*7/16; 
      
        /* if non-interlaced PNG, write row now */
        if (!rwpng_info.interlaced)
            rwpng_write_image_row(&rwpng_info);
    }
    
}

static void remap_simple(unsigned int cols, unsigned int rows, unsigned char map[MAXNETSIZE][4], unsigned int* remap,  uch **row_pointers)
{
    uch *outrow = NULL; /* Output image pixels */
    
    unsigned int i,row;
    /* Do each image row */
    for ( row = 0; (ulg)row < rows; ++row ) 
    {
        unsigned int offset;
        outrow = rwpng_info.interlaced? row_pointers[row] :
        rwpng_info.indexed_data;
        /* Assign the new colors */
        offset = row*cols*4;
        for( i=0;i<cols;i++){
            outrow[i] = remap[inxsearch(rwpng_info.rgba_data[i*4+offset+3],
                                        rwpng_info.rgba_data[i*4+offset+2],
                                        rwpng_info.rgba_data[i*4+offset+1],
                                        rwpng_info.rgba_data[i*4+offset])];
        }
        
        /* if non-interlaced PNG, write row now */
        if (!rwpng_info.interlaced)
            rwpng_write_image_row(&rwpng_info);
    }
    
    
}


static void set_binary_mode(FILE *fp)
{
#if defined(MSDOS) || defined(FLEXOS) || defined(OS2) || defined(WIN32)
#if (defined(__HIGHC__) && !defined(FLEXOS))
    setmode(fp, _BINARY);
#else
    setmode(fp == stdout ? 1 : 0, O_BINARY);
#endif
#endif
}

static int pngnq(char* filename, char* newext, char* newdir, 
		 int sample_factor, int n_colours, int verbose, 
		 int using_stdin, int force, int quantization_method, double force_gamma)
{
  char *outname = NULL;
  FILE *infile = NULL;
  FILE *outfile = NULL;

  int bot_idx, top_idx; /* for remapping of indices */
  unsigned int remap[MAXNETSIZE];

  ulg cols, rows;
  ulg row;
  unsigned char map[MAXNETSIZE][4];
  int x;
  uch **row_pointers=NULL; /* Pointers to rows of pixels */
  int newcolors = n_colours;

  double file_gamma;
  double quantization_gamma;
    
  if(using_stdin)
  {	
    set_binary_mode(stdin);
    infile=stdin;
  }

  /* Open input file. */
  else{
    if((infile = fopen(filename, "rb"))==NULL){
      PNGNQ_ERROR("  Cannot open %s for reading.\n",filename);
      return 14;
    }
  }
  
  /* Read input file */
  rwpng_read_image(infile, &rwpng_info);
  if (!using_stdin)
    fclose(infile);

  if (rwpng_info.retval) {
    PNGNQ_ERROR("  rwpng_read_image() error: %d\n", rwpng_info.retval);
    if (!using_stdin)
      fclose(outfile);
    return(rwpng_info.retval); 
  }
  
  /* Open output file */  
  if(using_stdin)
  { 
    set_binary_mode(stdout);
    outfile = stdout;
  }
  else
  { 
    outname = createoutname(filename,newext,newdir);

    if (!force) {
      if ((outfile = fopen(outname, "rb")) != NULL) {
	      PNGNQ_ERROR("  %s exists, not overwriting. Use -f to force.\n",outname);
	fclose(outfile);
	return 15;
      }
    }
 
    if ((outfile = fopen(outname, "wb")) == NULL) {
      PNGNQ_ERROR("  Cannot open %s for writing\n", outname);
      return 16;
    }
  }

  cols = rwpng_info.width;
  rows = rwpng_info.height;

  if(!rwpng_info.rgba_data)
    {
       PNGNQ_WARNING("  no pixel data found.");
    }
   
   
   if (force_gamma > 0)
   {
       quantization_gamma = force_gamma;
       file_gamma=0;
   }else if(rwpng_info.have_srgb){
       /* we ignore the file gamma and use the sRGB standard instead */
       quantization_gamma = 0.45455;
   }
   else if(rwpng_info.have_gamma && file_gamma > 0)
   {          
       quantization_gamma = file_gamma;
   }
   else
   {
      quantization_gamma = 1.8;
      file_gamma = 0;
   }

  
   if (file_gamma > 0) {
       PNGNQ_MESSAGE("Using image gamma %e (1/%e)\n", file_gamma, 1.0/file_gamma);
       }
     else { 
       PNGNQ_MESSAGE("Assuming gamma %1.4f (1/%1.1f)\n",quantization_gamma,1.0/quantization_gamma);   
       }     
    
    if (sample_factor<1)
    {
        sample_factor = 1 + rows*cols / (512*512);
        if (sample_factor > 10) sample_factor = 10;
        
        if (sample_factor > 1)
        {
	    PNGNQ_MESSAGE("Sampling 1//%d of image\n", sample_factor);
        }
    }
    

  /* Start neuquant */
  initnet((unsigned char*)rwpng_info.rgba_data,rows*cols*4,newcolors,quantization_gamma);
  learn(sample_factor,verbose);
  inxbuild(); 
  getcolormap((unsigned char*)map);

  /* Remap indexes so all tRNS chunks are together */
  PNGNQ_MESSAGE("  Remapping colormap to eliminate opaque tRNS-chunk entries...\n");
  
  for (top_idx = newcolors-1, bot_idx = x = 0;  x < newcolors;  ++x) {
    if (map[x][3] == 255) /* maxval */
      remap[x] = top_idx--;
    else
      remap[x] = bot_idx++;
  }
  
  PNGNQ_MESSAGE( "%d entr%s left\n", bot_idx,(bot_idx == 1)? "y" : "ies");
  

  /* sanity check:  top and bottom indices should have just crossed paths */
  if (bot_idx != top_idx + 1) {
    PNGNQ_WARNING("  Internal logic error: remapped bot_idx = %d, top_idx = %d\n",bot_idx, top_idx);
    if (rwpng_info.row_pointers)
      free(rwpng_info.row_pointers);
    if (rwpng_info.rgba_data)
      free(rwpng_info.rgba_data);
    if (!using_stdin)
      fclose(outfile);
    return 18;
  }

  rwpng_info.sample_depth = 8;
  rwpng_info.num_palette = newcolors;
  rwpng_info.num_trans = bot_idx;
 
  /* GRR TO DO:  if bot_idx == 0, check whether all RGB samples are gray
     and if so, whether grayscale sample_depth would be same
     => skip following palette section and go grayscale */
     
  /* Remap and make palette entries */
  for (x = 0; x < newcolors; ++x) {
    rwpng_info.palette[remap[x]].red  = map[x][0];
    rwpng_info.palette[remap[x]].green = map[x][1];
    rwpng_info.palette[remap[x]].blue = map[x][2];
    rwpng_info.trans[remap[x]] = map[x][3];
  }
 
  /* Allocate memory*/
  if (rwpng_info.interlaced) {
    if ((rwpng_info.indexed_data = (uch *)malloc(rows * cols)) != NULL) {
      if ((row_pointers = (uch **)malloc(rows * sizeof(uch *))) != NULL) 				
        for (row = 0;  (ulg)row < rows;  ++row)
	  row_pointers[row] = rwpng_info.indexed_data + row*cols;
    }
  } else rwpng_info.indexed_data = (uch *)malloc(cols);
	
  if (rwpng_info.indexed_data == NULL ||
      (rwpng_info.interlaced && row_pointers == NULL))
    {
      PNGNQ_ERROR(" Insufficient memory for indexed data and/or row pointers\n");
      if (rwpng_info.row_pointers)
	free(rwpng_info.row_pointers);
      if (rwpng_info.rgba_data)
	free(rwpng_info.rgba_data);
      if (rwpng_info.indexed_data)
	free(rwpng_info.indexed_data);
      if (!using_stdin)
	fclose(outfile);
      return 17;
    }	

  /* Write headers and such. */
  if (rwpng_write_image_init(outfile, &rwpng_info) != 0) {
    PNGNQ_ERROR("  rwpng_write_image_init() error\n" );
    if (rwpng_info.rgba_data)
      free(rwpng_info.rgba_data);
    if (rwpng_info.row_pointers)
      free(rwpng_info.row_pointers);
    if (rwpng_info.indexed_data)
      free(rwpng_info.indexed_data);
    if (row_pointers)
      free(row_pointers);
    if (!using_stdin)
      fclose(outfile);
    return rwpng_info.retval;
  }
    
    if (quantization_method > 0)
    {
        remap_floyd(cols,rows,map,remap,row_pointers, quantization_method);        
    }
    else
    {
        remap_simple(cols,rows,map,remap,row_pointers);
    }
    
  /* now we're done with the INPUT data and row_pointers, so free 'em */
  if (rwpng_info.rgba_data) {
    free(rwpng_info.rgba_data);
    rwpng_info.rgba_data = NULL;
  }
  if (rwpng_info.row_pointers) {
    free(rwpng_info.row_pointers);
    rwpng_info.row_pointers = NULL;
  }

  /* write entire interlaced palette PNG, or finish/flush noninterlaced one */
  if (rwpng_info.interlaced) {
    rwpng_info.row_pointers = row_pointers;   /* now for OUTPUT data */
    rwpng_write_image_whole(&rwpng_info);
  } else rwpng_write_image_finish(&rwpng_info);

  if (!using_stdin)
    fclose(outfile);

  /* now we're done with the OUTPUT data and row_pointers, too */
  if (rwpng_info.indexed_data) {
    free(rwpng_info.indexed_data);
    rwpng_info.indexed_data = NULL;
  }
  if (row_pointers) {
    free(row_pointers);
    row_pointers = rwpng_info.row_pointers = NULL;
  }

  /* Clean up file name */
  if(outname){
    free(outname);
    outname = NULL;
  }

  return 0;
}



/* pngnq.c - quantize the colors in an alphamap down to 256 using 
**  the Neuquant algorithm.
**
** Based on Greg Roelf's pngquant which was itself based on Jef Poskanzer's ppmquant.
** Uses Anthony Dekker's Neuquant algorithm extended to handle the alpha channel.
** 
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
** Copyright (C) 1997, 2000, 2002 by Greg Roelofs; based on an idea by
**                                Stefan Schneider.
** Copyright (C) 2004-2007 by Stuart Coyle
** 
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

#define FNMAX 1024
#define PNGNQ_USAGE "\
  pngnq - png image quantization\n\
  usage:  pngnq [-vfhV][-n colours][input files]\n\
          use -h for more help and options.\n"
#define PNGNQ_HELP "\
  usage:  pngnq [-vfhV][-e extension][-d dir][-s sample factor][-n colours][input files]\n\
  options:\n\
     -v Verbose mode. Prints status messages.\n\
     -f Force ovewriting of files.\n\
     -s Sample factor. The neuquant algorithm samples pixels stepping by this value.\n\
     -n Number of colours the quantized image is to contain. Range: 2 to 256. Defaults to 256.\n\
     input files: The png files to be processed. Defaults to standard input if not specified.\n\n\
     -e Specify the new extension for quantized files. Default -nq8.png\n\
     -d Directory to put quantized images into.\n\
     -V Print version number and library versions.\n\
     -h Print this help.\n\n\
  Quantizes a 32-bit RGBA PNG image to an 8 bit RGBA palette PNG\n\
  using the neuquant algorithm. The output file name is the input file name\n\
  extended with \"-nq8.png\"\n"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> /* isprint() */
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#ifdef HAS_HEADER_VALGRIND
#include <valgrind/callgrind.h>
#endif
#ifdef WIN32
#include "freegetopt/getopt.h"
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
#include "config.h"             
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
		 int using_stdin, int force);

int main(int argc, char** argv)
{
  int verbose = 0;
  int force = 0;
  int sample_factor = 3; /* This is a reasonable default */

  char *input_file_name = NULL;
  char *output_file_extension = "-nq8.png";
  char *output_directory = NULL;

  int using_stdin = FALSE;
  int c; /* argument count */

  int errors = 0, file_count =0;
  int retval;
  int n_colours = 256; /* number of colours to quantize to. Default 256 */

  /* Parse arguments */
  while((c = getopt(argc,argv,"hVvfn:s:d:e:"))!=-1){
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
      fprintf(stderr, "pngnq %s\n",VERSION);
      rwpng_version_info();
      exit(EXIT_SUCCESS);
      break;
    case 'h':
      fprintf(stderr,PNGNQ_HELP);
      exit(EXIT_SUCCESS);
      break;
    case 'n':
      n_colours = atoi(optarg);
      if(n_colours > 256){
	fprintf (stderr, "  -n option requested %d colors.\n  PNG indexed images cannot contain more than 256 colours.\n  Setting the number of colours to 256!\n",n_colours);
	n_colours = 256;
      }else if(n_colours<2){
	fprintf(stderr,"  -n option requested %d colors, which is silly.\n  Setting number of colors to the minimum value of 1!\n",n_colours);
	n_colours = 1;      
      }
      break;
    case 'd':
      output_directory = optarg;
      break;
    case 'e':
      output_file_extension = optarg;
      break;
    case '?': 
      if (isprint(optopt))
	fprintf (stderr, "  unknown option or missing argument `-%c'.\n", optopt);
      else
	fprintf (stderr,
		 "  unknown option character `\\x%x'.\n",
		 optopt);
    default:
      fprintf(stderr,PNGNQ_USAGE);
      exit(EXIT_FAILURE);
    }
  }

  if(argc == 1){
      fprintf(stderr,PNGNQ_USAGE);
  }

  if(sample_factor<1){
    fprintf (stderr, "  sample factor must be 1 or greater. Default is 3.\n");
    exit(EXIT_FAILURE);
  }

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
  while(optind<=argc){

    if(verbose){
      fprintf(stderr,"  quantizing: %s \n",input_file_name);
      fflush(stderr);
    }
		
    retval = pngnq(input_file_name, output_file_extension, output_directory,
		   sample_factor, n_colours, verbose, using_stdin,force);

    if(retval){ 
      errors++;
    }
		       
    input_file_name=argv[optind];
    file_count++;
    optind++;
  }

  if (verbose) {
    if (errors)
      fprintf(stderr, "There were errors quantizing %d file%s out of a"
	      " total of %d file%s.\n",
	      errors, (errors == 1)? "" : "s",
	      file_count, (file_count == 1)? "" : "s");
    else
      fprintf(stderr, "No errors detected while quantizing %d image%s.\n",
	      file_count, (file_count == 1)? "" : "s");
    fflush(stderr);
  }

  exit(errors);

}


/* Creates an output file name based on the input file, extension and directory requested */
char *create_output_name(char *infilename, char* newext, char *newdir){

  char *outname = NULL;
  int fn_len, ext_len, dir_len = 0;
  char *loc;

  outname = malloc(FNMAX);
  if (!outname){
    fprintf(stderr,
	    "  error:  out of memory, cannot allocate output file name\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  fn_len = strlen(infilename); 
  ext_len = strlen(newext)+1; /* include the terminating NULL*/
 
  if(newdir){
    dir_len = strlen(newdir);
    if(dir_len+fn_len+ext_len > FNMAX){
      fprintf(stderr,
	      "  warning:  directory name too long, ignoring -d option\n");
      fflush(stderr);
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
    if(newdir && newdir[dir_len] != DIR_SEPARATOR_CHAR){
      strncpy(newdir+dir_len,DIR_SEPARATOR_STR,1);
      dir_len++;
    }
    /* copy new directory name to output */
    strncpy(outname,newdir,dir_len);
  }

  if (fn_len > FNMAX-ext_len-dir_len) {
    fprintf(stderr,
	    "  warning:  base filename [%s] will be truncated\n", infilename);
    fflush(stderr);
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
 
void free_rwpng_info() {
      if (rwpng_info.row_pointers)
	free(rwpng_info.row_pointers);
      if (rwpng_info.rgba_data)
	free(rwpng_info.rgba_data);
      if (rwpng_info.indexed_data)
	free(rwpng_info.indexed_data);
      if (rwpng_info.title)
	free(rwpng_info.title);
      if (rwpng_info.author)
	free(rwpng_info.author);
      if (rwpng_info.desc)
	free(rwpng_info.desc);
      if (rwpng_info.email)
	free(rwpng_info.email);
      if (rwpng_info.url)
	free(rwpng_info.url);
}

static int pngnq(char* filename, char* newext, char* newdir, 
		 int sample_factor, int n_colours, int verbose, 
		 int using_stdin, int force)
{
  char *outname = NULL;
  FILE *infile = NULL;
  FILE *outfile = NULL;
  struct stat st;

  int bot_idx, top_idx; /* for remapping of indices */
  int remap[MAXNETSIZE];

  ulg cols, rows;
  ulg row;
  unsigned char map[MAXNETSIZE][4];
  int i,x;

  uch *outrow = NULL;      /* Output image pixels */
  uch **row_pointers=NULL; /* Pointers to rows of pixels */
  int newcolors = n_colours;
  int is_grey;

  if(using_stdin){	
#if defined(MSDOS) || defined(FLEXOS) || defined(OS2) || defined(WIN32)
#if (defined(__HIGHC__) && !defined(FLEXOS))
    setmode(stdin, _BINARY);
#else
    setmode(0, O_BINARY);
#endif
#endif
    infile=stdin;
  }

  /* Open input file. */
  else{
    if(stat(filename,&st) <0){
      fprintf(stderr,"  error: stat error on %s \n",filename);
      fflush(stderr);
      return 14;
    }
    if(S_ISDIR(st.st_mode)){
      fprintf(stderr,"  warning: skipping directory %s \n",filename);
      fflush(stderr);
      return 0;
    }
    if((infile = fopen(filename, "rb"))==NULL){
      fprintf(stderr,"  error: cannot open %s for reading.\n",filename);
      fflush(stderr);
      return 14;
    }
  }
	
  if(using_stdin){ 
#if defined(MSDOS) || defined(FLEXOS) || defined(OS2) || defined(WIN32)
#if (defined(__HIGHC__) && !defined(FLEXOS))
    setmode(stdout, _BINARY);
#else
    setmode(1, O_BINARY);
#endif
#endif
    outfile = stdout;
  }
  else{ 
    outname = create_output_name(filename,newext,newdir);
 
    if (!force) {
      if ((outfile = fopen(outname, "rb")) != NULL) {
	fprintf(stderr, "  error:  %s exists; not overwriting, use -f to force.\n",
		outname);
	fflush(stderr);
	fclose(outfile);
	return 15;
      }
    }
 
    if ((outfile = fopen(outname, "wb")) == NULL) {
      fprintf(stderr, "  error:  cannot open %s for writing\n", outname);
      fflush(stderr);
      return 16;
    }
  }


  /* Read input file */
  rwpng_read_image(infile, &rwpng_info);
  if (!using_stdin)
    fclose(infile);

  if (rwpng_info.retval) {
    fprintf(stderr, "  rwpng_read_image() error:%d %s \n", rwpng_info.retval,filename);
    fflush(stderr);
    if (!using_stdin)
      fclose(outfile);
    return(rwpng_info.retval); 
  }

  cols = rwpng_info.width;
  rows = rwpng_info.height;

  if(!rwpng_info.rgba_data)
    {
      fprintf(stderr,"  no pixel data found.");
    }

  /* Start neuquant */
  initnet((unsigned char*)rwpng_info.rgba_data,rows*cols*4,sample_factor,newcolors);
  learn(verbose);
  unbiasnet();
  getcolormap((unsigned char*)map);
  inxbuild(); 

  /* Remap indexes so all tRNS chunks are together */
  if (verbose) {
    fprintf(stderr,
	    "  remapping colormap to eliminate opaque tRNS-chunk entries...");
    fflush(stderr);
  }
  for (top_idx = newcolors-1, bot_idx = x = 0;  x < newcolors;  ++x) {
    if (map[x][3] == 255) /* maxval */
      remap[x] = top_idx--;
    else
      remap[x] = bot_idx++;
  }
  if (verbose) {
    fprintf(stderr, "%d entr%s left\n", bot_idx,
	    (bot_idx == 1)? "y" : "ies");
    fflush(stderr);
  }

  /* sanity check:  top and bottom indices should have just crossed paths */
  if (bot_idx != top_idx + 1) {
    fprintf(stderr,
	    "  internal logic error: remapped bot_idx = %d, top_idx = %d\n",
	    bot_idx, top_idx);
    fflush(stderr);
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
 
  /* if bot_idx == 0, check whether all RGB samples are gray
     and if so, whether grayscale sample_depth would be same
     => skip following palette section and go grayscale */

  /* TODO check for no alpha at load */
  if(bot_idx == 0){
    x=0;
    is_grey=TRUE;
    while((x<newcolors) & is_grey){
      if (map[x][0] != map[x][1] ||
	  map[x][0] != map[x][1] ||
          map[x][0] != map[x][1]){
	is_grey=FALSE;       
      }
      x++;
     }
     } 
  
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
      fprintf(stderr,
	      "  insufficient memory for indexed data and/or row pointers\n");
      fflush(stderr);
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
  /* Set the modification time to now. */
  rwpng_info.modtime = time(NULL);
  rwpng_info.have_time = TRUE;

  if (rwpng_write_image_init(outfile, &rwpng_info) != 0) {
    fprintf( stderr, "  rwpng_write_image_init() error\n" );
    fflush( stderr );
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
 
  /* Do each image row */
  for ( row = 0; (ulg)row < rows; ++row ) {
    int offset;
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


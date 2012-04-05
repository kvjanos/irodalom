
/************************************************************
 * main.c                                                   *
 *----------------------------------------------------------*
 *  Driver for the 2D shape correspondance algorithm        *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                       *
 ************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "cdf.h"
#include "shape.h"
#include "xfig_object.h"

#define XFIG_DIM_X 5
#define XFIG_DIM_Y 5 

//external variables for parsing commandline options with getopt()
extern char *optarg;
extern int optind;

int main(int argc, char *argv[]) {

// Usage message

if (argc == 1) {
    fprintf(stderr, "Usage: ss2d [options] filename.cdf");
    fprintf(stderr, "\n\toptions supported by ss2d are:\n\n");
    fprintf(stderr, "\t-c  to specify open contour shapes.\n\n");
    fprintf(stderr, "\t-f  to specify the output cdf filename.\n\n");
    fprintf(stderr, "\t-v  to verbosely print out the information of the current\n");
    fprintf(stderr, "\t    calculations on the shapes being worked on.\n\n");
    fprintf(stderr, "\t-x  to specify the output's xfig filename.\n\n");
    exit(0);
}

/************************************\
 * Variables used for the algorithm *
\************************************/   
  //loop controls
  int num_landmarks = 0; //number of landmarks in the template
  int num_shapes = 0; //number of targets
 

  //arrays for holding the transformations removed by Procrustes
  double *translate = NULL, *scale = NULL, *rotate = NULL, *shape_t = NULL;
  //array of corresponded landmarks.  Use for Procrustes
  double **shapes = NULL;

  //statistics vairables
  double **shape_space = NULL; //normalized shape space
  double *shape_space_mean = NULL; //will hold the mean shape
  double *covariance_matrix = NULL; //covariance matrix for the shape space

  //funtion pointer
  static void (* draw_shape_space)(FILE *, double **, double *, int, int);

/***************************\
 * Variables for usability *
\***************************/
  //random integers we use, indexing, error, util...
  int i = 0;
  //for handling options with getopt()
  char c = '\0', *options = ":cf:v::x:";
  //input/output filenames
  char *cdf_in_filename = NULL, *statistics_filename = NULL, *shape_space_filename = NULL;
  //hold cdf variable name
  char **dataset_name = NULL;

  //hold return code of CDF library function
  CDFstatus status = 0;
  //IDs for CDF files
  CDFid cdf_in = NULL, cdf_out = NULL;
  //for holding info about how cdf variables are stored
  long dim_sizes[2], majority_in, majority_out;
  //for indexing CDF zVar's and Attributes
  long output_zvar_num, zvar_attr_num, trans_attr_num, scale_attr_num, rotate_attr_num;
  //for writing variables to the outputcdf
  static long REC_VARY = {VARY};
  static long DIM_VARY[2] = {VARY, VARY};

  draw_shape_space = closed_draw_shape_space;

  //don't print info by default
  VERBOSE = 0;

/******************************************\
 * Get started with setting everything up *
\******************************************/

  while ((c = getopt(argc, argv, options)) != EOF) {
    switch (c) {
      case 'c':
        draw_shape_space = open_draw_shape_space;
      break;
      case 'f':
        statistics_filename = strdup(optarg);
      break;
      case 'x':
        shape_space_filename = strdup(optarg);
      break;
      case 'v':
        VERBOSE = 1;
        if ((optarg != NULL) && (strspn(optarg, "+") == strlen(optarg))) {
          ++VERBOSE;
        }
      break;
      case '?':
        fprintf(stderr, "Unsupported option %c\n", optopt);
      break;
      case ':':
        fprintf(stderr, "%s missing option\n", argv[optind-1]);
      break;
    }
  }

  //load cdf data, and set up template
  cdf_in_filename = argv[argc-1]; //getopt should put all non options at the end of argv regardless of where they occur in command
  //find last '.' in filename
  int dot = strlen(cdf_in_filename)-4;
  cdf_in_filename[dot] = '\0';
  
  //setup filenames for output
  if (statistics_filename == NULL) {
    statistics_filename = (char *) malloc(sizeof(char) * strlen(cdf_in_filename) + 16);
    statistics_filename[0] = '\0';
    strcat(statistics_filename, cdf_in_filename);
    strcat(statistics_filename, "_statistics.cdf");
  }
  if (shape_space_filename == NULL) {
    shape_space_filename = (char *) malloc(sizeof(char) * strlen(cdf_in_filename) + 17);
    shape_space_filename[0] = '\0';
    strcat(shape_space_filename, cdf_in_filename);
    strcat(shape_space_filename, "_shape_space.fig");
  }
  cdf_in_filename[dot] = '.';
 
//////////////VERBOSE/////////////////  
    if (VERBOSE) { 
      fprintf(stdout, "CDF input file:\t\t\t%s\n", cdf_in_filename);
      fprintf(stdout, "output files:\n\t[Procrustes Analysis]\t%s\n\t[Shape Space]\t\t%s\n", statistics_filename, shape_space_filename);
    }
//////////////////////////////////////
    
  //open input/output CDF's
  if ((status = CDFopenCDF(cdf_in_filename, &cdf_in)) != CDF_OK) {
    fprintf(stderr, "Could not open file %s.\n", cdf_in_filename);
    exit(0);
  }
  
  //get some info about the cdf files
  CDFgetMajority(cdf_in, &majority_in);
  //get targets
  CDFgetNumzVars(cdf_in, &num_shapes);

  //allocate strings for each target
  dataset_name = (char **) malloc(sizeof(char *) * num_shapes);

  //allocate space for shapes.  Will be used for procrustes
  shapes = (double **) malloc(sizeof(double *) * num_shapes);

  for (i = 0; i < num_shapes; ++i) {
    CDFgetzVarDimSizes(cdf_in, i, dim_sizes);
    dataset_name[i] = (char *) malloc(sizeof(char) * CDF_VAR_NAME_LEN256);
    CDFgetzVarName(cdf_in, i, dataset_name[i]);
    shapes[i] = load_data_cdf(cdf_in, i, majority_in, dim_sizes[0], dim_sizes[1]);
  }
  num_landmarks = dim_sizes[0];
  
/******************************\
 * Statistical Shape Analysis *
\******************************/

  translate = (double *) malloc(sizeof(double) * num_shapes * 2);
  scale = (double *) malloc(sizeof(double) * num_shapes);
  rotate = (double *) malloc(sizeof(double) * num_shapes);
  //do procrustes fit

//////////////VERBOSE/////////////////
    if (VERBOSE) { fprintf(stdout, "Starting Statistical Shape analysis:\n\tPerforming General Procrustes Fit on shapes\n"); }
//////////////////////////////////////
  
  shape_space = general_procrustes(shapes, num_shapes, num_landmarks, translate, scale, rotate);
  free_matrix((void **)shapes, num_shapes);

  //some statiscal stuff
  //get covariance matrix
  shape_space_mean = (double *) calloc(num_landmarks * 2, sizeof(double));
  covariance_matrix = covariance(shape_space, shape_space_mean, num_landmarks, num_shapes, 0);

  //now output statistical info
  //create cdf for procrustes info
  if ((status = CDFcreateCDF(statistics_filename, &cdf_out)) != CDF_OK) {
    fprintf(stderr, "Could not create %s for output.\n", statistics_filename);
    exit(0);
  }
  free_array((void *) statistics_filename);
  
  //create translate rotate and scale attributes
  if (((status = CDFcreateAttr(cdf_out, "Translation", VARIABLE_SCOPE, &trans_attr_num)) != CDF_OK)) {
    fprintf(stderr, "CDF Error: %d\n", status);
    fprintf(stderr, "Could not create translate attribute for output. The translation not be included in the output.\n");
  }
  if (((status = CDFcreateAttr(cdf_out, "Scaling", VARIABLE_SCOPE, &scale_attr_num)) != CDF_OK)) {
    fprintf(stderr, "CDF Error: %d\n", status);
    fprintf(stderr, "Could not create scale attribute for output. The scaling will not be included in the output.\n");
  }
  if (((status = CDFcreateAttr(cdf_out, "Rotation", VARIABLE_SCOPE, &rotate_attr_num)) != CDF_OK)) {
    fprintf(stderr, "CDF Error: %d\n", status);
    fprintf(stderr, "Could not create rotate attribute for output. The rotation will not be included in the output.\n");
  }
 
  //get some info
  CDFgetMajority(cdf_out, &majority_out);
  for (i = 0; i < num_shapes; ++i) {

//////////////VERBOSE/////////////////
      if (VERBOSE) { 
        fprintf(stdout, "\t  Generating output for Procrustes fit of shape ");
        long tell = ftell(stdout);
        fprintf(stdout, "%s.\r", dataset_name[i]);
        if (i < num_shapes-1) {
          long now = ftell(stdout);
          fseek(stdout, tell-now, SEEK_SET);
        }
        else {
          fprintf(stdout, "\n");
        }
      }
//////////////////////////////////////

    if ((status = CDFcreatezVar(cdf_out, dataset_name[i], CDF_DOUBLE, 1, 2, dim_sizes, REC_VARY, DIM_VARY, &output_zvar_num)) != CDF_OK) {
      fprintf(stderr, "Could not create a variable for %s in the output file. %s will not be included in the output.\n", dataset_name[i], dataset_name[i]);
    }
    else {
      //store procrustes shape fit
      store_data_cdf(cdf_out, shape_space[i], output_zvar_num, ROW_MAJOR, num_landmarks, 2); 

      //store attributes
      if ((status = CDFconfirmAttrExistence(cdf_out, "Translation")) == CDF_OK) {
        CDFputAttrzEntry(cdf_out, trans_attr_num, output_zvar_num, CDF_DOUBLE, 2, translate+(i*2));
      }
      if ((status = CDFconfirmAttrExistence(cdf_out, "Scaling")) == CDF_OK) {
        CDFputAttrzEntry(cdf_out, scale_attr_num, output_zvar_num, CDF_DOUBLE, 1, scale+i);
      }
      if ((status = CDFconfirmAttrExistence(cdf_out, "Rotation")) == CDF_OK) {
        CDFputAttrzEntry(cdf_out, rotate_attr_num, output_zvar_num, CDF_DOUBLE, 1, rotate+i);
      }
    }
  }

  //store mean shape
  if ((status = CDFcreatezVar(cdf_out, "Mean Shape", CDF_DOUBLE, 1, 2, dim_sizes, REC_VARY, DIM_VARY, &output_zvar_num)) != CDF_OK) {
    fprintf(stderr, "Could not create a variable for the Mean Shape in the output file.  The Mean Shape will not be included in the output.\n");
  }
  else {
    store_data_cdf(cdf_out, shape_space_mean, output_zvar_num, majority_out, num_landmarks, 2);
  }

  //store covariance matrix
  dim_sizes[0] = dim_sizes[1] = num_landmarks * 2;
  if ((status = CDFcreatezVar(cdf_out, "Covariance", CDF_DOUBLE, 1, 2, dim_sizes, REC_VARY, DIM_VARY, &output_zvar_num)) != CDF_OK) {
    fprintf(stderr, "Could not create a variable for Covariance in the output file. Covariance will not be included in the output.\n");
  }
  else {
    store_data_cdf(cdf_out, covariance_matrix, output_zvar_num, majority_out, num_landmarks*2, num_landmarks*2);
  }
  CDFclose(cdf_out);

//////////////VERBOSE/////////////////  
    if (VERBOSE) { fprintf(stdout, "\tFinished generating statistical data\n"); }
//////////////////////////////////////

  //now the shape space xfigure
  XFIG *xfig = create_xfig(shape_space_filename, LANDSCAPE, CENTER, INCHES, LETTER, 100.00, MULTIPLE, DEFAULT_COLOR);
  if (xfig == NULL) {
    fprintf(stderr, "Could not open xfigure file %s for writing.  Figures will not be generated.\n", shape_space_filename);
  }
  else {
    draw_shape_space(xfig, shape_space, shape_space_mean, num_shapes, num_landmarks);
    close_xfig(xfig);
  }
  free_array((void *) shape_space_filename);

  fclose(stderr);
 
 /* 
  //free up before leaving
  free_matrix((void **)shape_space, num_shapes);
  free_matrix((void **)dataset_name, num_shapes);
  free_array((void *)translate);
  free_array((void *)scale);
  free_array((void *)rotate);
  free_array((void *)shape_space_mean);
  free_array((void *)covariance_matrix);
  */
  
  return 0;
}


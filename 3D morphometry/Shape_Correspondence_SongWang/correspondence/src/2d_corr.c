
/************************************************************
 * main.c                                                   *
 *----------------------------------------------------------*
 *  Driver for the 2D shape correspondenceI algorithm       *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
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

/************************************\
 * Variables used for the algorithm *
\************************************/   
  //loop controls
  int num_landmarks = 0; //number of landmarks in the template
  int num_targets = 0; //number of targets
  int num_slides = 20; //num of QP slides to perform
  int num_target_landmarks = 0; //num or landmarks in the current target

  double lambda = -1;

  //for keeping things on the true curve
  double *target_offset = NULL;
  //for landmark representation error calculations
  double *init_seg_length = NULL;

  //bending energy variables
  double init_be = 0.0;
  double final_be = 0.0;

  //landmark arrays
  double *template_landmarks = NULL;
  double *target_landmarks = NULL;
  double *corresponded_landmarks = NULL;

  //thin plate matrix for the template
  double *template_thin_plate = NULL;

  //function pointer
  static point * (* locate_point)(double *, int, double);
  static void (* initialize_landmarks)(double *, int, double *, int, double *, double *, double *, double *);
  static void (* slide_a_step)(double *, int, double *, int, double *, double *, double, double *, int *);
  static void (* draw_xfig_shapes)(FILE *, char *, double, double *, int, float, float, int);

/***************************\
 * Variables for usability *
\***************************/
  //random integers we use, indexing, error, util...
  int i = 0, j = 0, k = 0, error = 0, template_index = 0;
  //for handling options with getopt()
  char c = '\0', *options = ":ce::l:o:s:v::x:";
  //input/output filenames
  char *cdf_in_filename = NULL, *cdf_out_filename = NULL, *xfig_filename = NULL;
  //for printing error code from OOQP
  char *error_string[5] = {"SUCCESSFUL_TERMINATION", "NOT_FINISHED", "MAX_ITS_EXCEEDED", "INFEASABLE", "UNKNOWN"};
  //hold cdf variable name
  char *dataset_name = NULL;

  //stream to print errors on
  FILE *ERROR_LOG = NULL;
  //don't log errors by default
  int ERROR = 0;
  
  //hold return code of CDF library function
  CDFstatus status = 0;
  //IDs for CDF files
  CDFid cdf_in = NULL, cdf_out = NULL;
  //for holding info about how cdf variables are stored
  long dim_sizes[2], majority_in, majority_out;
  //for indexing CDF zVar's and Attributes
  long output_zvar_num, zvar_attr_num;
  //for writing variables to the outputcdf
  static long REC_VARY = {VARY};
  static long DIM_VARY[2] = {VARY, VARY};

  //don't print info by default
  VERBOSE = 0;

  //default to closed curves
  locate_point = closed_locate_point;
  initialize_landmarks = closed_initialize_landmarks;
  slide_a_step = closed_slide_a_step;
  draw_xfig_shapes = closed_draw_xfig_shapes;

/******************************************\
 * Get started with setting everything up *
\******************************************/

  if (argc < 2) {
    fprintf(stderr, "Usage: 2dcor [options] filename.cdf");
    fprintf(stderr, "\n\toptions supported by 2dcor are:\n\n");
    fprintf(stderr, "\t-c  to specify open contour shapes.\n\n");
    fprintf(stderr, "\t-o  to specify the output cdf filename.\n\n");
    fprintf(stderr, "\t-v  to verbosely print out the information of the current\n");
    fprintf(stderr, "\t    calculations on the shapes being worked on.\n\n");
    fprintf(stderr, "\t-s  to specify the number of slides.\n\n");
    fprintf(stderr, "\t\tcorrespond -s 10 foo.cdf\n\n");
    fprintf(stderr, "\t-x  to specify the output's xfig filename.\n\n");
    fprintf(stderr, "\t-e  print errors.\n");
    fprintf(stderr, "\t\t-eo print to stdout.\n");
    fprintf(stderr, "\t\t-ee print to stderr.\n");
    fprintf(stderr, "\t\t-e  print to correspondence_error.log.\n\n");
    exit(0);
  }

  while ((c = getopt(argc, argv, options)) != EOF) {
    switch (c) {
      case 'c':
        locate_point = open_locate_point;
        initialize_landmarks = open_initialize_landmarks;
        slide_a_step = open_slide_a_step;
        draw_xfig_shapes = open_draw_xfig_shapes;
      break;
      case 'e':
        ERROR = 1;
        if ((optarg != NULL) && (strspn(optarg, "eo") == strlen(optarg))) {
          ERROR_LOG = ((strchr(optarg, 'o')) ? stdout : NULL);
          ERROR_LOG = ((strchr(optarg, 'e')) ? stderr : ERROR_LOG);
        }
        else if (optarg != NULL) {
          fprintf(stderr, "Invalid optional argument to -e\n\t-ee -> send errors to stderr\n\t-eo -> send errors to stdout\n\t-e -> send errors to correspondence_error.log\n");
        }
        else {
          if ((ERROR_LOG = fopen("correspondence_error.log", "w")) == NULL) {
            fprintf(stderr, "Could not open correspondence_error.log for logging.  No errors will be logged.\n");
          }
        }
      break;
      case 'l':
        lambda = strtod(optarg, NULL);
      break;
      case 'o':
        cdf_out_filename = strdup(optarg);
      break;
      case 's':
        num_slides = strtol(optarg, NULL, 10);
      break;
      case 'v':
        VERBOSE = 1;
        if ((optarg != NULL) && (strspn(optarg, "+") == strlen(optarg))) {
          ++VERBOSE;
        }
      break;
      case 'x':
        xfig_filename = strdup(optarg);
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
  if (xfig_filename == NULL) {
    xfig_filename = (char *) malloc(sizeof(char) * strlen(cdf_in_filename) + 5);
    xfig_filename[0] = '\0';
    strcat(xfig_filename, cdf_in_filename);
    strcat(xfig_filename, ".fig");
  }
  if (cdf_out_filename == NULL) {
    cdf_out_filename = (char *) malloc(sizeof(char) * strlen(cdf_in_filename) + 18);
    cdf_out_filename[0] = '\0';
    strcat(cdf_out_filename, cdf_in_filename);
    strcat(cdf_out_filename, "_corresponded.cdf");
  }
  cdf_in_filename[dot] = '.';
 
//////////////VERBOSE/////////////////  
    if (VERBOSE) { 
      if (ERROR) {
        fprintf(stdout, "\nErrors logged with %s\n", ((ERROR_LOG == stdout) ? "stdout" : ((ERROR_LOG == stderr) ? "stderr" : "correspondence_error.log")));
      }
      else {
        fprintf(stdout, "Errors not logged\n");
      }
      fprintf(stdout, "CDF input file:\t\t\t%s\n", cdf_in_filename);
      fprintf(stdout, "Output files:\n\t[Corresponded Landmarks]\t%s\n\t[Corresponded Shapes] \t\t%s\n", cdf_out_filename, xfig_filename);
    }
//////////////////////////////////////
    
  //open input/output CDF's
  if ((status = CDFopenCDF(cdf_in_filename, &cdf_in)) != CDF_OK) {
    fprintf(stderr, "Could not open file %s.\n", cdf_in_filename);
    exit(0);
  }
  if ((status = CDFcreateCDF(cdf_out_filename, &cdf_out)) != CDF_OK) {
    fprintf(stderr, "Could not create %s for output.\n", cdf_out_filename);
    exit(0);
  }
  free_array((void *) cdf_out_filename);
  
  //set up variable attribute in cdf outputfile
  if (((status = CDFcreateAttr(cdf_out, "Bending Energy", VARIABLE_SCOPE, &zvar_attr_num)) != CDF_OK) && (ERROR)) {
    fprintf(ERROR_LOG, "CDF error: %d\n", status);
    fprintf(ERROR_LOG, "Could not create bending energy attribute for output. The bending energy will not be included in the output.\n");
  }

  //get some info about the cdf files
  CDFgetMajority(cdf_in, &majority_in);
  CDFgetMajority(cdf_out, &majority_out);

  //get template
  if ((template_index = CDFgetVarNum(cdf_in, "template")) < 0) {
    fprintf(stderr, "No Template specified in CDF file.\n A zVariable must exitst with the name 'template'\n");
    exit(0);
  }
  CDFgetzVarDimSizes(cdf_in, template_index, dim_sizes); //get dimensions
  template_landmarks = load_data_cdf(cdf_in, template_index, majority_in, dim_sizes[0], dim_sizes[1]); //load template
  num_landmarks = dim_sizes[0]; //get number of landmarks
  if (lambda == -1.0) {
    lambda = 1e-5 / num_landmarks;
  }

  //get targets
  CDFgetNumzVars(cdf_in, &num_targets);
  --num_targets;//account for the template zVar

  //allocate strings for each target
  dataset_name = (char *) malloc(sizeof(char) * CDF_VAR_NAME_LEN256);

//////////////VERBOSE/////////////////    
    if (VERBOSE) { fprintf(stdout, "\nTemplate landmarks: %d\nCorresponding %d shapes in %d slides.\n\n", num_landmarks, num_targets, num_slides); }
//////////////////////////////////////
  
  //get thin_plate_spline matrix
  template_thin_plate = thin_plate_spline(template_landmarks, num_landmarks);
  
  //allocate space for shapes.  Will be used for procrustes
  corresponded_landmarks = (double *) malloc(sizeof(double) * num_landmarks * 2);
  target_offset = (double *) malloc(sizeof(double) * num_landmarks);
  init_seg_length = (double *) malloc(sizeof(double) * num_landmarks);
  
  //create new xfig file for the corresponded shapes
  XFIG *xfig = create_xfig(xfig_filename, LANDSCAPE, CENTER, INCHES, LETTER, 100.00, MULTIPLE, DEFAULT_COLOR);
  if (xfig == NULL) {
    if (ERROR) { fprintf(ERROR_LOG, "XFIG error: Could not open xfigure file %s for writing.  Figures will not be generated.\n", xfig_filename); }
  }
  free_array((void *) xfig_filename);

/********************************\
 * The Correspondance algorithm *
\********************************/

  for (i = 0, j = 0; i < num_targets; ++i, ++j) {
    //dont want to slide the template
    if (i == template_index) {
      ++j;
    }

    //get dimension info
    CDFgetzVarDimSizes(cdf_in, j, dim_sizes);

    //get variable name
    CDFgetzVarName(cdf_in, j, dataset_name);
    num_target_landmarks = dim_sizes[0];
    
    //load jth shape instance
    target_landmarks = load_data_cdf(cdf_in, j, majority_in, dim_sizes[0], dim_sizes[1]);

//////////////VERBOSE/////////////////    
      if (VERBOSE) { 
        fprintf(stdout, "Working on data set %d[%s] with %d landmarks\n", i+1, dataset_name, num_target_landmarks); 
        fprintf(stdout, "\tInitializing target landmarks\n"); 
      }
//////////////////////////////////////
    
    //initialize target landmarks
    initialize_landmarks(template_landmarks, num_landmarks, target_landmarks, num_target_landmarks, template_thin_plate, &init_be, target_offset, init_seg_length);

//////////////VERBOSE/////////////////    
      if (VERBOSE) {
        fprintf(stdout, "\t  Initialized with a %.4lf bending energy\n", init_be);
        fprintf(stdout, "\tSliding landmarks:\n");
      }
//////////////////////////////////////

    for (k = 0; k < num_slides; ++k) {
      //do kth slide 
      slide_a_step(target_landmarks, num_target_landmarks, target_offset, num_landmarks, template_thin_plate, init_seg_length, lambda, &final_be, &error);

      if (final_be < 0.0) {
        break;
      }
      //make sure OOQP was able to solve
      if (error != 0) {

        //if error is not UNKNOWN
        if (error != 4) {
          
//////////////VERBOSE/////////////////      
            if (VERBOSE) { fprintf(stdout, "Could not solve.  OOQP exited with code [%s] on slide %d.\n\n", error_string[error], k+1); }
//////////////////////////////////////

          if (ERROR) {
            fprintf(ERROR_LOG, "OOQP Error: %s\n", dataset_name[i]);
            fprintf(ERROR_LOG, "Could not solve.  OOQP exited with code [%s] on slide %d.\nOutput data for this shape may or maynot be accurate.\n", error_string[error], k+1);
          }
          break;
        }
        //for UNKNOWN error code
        else {

//////////////VERBOSE/////////////////      
            if (VERBOSE) { fprintf(stdout, "\tOOQP was progressing too slowly. Did not complete full iteration of sliding.\n\tResults should still be usable.\n"); }
//////////////////////////////////////

          if (ERROR) {
            fprintf(ERROR_LOG, "OOQP reported 'unacceptably slow progress'. Assuming convergence.\n");
          }
          break;
        }
          
      }

        if (VERBOSE > 1) { fprintf(stdout, "\t  Slide %d finished with %.4lf bending energy.\n", k, final_be); }
      //fprintf(stderr, "lnd: %d|%d[%p|%p]\n", num_landmarks, num_target_landmarks, &num_landmarks, &num_target_landmarks);
    }
    
//////////////VERBOSE/////////////////   
      if (VERBOSE) {
        fprintf(stdout, "\t  Finished %d slides with a %.4lf bending energy.\n", k, final_be);
        fprintf(stdout, "\tGenerating xFigure\n\n");
      }
//////////////////////////////////////

    //allocate and set up shapes for Procrustes later
    for (k = 0; k < num_landmarks; ++k) {
      point *corresponded_point = locate_point(target_landmarks, num_target_landmarks, target_offset[k]);
      corresponded_landmarks[k*2] = corresponded_point->x_coord;
      corresponded_landmarks[(k*2)+1] = corresponded_point->y_coord;
      free_array((void *) corresponded_point);
    }

    //do our output stuff
    if (xfig != NULL) {
      draw_xfig_shapes(xfig, dataset_name, final_be, corresponded_landmarks, num_landmarks, XFIG_DIM_X, XFIG_DIM_Y, i);
    }
    //create new zVar if it can't be created, then it will be skipped
    dim_sizes[0] = num_landmarks;
    if ((status = CDFcreatezVar(cdf_out, dataset_name, CDF_DOUBLE, 1, 2, dim_sizes, REC_VARY, DIM_VARY, &output_zvar_num)) != CDF_OK) {
      if (ERROR) { fprintf(ERROR_LOG, "Could not create a variable for %s in the output file. %s will not be included in the output.\n", dataset_name[i], dataset_name[i]); }
    }
    else {
      //write corresponded shape to output *.cdf
      store_data_cdf(cdf_out, corresponded_landmarks, output_zvar_num, majority_out, num_landmarks, 2);
      if ((status = CDFconfirmAttrExistence(cdf_out, "Bending Energy")) == CDF_OK) {
        CDFputAttrzEntry(cdf_out, zvar_attr_num, output_zvar_num, CDF_DOUBLE, 1, &final_be);
      }
    }
    
    //free up before next round
    free_array((void *) target_landmarks);

    //reset bending energies
    init_be = 0.0;
    final_be = 0.0;
    error = 0;

//////////////VERBOSE/////////////////
      if (!VERBOSE) { fprintf(stdout, "%.2lf%%\r", ((double) ((i+1)*100))/num_targets); fseek(stdout, -5L, SEEK_CUR); }
//////////////////////////////////////
  }

  close_xfig(xfig);
  CDFclose(cdf_in);
  CDFclose(cdf_out);
  if (ERROR) {
    fclose(ERROR_LOG);
  }

  return 0;
}


/************************************************************
 * shape.h                                                  *
 *----------------------------------------------------------*
 *  Header file for Shape Correspondence algorithm          *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/


#ifndef SHAPE_H
#define SHAPE_H
int VERBOSE; //for printing stuff

//struct for the curve information
typedef struct curve_data {
	double curve_length;//total curve length
	double *seg_length;//array of segment lengths
	double *accum_length;//array of total length upto and including the segment
} curve;

//struct for the point data
typedef struct point_data {
  double x_coord, y_coord;
  double x_tangent, y_tangent;
  double x_normal, y_normal;
  double curvature;
} point;

//curve_length.c
curve *closed_curve_length(double *, int); //for closed-curve shape correspondence
curve *open_curve_length(double *, int);

//locate_point.c
point *closed_locate_point(double *, int, double); //for closed-curve shape correspondence
point *open_locate_point(double *, int, double);

//thin_plate_spline.c
double *thin_plate_spline(double *, int);

//slide_a_step_closed.c
void closed_slide_a_step(double *, int, double *, int, double *, double *, double, double *, int *);
void open_slide_a_step(double *, int, double *, int, double *, double *, double, double *, int *);

//initialize_landmarks.c
void closed_initialize_landmarks(double *, int, double *, int, double *, double *, double *, double *); //for closed-curve shape correspondence
void open_initialize_landmarks(double *, int, double *, int, double *, double *, double *, double *);

//xfig_utils.c
void find_normalizing_factors(double *, int, double *, double *, double *);
FILE *create_xfig(char *, char *, char *, char *, char *, float, char *, int);
void close_xfig(FILE *);
void closed_draw_xfig_shapes(FILE *, char *, double, double *, int, float, float, int);
void closed_draw_shape_space(FILE *, double **, double *, int, int);
void open_draw_xfig_shapes(FILE *, char *, double, double *, int, float, float, int);
void open_draw_shape_space(FILE *, double **, double *, int, int);

//shape_utils.c
double *invert(double *, int);
inline double sum_l2norm(double *, int);
inline void free_matrix(void **, int);
inline void free_array(void *);
inline void kill_curve(curve *);
void store_data_cdf(void *, double *, long, long, int, int);
double *load_data_cdf(void *, long, long, int, int);
inline void init_tangent(double *, float);
void gauss_elim(double *, int *, int);
void mat_mult(double *, int, double *, int, int, double *);
double *transpose(double *, int, int);
double **general_procrustes(double **, int, int, double *, double *, double *);
int svd(double *, int, int, double *, double *);
double pythag(double, double);
double *covariance(double **, double *, int, int, int);

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define SIGN(x, y) (((y) < 0) ? -fabs(x) : fabs(x))

#endif

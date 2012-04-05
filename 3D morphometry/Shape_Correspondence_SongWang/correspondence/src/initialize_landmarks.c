/************************************************************
 * initialize_landmarks.c                                   *
 *----------------------------------------------------------*
 *  Functions for initial rough correspondence              *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"

#define  NUM_INIT_V0_TRIES 40

/***************************************************************************************
 * initialize_landmarks_closed(template, number1, target, number2, thin plate spline)  *
 * --                                                                                  *
 *  Creates initial landmarks from the landmarks in target such that:                  *
 *   1)  number of landmarks = number1                                                 *
 *   2)  is the set of landmarks with lowest bending energy                            *
 *   3)  it is meaningful to compare template->landmark[1] to init_target->landmark[1] *
 *   4) A strict equality is held that the ratio of distances between landmarks and    *
 *        the total curve length is the same as it is for template                     * 
 ***************************************************************************************/
void closed_initialize_landmarks(double *template, int num_landmarks, double *target, int num_target_landmarks, double *template_thin_plate, double *best_be, double *init_target_offset, double *init_seg_length) {
  int i, j;

  double this_be, length_ratio;
  double distance_to_v0;
  
  double *be_temp = (double *) malloc(sizeof(double) * 1 * num_landmarks);
  double *be_temp2 = (double *) malloc(sizeof(double));
  double *this_fit_x = (double *) malloc(sizeof(double) * num_landmarks);
  double *this_fit_y = (double *) malloc(sizeof(double) * num_landmarks);
  double *this_fit_offset = (double *) malloc(sizeof(double) * num_landmarks);
  
  point *temp_point;

  curve *template_curve = closed_curve_length(template, num_landmarks);
  curve *target_curve = closed_curve_length(target, num_target_landmarks);
  length_ratio = target_curve->curve_length / template_curve->curve_length;
 
  //malloc individual landmarks in best_fit and
  //initialize best_fit with v0 of target as v0 of best fit
  for (i = 0; i < num_landmarks; ++i) {
    //keep strict equality in (6) or ...
    //distribute initial landmarks along target
    //with same length distribution as template
   
    init_target_offset[i] = ((i == 0) ? 0.0 : (template_curve->accum_length[i-1]*length_ratio));
    temp_point = closed_locate_point(target, num_target_landmarks, init_target_offset[i]);
    this_fit_x[i] = temp_point->x_coord;
    this_fit_y[i] = temp_point->y_coord;

    //temp_points gets allocated with each call to locate_point so we should free it
    free_array((void *) temp_point);
  }

  //calculate initial bending energy of best_fit
  //be = x'Lx + y'Ly
  mat_mult(this_fit_x, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, this_fit_x, num_landmarks, 1, be_temp2);
  *best_be = *be_temp2;
  mat_mult(this_fit_y, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, this_fit_y, num_landmarks, 1, be_temp2);
  *best_be += *be_temp2;

  //split target up into num_target_landmarks points and pick each one as v0 calculate
  //bending energy and compare
  for (i = 1; i < num_target_landmarks; ++i) {
    distance_to_v0 = target_curve->accum_length[i-1];

    for (j = 0; j < num_landmarks; ++j) {
      //still keep equality
      this_fit_offset[j] =  distance_to_v0 + ((j == 0) ? 0.0 : (template_curve->accum_length[j-1]*length_ratio));
      temp_point = closed_locate_point(target, num_target_landmarks, this_fit_offset[j]);
      this_fit_x[j] = temp_point->x_coord;
      this_fit_y[j] = temp_point->y_coord;
      free_array((void *) temp_point);
    }
  
    //be = x'Lx + y'Ly
    mat_mult(this_fit_x, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
    mat_mult(be_temp, 1, this_fit_x, num_landmarks, 1, be_temp2);
    this_be = *be_temp2;
    mat_mult(this_fit_y, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
    mat_mult(be_temp, 1, this_fit_y, num_landmarks, 1, be_temp2);
    this_be += *be_temp2;
  
    if ((*best_be - this_be) > 0.0) {
      *best_be = this_be;
  
      for (j = 0; j < num_landmarks; ++j) {
        init_target_offset[j] = this_fit_offset[j];
      }
    }
  
  }

  for (i = 0; i < num_landmarks; ++i) {
    init_seg_length[i] = fmod(target_curve->curve_length + init_target_offset[(i+1)%num_landmarks] - init_target_offset[i], target_curve->curve_length);
  }

  free_array((void *) be_temp2);
  free_array((void *) be_temp);
  free_array((void *) this_fit_x);
  free_array((void *) this_fit_y);
  free_array((void *) this_fit_offset);
  kill_curve(template_curve);
  kill_curve(target_curve);
  //kill_curve(best_curve);
  
}

void open_initialize_landmarks(double *template, int num_landmarks, double *target, int num_target_landmarks, double *template_thin_plate, double *best_be, double *init_target_offset, double *init_seg_length) {
  int i, j;
  double length_ratio;

  double *be_temp = (double *) malloc(sizeof(double) * 1 * num_landmarks);
  double *be_temp2 = (double *) malloc(sizeof(double));
  double *this_fit_x = (double *) malloc(sizeof(double) * num_landmarks);
  double *this_fit_y = (double *) malloc(sizeof(double) * num_landmarks);
  
  point *temp_point;

  curve *template_curve = open_curve_length(template, num_landmarks);
  curve *target_curve = open_curve_length(target, num_target_landmarks);
  length_ratio = target_curve->curve_length / template_curve->curve_length;
 
  //malloc individual landmarks in best_fit and
  //initialize best_fit with v0 of target as v0 of best fit
  for (i = 1; i < num_landmarks-1; ++i) {
    //keep strict equality in (6) or ...
    //distribute initial landmarks along target
    //with same length distribution as template

    init_target_offset[i] = ((i == 0) ? 0.0 : (template_curve->accum_length[i-1]*length_ratio));
    temp_point = open_locate_point(target, num_target_landmarks, init_target_offset[i]);
    this_fit_x[i] = temp_point->x_coord;
    this_fit_y[i] = temp_point->y_coord;

    //temp_points gets allocated with each call to locate_point so we should free it
    free_array((void *) temp_point);
  }
  this_fit_x[0] = target[0];
  this_fit_x[num_landmarks-1] = target[2*num_target_landmarks-2];
  this_fit_y[0] = target[1];
  this_fit_y[num_landmarks-1] = target[2*num_target_landmarks-1];
  init_target_offset[0] = 0.0;
  init_target_offset[num_landmarks-1] = target_curve->curve_length;

  //calculate initial bending energy of best_fit
  //be = x'Lx + y'Ly
  mat_mult(this_fit_x, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, this_fit_x, num_landmarks, 1, be_temp2);
  *best_be = *be_temp2;
  mat_mult(this_fit_y, 1, template_thin_plate, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, this_fit_y, num_landmarks, 1, be_temp2);
  *best_be += *be_temp2;

  for (i = 0; i < num_landmarks; ++i) {
    init_seg_length[i] = fmod(target_curve->curve_length + init_target_offset[(i+1)%num_landmarks] - init_target_offset[i], target_curve->curve_length);
  }

  free_array((void *) be_temp2);
  free_array((void *) be_temp);
  free_array((void *) this_fit_x);
  free_array((void *) this_fit_y);
  kill_curve(template_curve);
  kill_curve(target_curve);
  //kill_curve(best_curve);
  
}

/************************************************************
 * locate_point.c                                           *
 *----------------------------------------------------------*
 *  Functions to locate a point along a curve using Catmull-*
 *   Rom splines.                                           *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "shape.h"

/************************************************************
 *  locate_point_closed(landmarks, num_landmarks, distance) *
 *  --                                                      *
 *    Find the point stats for the point located a distance *
 *    of distance from landmark[0], that lies on the curve  *
 *    defined by landmarks.                                 *
 ************************************************************/
point *closed_locate_point(double * landmarks, int num_landmarks, double distance_to_point) {
  int i;
  int l = 0;
  double S = 0.5;
  long double u;
  
  double crs_basis[] = {-1.0, 3.0, -3.0, 1.0, 2.0, -5.0, 4.0, -1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0};  
  double *accum_seg_length = (double *) malloc(sizeof(double) * 100);
  double *cubic = (double *) malloc(sizeof(double) * 4);
  double *coords = (double *) malloc(sizeof(double) * 2);
  double *tangent = (double *) malloc(sizeof(double) * 101 * 2);
  double *cubic_derivative = (double *) malloc(sizeof(double) * 101 * 4);
  double *temp = (double *) malloc(sizeof(double) * 101* 4);
  double *crs_control = (double *) malloc(sizeof(double) * 4 * 2);
  
  point *point = (struct point_data *) malloc(sizeof(struct point_data));
  curve *curve = closed_curve_length(landmarks, num_landmarks);

  //handle modulus with negative numbers
  double mod = 0.0;
  if (distance_to_point < 0.0) {
    mod = floor(fabs(distance_to_point/curve->curve_length)) + 1.0;
    mod = mod * curve->curve_length;
  }

  distance_to_point = fmod(distance_to_point + mod, curve->curve_length);
  
  //find the landmark of the segment on which the point lies
  while ( l < num_landmarks-1) {
    if ((curve->accum_length[l] - distance_to_point) > 0.000001) {
      break;
    }
    ++l;
  }

  //adjust distance to account for what already been found
  if (l > 0) {
    distance_to_point = distance_to_point - curve->accum_length[l-1];
  }

  //build tangents for each segment of the larger segment
  init_tangent(cubic_derivative, S);
  //get control points

  crs_control[0] = landmarks[((l-1+num_landmarks)%num_landmarks)*2];
  crs_control[1] = landmarks[(((l-1+num_landmarks)%num_landmarks)*2)+1];
  crs_control[2] = landmarks[l*2];
  crs_control[3] = landmarks[(l*2)+1];
  crs_control[4] = landmarks[((l+1)%num_landmarks)*2];
  crs_control[5] = landmarks[(((l+1)%num_landmarks)*2)+1];
  crs_control[6] = landmarks[((l+2)%num_landmarks)*2];
  crs_control[7] = landmarks[(((l+2)%num_landmarks)*2)+1];

  mat_mult(cubic_derivative, 101, crs_basis, 4, 4, temp);
  //get fianl tangent
  mat_mult(temp, 101, crs_control, 4, 2, tangent);
  
  for (i = 0; i < 100; ++i) {
    accum_seg_length[i] = ((i == 0) ? (0.0) : (accum_seg_length[i-1])) + (hypot(tangent[i*2], tangent[(i*2)+1]) * 0.01);
    if ((accum_seg_length[i] - distance_to_point) > 0.000001) {
      break;
    }
  }

  u = (double)i *.01;

  free_array((void *)temp);
  temp = (double *) malloc(sizeof(double) * 1 * 4);
  
  cubic[0] = S*u*u*u;
  cubic[1] = S*u*u;
  cubic[2] = S*u;
  cubic[3] = S;

  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_coord = coords[0];
  point->y_coord = coords[1];
  
  cubic[0] = S*3*u*u;
  cubic[1] = S*2*u;
  cubic[2] = S;
  cubic[3] = 0.0;

  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_tangent = coords[0];
  point->y_tangent = coords[1];

  cubic[0] = S*6*u;
  cubic[1] = S*2;
  cubic[2] = 0.0;
  cubic[3] = 0.0;
  
  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_normal = coords[0];
  point->y_normal = coords[1];

  point->curvature = hypot(point->x_tangent, point->y_tangent);
  point->curvature = point->curvature * point->curvature * point->curvature;
  point->curvature = ((point->x_tangent * point->y_normal) - (point->y_tangent * point->x_normal)) / point->curvature;

  free_array((void *)accum_seg_length);
  free_array((void *)cubic);
  free_array((void *)coords);
  free_array((void *)tangent);
  free_array((void *)temp);
  free_array((void *)cubic_derivative);
  free_array((void *)crs_control);
  kill_curve(curve);

  return point;
}

point *open_locate_point(double *landmarks, int num_landmarks, double distance_to_point) {
  int i;
  int l = 0;
  double S = 0.5;
  long double u;
  
  double crs_basis[] = {-1.0, 3.0, -3.0, 1.0, 2.0, -5.0, 4.0, -1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0};  
  double *accum_seg_length = (double *) malloc(sizeof(double) * 100);
  double *cubic = (double *) malloc(sizeof(double) * 4);
  double *coords = (double *) malloc(sizeof(double) * 2);
  double *tangent = (double *) malloc(sizeof(double) * 101 * 2);
  double *cubic_derivative = (double *) malloc(sizeof(double) * 101 * 4);
  double *temp = (double *) malloc(sizeof(double) * 101* 4);
  double *crs_control = (double *) malloc(sizeof(double) * 4 * 2);
  
  point *point = (struct point_data *) malloc(sizeof(struct point_data));
  curve *curve = open_curve_length(landmarks, num_landmarks);

  //handle modulus with negative numbers
  //double mod = 0.0;
  //if (distance_to_point < 0.0) {
    //mod = floor(fabs(distance_to_point/curve->curve_length)) + 1.0;
    //mod = mod * curve->curve_length;
  //}

  //distance_to_point = fmod(distance_to_point + mod, curve->curve_length);
  
  //find the landmark of the segment on which the point lies
  while ( l < (num_landmarks-2)) {
    if ((curve->accum_length[l] - distance_to_point) > 0.000001) {
      break;
    }
    ++l;
  }

  //adjust distance to account for what already been found
  if (l > 0) {
    distance_to_point = distance_to_point - curve->accum_length[l-1];
  }

  //build tangents for each segment of the larger segment
  init_tangent(cubic_derivative, S);
  //get control points

  if (l == 0) {
    crs_control[0] = (2 * landmarks[0]) - landmarks[2];
    crs_control[1] = (2 * landmarks[1]) - landmarks[3];
  }
  else {
    crs_control[0] = landmarks[(l-1)*2];
    crs_control[1] = landmarks[((l-1)*2)+1];
  }
  crs_control[2] = landmarks[l*2];
  crs_control[3] = landmarks[(l*2)+1];
  crs_control[4] = landmarks[(l+1)*2];
  crs_control[5] = landmarks[((l+1)*2)+1];
  if (l == (num_landmarks-2)) {
    crs_control[6] = (2 * crs_control[4]) - crs_control[2];
    crs_control[7] = (2 * crs_control[5]) - crs_control[3];
  }
  else {
    crs_control[6] = landmarks[(l+2)*2];
    crs_control[7] = landmarks[((l+2)*2)+1];
  }

  mat_mult(cubic_derivative, 101, crs_basis, 4, 4, temp);
  //get fianl tangent
  mat_mult(temp, 101, crs_control, 4, 2, tangent);
  
  for (i = 0; i < 100; ++i) {
    accum_seg_length[i] = ((i == 0) ? (0.0) : (accum_seg_length[i-1])) + (hypot(tangent[i*2], tangent[(i*2)+1]) * 0.01);
    if ((accum_seg_length[i] - distance_to_point) > 0.000001) {
      break;
    }
  }

  u = (double)i *.01;

  free_array((void *)temp);
  temp = (double *) malloc(sizeof(double) * 1 * 4);
  
  cubic[0] = S*u*u*u;
  cubic[1] = S*u*u;
  cubic[2] = S*u;
  cubic[3] = S;

  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_coord = coords[0];
  point->y_coord = coords[1];
  
  cubic[0] = S*3*u*u;
  cubic[1] = S*2*u;
  cubic[2] = S;
  cubic[3] = 0.0;

  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_tangent = coords[0];
  point->y_tangent = coords[1];

  cubic[0] = S*6*u;
  cubic[1] = S*2;
  cubic[2] = 0.0;
  cubic[3] = 0.0;
  
  mat_mult(cubic, 1, crs_basis, 4, 4, temp);
  mat_mult(temp, 1, crs_control, 4, 2, coords);
  point->x_normal = coords[0];
  point->y_normal = coords[1];

  point->curvature = hypot(point->x_tangent, point->y_tangent);
  point->curvature = point->curvature * point->curvature * point->curvature;
  point->curvature = ((point->x_tangent * point->y_normal) - (point->y_tangent * point->x_normal)) / point->curvature;

  free_array((void *)accum_seg_length);
  free_array((void *)cubic);
  free_array((void *)coords);
  free_array((void *)tangent);
  free_array((void *)temp);
  free_array((void *)cubic_derivative);
  free_array((void *)crs_control);
  kill_curve(curve);

  return point;
}

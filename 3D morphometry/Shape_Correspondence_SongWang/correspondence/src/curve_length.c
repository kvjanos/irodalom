/************************************************************
 * curve_length.c                                           *
 *----------------------------------------------------------*
 *  Functions to calculate curve attributes                 *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"
	

/*************************************************
 * curve_length_closed(landmarks, num_landmarks) *
 * --                                            *
 *  Find the total curve length defined          *
 *  by num_landmarks landmarks in landmarks      *
 *************************************************/
curve *closed_curve_length(double *landmarks, int num_landmarks) {
  int i;
  double accum;
  double S = 0.5;//default spline tension
  double crs_basis[] = {-1.0, 3.0, -3.0, 1.0, 2.0, -5.0, 4.0, -1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0};  
  double *cubic_prime = (double *) malloc(sizeof(double) * 101 * 4);//matrix of polynomials 3x^2 + 2x + 1 + 0, 0<=x<=1
  double *temp = (double *) malloc(sizeof(double) * 101 * 4);
  double *tangent = (double *) malloc(sizeof(double) * 101 * 2);
  double *crs_control = (double *) malloc(sizeof(double) * 4 * 2);//control landmarks matrix
  curve *ret_data = (curve*) malloc(sizeof(curve));//return curve information pointer
  ret_data->seg_length = (double *) malloc(sizeof(double)*num_landmarks);//alloc space for segment length array
  ret_data->accum_length = (double *) malloc(sizeof(double)*num_landmarks);//alloc space for accumulated length array
  
  //initialize cubic derivative matrix
  init_tangent(cubic_prime, S);
  //premult cubic_prime by basis
  mat_mult(cubic_prime, 101, crs_basis, 4, 4, temp);
  
  //parse landmarks, generate splines, and gather curve length data
  for (i = 0; i < num_landmarks; ++i) {
    //get four control landmarks, draw between 2nd and 3rd landmarks
    crs_control[0] = landmarks[((i-1+num_landmarks)%num_landmarks)*2];
    crs_control[1] = landmarks[(((i-1+num_landmarks)%num_landmarks)*2)+1];
    crs_control[2] = landmarks[i*2];
    crs_control[3] = landmarks[(i*2)+1];
    crs_control[4] = landmarks[((i+1)%num_landmarks)*2];
    crs_control[5] = landmarks[(((i+1)%num_landmarks)*2)+1];
    crs_control[6] = landmarks[((i+2)%num_landmarks)*2];
    crs_control[7] = landmarks[(((i+2)%num_landmarks)*2)+1];
    //calculate T
    mat_mult(temp, 101, crs_control, 4, 2, tangent);
    //calculate curve information
    //segment length is sum of l2norms of intermediary landmarks
    ret_data->seg_length[i] = sum_l2norm(tangent, 100)*0.01;
    //accumulated length = previous accum_length + length of current segment
    ret_data->accum_length[i] = ((i == 0) ? (0.0) : (ret_data->accum_length[i-1])) + ret_data->seg_length[i];
  }
  ret_data->curve_length = ret_data->accum_length[i-1];
  free_array((void *)temp);
  free_array((void *)cubic_prime);
  free_array((void *)tangent);
  free_array((void *)crs_control);

  return ret_data;
}

curve *open_curve_length(double *landmarks, int num_landmarks) {
  int i;
  double accum;
  double S = 0.5;//default spline tension
  double crs_basis[] = {-1.0, 3.0, -3.0, 1.0, 2.0, -5.0, 4.0, -1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0};  
  double *cubic_prime = (double *) malloc(sizeof(double) * 101 * 4);//matrix of polynomials 3x^2 + 2x + 1 + 0, 0<=x<=1
  double *temp = (double *) malloc(sizeof(double) * 101 * 4);
  double *tangent = (double *) malloc(sizeof(double) * 101 * 2);
  double *crs_control = (double *) malloc(sizeof(double) * 4 * 2);//control landmarks matrix
  curve *ret_data = (curve*) malloc(sizeof(curve));//return curve information pointer
  ret_data->seg_length = (double *) malloc(sizeof(double) * (num_landmarks-1));//alloc space for segment length array
  ret_data->accum_length = (double *) malloc(sizeof(double) * (num_landmarks-1));//alloc space for accumulated length array
  
  //initialize cubic derivative matrix
  init_tangent(cubic_prime, S);
  //premult cubic_prime by basis
  mat_mult(cubic_prime, 101, crs_basis, 4, 4, temp);
  
  //parse landmarks, generate splines, and gather curve length data
  for (i = 0; i < (num_landmarks-1); ++i) {
    //get four control landmarks, draw between 2nd and 3rd landmarks
    if (i == 0) {
      crs_control[0] = (2 * landmarks[0]) - landmarks[2];
      crs_control[1] = (2 * landmarks[1]) - landmarks[3];
    }
    else {
      crs_control[0] = landmarks[(i-1)*2];
      crs_control[1] = landmarks[((i-1)*2)+1];
    }
    crs_control[2] = landmarks[i*2];
    crs_control[3] = landmarks[(i*2)+1];
    crs_control[4] = landmarks[(i+1)*2];
    crs_control[5] = landmarks[((i+1)*2)+1];
    if (i == (num_landmarks - 2)) {
      crs_control[6] = (2 * crs_control[4]) - crs_control[2];
      crs_control[7] = (2 * crs_control[5]) - crs_control[3];
    }
    else {
      crs_control[6] = landmarks[(i+2)*2];
      crs_control[7] = landmarks[((i+2)*2)+1];
    }
    //calculate T
    mat_mult(temp, 101, crs_control, 4, 2, tangent);
    //calculate curve information
    //segment length is sum of l2norms of intermediary landmarks
    ret_data->seg_length[i] = sum_l2norm(tangent, 100)*0.01;
    //accumulated length = previous accum_length + length of current segment
    ret_data->accum_length[i] = ((i == 0) ? (0.0) : (ret_data->accum_length[i-1])) + ret_data->seg_length[i];
  }
  ret_data->curve_length = ret_data->accum_length[i-1];
  free_array((void *)temp);
  free_array((void *)cubic_prime);
  free_array((void *)tangent);
  free_array((void *)crs_control);

  return ret_data;
}

/************************************************************
 * thin_plate_matrix.c                                      *
 *----------------------------------------------------------*
 *  Calculate this thin plate spline matrix for a set of    *
 *   landmark landmarks                                     *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: September 06, 2006                       *
 ************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"

/****************************************/
double *thin_plate_spline(double *landmarks, int num_landmarks) {
  int i, j, pid;
  double r_squared;
  double *L = (double *) calloc((num_landmarks+3) * (num_landmarks+3), sizeof(double));//alloc_dmatrix(num_landmarks+3, num_landmarks+3);
  double *L_inverse;
  double *thin_plate_matrix = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);//alloc_dmatrix(num_landmarks, num_landmarks);

  //r = ||{x[i],y[i]} - {x[j],y[j]} ||
  //U(r) = {r^2 log r, r>0} || {0, r = 0}
  //K[((i)*num_landmarks)+j] = U(r)
  //K is symmetric only need to calculate upper(or lower) triangle
  //K is upper NXN, matrix of L
 
  for (i = 0; i < num_landmarks; ++i) {
    L[i*(num_landmarks+4)] = 0.0;
    for (j = i+1; j < num_landmarks; ++j) {
      //r = ||{x[i],y[i]} - {x[j],y[j]} ||
      r_squared = (landmarks[i*2]-landmarks[j*2])*(landmarks[i*2]-landmarks[j*2]) + (landmarks[(i*2)+1]-landmarks[(j*2)+1])*(landmarks[(i*2)+1]-landmarks[(j*2)+1]);
      if (sqrt(r_squared) < 10e-6) {
        L[(j*(num_landmarks+3))+i] = L[(i*(num_landmarks+3))+j] = 0.0;
      }
      else {
        L[(j*(num_landmarks+3))+i] = L[(i*(num_landmarks+3))+j] = (r_squared*log10(r_squared)) / 2; //log(x^2) = log(x)/2
      }
    }
  }

  
  //set up P and P'
  //P is upper right NX3 matrix of L
  //P' is lower left 3XN matrix of L
  for (i = 0; i < num_landmarks; ++i) {
    L[(i*(num_landmarks+3))+num_landmarks] = L[(num_landmarks*(num_landmarks+3))+i] = 1.0;
    L[(i*(num_landmarks+3))+num_landmarks+1] = L[((num_landmarks+1)*(num_landmarks+3))+i] = landmarks[i*2];
    L[(i*(num_landmarks+3))+num_landmarks+2] = L[((num_landmarks+2)*(num_landmarks+3))+i] = landmarks[(i*2)+1];
  }

  //print_matrix(fp, L, num_landmarks+3, num_landmarks, num_landmarks);
  //fclose(fp);
  L_inverse = invert(L, num_landmarks+3);

  for (i = 0; i < num_landmarks; ++i) {
    for (j = 0; j < num_landmarks; ++j) {
      thin_plate_matrix[(i*num_landmarks)+j] = L_inverse[(i*(num_landmarks+3))+j];
    }
  }
  
  free_array((void *)L);
  free_array((void *)L_inverse);
 
  return thin_plate_matrix;
}

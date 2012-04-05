/************************************************************
 * slide_a_step.c                                           *
 *----------------------------------------------------------*
 *  Function to determine optimal sliding distance          *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cQpGenDense.h>
#include "shape.h"
#include "cdf.h"
/*******************************************************************************************************************************
 * slide_a_step_closed(true_target, num_landmarks, target_landmarks, tps_Matrix, num_landmarks, offset, bending_energy, error) *
 * --                                                                                                                          *
 *  slides the landmarks in target along the tangent directions of true_target                                                 *
 *******************************************************************************************************************************/
void closed_slide_a_step(double *true_target, int true_num_landmarks, double *target_offset, int num_landmarks, double *thin_plate_matrix, double *init_seg_length, double shape_rep_lambda, double *final_be, int *error) {
static slide_num = 0;
  int i, j, k;
  float proj_error = .1; //projected error, subject to change
  float S = .5; //default Catmull-Rom spline tension
  //double shape_rep_lambda = 1e-5 / num_landmarks;
  
  double *curve_radius = (double *) malloc(sizeof(double) * num_landmarks);
  double *Tan_x = (double *) calloc(num_landmarks * num_landmarks, sizeof(double)); //Tan_x matrix with x tangent values on the diagonal. Px in paper
  double *Tan_y = (double *) calloc(num_landmarks * num_landmarks, sizeof(double)); //Tan_y matrix with y tangent values on the diagonal. Py in paper
  double *landmarks_x = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double *landmarks_y = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double *slide = (double *) malloc(sizeof(double) * num_landmarks);
  
  point *pts;
  curve *true_curve = closed_curve_length(true_target, true_num_landmarks);

  /*********************************
   * calculate the current slide   *
   * points locations, tangents    *
   * and curvatures then normalize *
   * the tangents                  *
   *********************************/
  double length;
  for (i = 0; i < num_landmarks; ++i) {
    pts = closed_locate_point(true_target, true_num_landmarks, target_offset[i]/**init_offset + ((i == 0) ? (0.0) : (target_curve->accum_length[i-1]))*/);
    length = hypot(pts->x_tangent, pts->y_tangent);

    curve_radius[i] = 1/fabs(pts->curvature);
  
    //this is to initialize our Tan_x and Tan_y for
    //with x and y values being diagonally dominant
    //so Tan_x == Tan_x' and Tan_y == Tan_y'
    Tan_x[i*(num_landmarks+1)] = pts->x_tangent/length;
    Tan_y[i*(num_landmarks+1)]= pts->y_tangent/length;
    landmarks_x[i] = pts->x_coord;
    landmarks_y[i] = pts->y_coord;
    slide[i] = 0.0;
    free_array((void *)pts);
  }

  double *temp = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp2 = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp3 = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp4 = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double gamma[num_landmarks], phi[num_landmarks], qpy[0], z[num_landmarks], lambda[num_landmarks], pi[num_landmarks], objval;
  int ierr;
  double *H, *f, *A, *b, *lower_bound, *upper_bound, *DUMMY, *dummy, *clow;
  char *valid_lower_bound, *valid_upper_bound, *iclow, *valid_b;

  //allocate space for Quadratic Programming parameters
  //the two dummy variables are there bequase OOQP requires them but they arent
  //used for our case
  //newQpGenDense(&f, num_landmarks, &H, &lower_bound, &valid_lower_bound, &upper_bound, &valid_upper_bound, &DUMMY, 0, &dummy, &A, num_landmarks, &clow, &iclow, &b, &valid_b, &ierr);
  H = (double *) calloc(num_landmarks * num_landmarks, sizeof(double));
  f = (double *) calloc(num_landmarks, sizeof(double));
  A = (double *) calloc(num_landmarks * num_landmarks, sizeof(double));
  b = (double *) calloc(num_landmarks, sizeof(double));
  valid_b = (char *) calloc(num_landmarks, sizeof(char));
  clow = (double *) calloc(num_landmarks, sizeof(double));
  iclow = (char *) calloc(num_landmarks, sizeof(char));
  lower_bound = (double *) calloc(num_landmarks, sizeof(double));
  upper_bound = (double *) calloc(num_landmarks, sizeof(double));
  valid_lower_bound = (char *) calloc(num_landmarks, sizeof(char));
  valid_upper_bound = (char *) calloc(num_landmarks, sizeof(char));
  DUMMY = (double *) calloc(0, sizeof(double));
  dummy = (double *) calloc(0, sizeof(double));
 
  /******************************************************************
   * In paper H = (Px'*L*Px + Py'*L*Py) and f = (Px'*L*x + Py'*L*y) *
   * Px and Py are both diagonal matrices so Px' = Px and Py' = Py  *
   ******************************************************************/
  //Do Px'*L*Px
  // Tan_x' * thin_plate_matrix
  mat_mult(Tan_x, num_landmarks, thin_plate_matrix, num_landmarks, num_landmarks, temp);
  // (Tan_x' * thin_plate_matrix) * Tan_x
  mat_mult(temp, num_landmarks, Tan_x, num_landmarks, num_landmarks, H);
  
  //Do Py'*L*Py
  // Tan_y' * thin_plate_matrix
  mat_mult(Tan_y, num_landmarks, thin_plate_matrix, num_landmarks, num_landmarks, temp2);
  // (Tan_y' * thin_plate_matrix) * Tan_y
  mat_mult(temp2, num_landmarks, Tan_y, num_landmarks, num_landmarks, temp3);

  free_array((void *)Tan_x);
  free_array((void *)Tan_y);

  //Do Px'*L*x
  mat_mult(temp, num_landmarks, landmarks_x, num_landmarks, 1, f);
  //Do Py'*L*y
  mat_mult(temp2, num_landmarks, landmarks_y, num_landmarks, 1, temp4);

  free_array((void *)landmarks_x);
  free_array((void *)landmarks_y);

  //this yields (Tan_x * thin_plate_matrix * Tan_x) + (Tan_y * thin_plate_matrix * Tan_y) our H
  for (i = 0; i < num_landmarks; ++i) {
      f[i] += temp4[i];
    for (j = 0; j < num_landmarks; ++j) {
      H[(i*num_landmarks)+j] += temp3[(i*num_landmarks)+j];
    }
  }
  
  free_array((void *)temp);
  free_array((void *)temp2);
  free_array((void *)temp3);
  free_array((void *)temp4);

  //this is our topology preservation
  //quadratic programing will solve with the constraint
  //Ax <= b
  //so set up such that landmarks cant slide past their neighbors
  double landmark_rep_error;
  for (i = 0; i < num_landmarks; ++i) {
    A[i*(num_landmarks+1)] = 1.0;
    if (i != num_landmarks-1) {
      A[i*(num_landmarks+1)+1] = -1.0;
    }
    else {
      A[i] = -1.0;
    }
    b[i] = fmod(true_curve->curve_length + target_offset[(i+1)%num_landmarks] - target_offset[i], true_curve->curve_length);
    valid_b[i] = 1;
    clow[i] = 0.0;
    iclow[i] = 0;

    //add landmark representation error into H and fa
    landmark_rep_error = shape_rep_lambda / (init_seg_length[i]*init_seg_length[i]);
    H[i*(num_landmarks+1)] += landmark_rep_error;
    H[((i+1)%num_landmarks)*(num_landmarks+1)] += landmark_rep_error;
    H[(i*num_landmarks)+((i+1)%num_landmarks)] -= landmark_rep_error;
    H[((i+1)%num_landmarks)*(num_landmarks)+i] -= landmark_rep_error;
    f[i] -= 2 * (b[i] - init_seg_length[i]) * landmark_rep_error;
    f[(i+1)%num_landmarks] += 2 * (b[i] - init_seg_length[i]) * landmark_rep_error;
    
    //error constraint from projection
    upper_bound[i] = (sqrt((2*proj_error*curve_radius[i])+(proj_error*proj_error)))*.5;
    valid_upper_bound[i] = 1;
    lower_bound[i] = -upper_bound[i];
    valid_lower_bound[i] = 1;
  }
  free_array((void *) curve_radius);
  
  //do Quadratic Programming
  //min  0.5 x'*H*x + f'*x
  //Subject to A*x <= b
  //        low <= x <= up
  qpsolvede(f, num_landmarks, H, lower_bound, valid_lower_bound, upper_bound, valid_upper_bound, DUMMY, 0, dummy, A, num_landmarks, clow, iclow, b, valid_b, slide, gamma, phi, qpy, z, lambda, pi, &objval, 0, error);

 //for getting landmarks before error 
  if (*error) {
    for (i = 0; i < num_landmarks; ++i) {
      slide[i] = 0.0;
    }
  }
    
  //free up space allocated to Quadratic Programming parameters
  //freeQpGenDense(&f, &H, &lower_bound, &valid_lower_bound, &upper_bound, &valid_upper_bound, &DUMMY, &dummy, &A, &clow, &iclow, &b, &valid_b);
  free_array((void *)H);
  free_array((void *)f);
  free_array((void *)A);
  free_array((void *)b);
  free_array((void *)clow);
  free_array((void *)lower_bound);
  free_array((void *)upper_bound);
  free_array((void *)valid_b);
  free_array((void *)iclow);
  free_array((void *)valid_lower_bound);
  free_array((void *)valid_upper_bound);
  free_array((void *)DUMMY);
  free_array((void *)dummy);
  
  double *x = (double *) malloc(sizeof(double) * num_landmarks * 1);//in 1d array x = x'
  double *y = (double *) malloc(sizeof(double) * num_landmarks * 1);//in 1d array y = y'
  double *be_temp = (double *) malloc(sizeof(double) * 1 * num_landmarks);
  double *be_temp2 = (double *) malloc(sizeof(double));

  for (i = 0; i < num_landmarks; ++i) {
    point *pts, *pts_ooqp;
    target_offset[i] = target_offset[i] + slide[i];
    pts = closed_locate_point(true_target, true_num_landmarks, target_offset[i]/**init_offset + slide[i] + ((i == 0) ? (0.0) : (target_curve->accum_length[i-1]))*/);
    x[i] = pts->x_coord;
    y[i] = pts->y_coord;
    free_array((void *)pts);
  }

  free_array((void *)slide);
  
  //be = x'Lx + y'Ly
  mat_mult(x, 1, thin_plate_matrix, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, x, num_landmarks, 1, be_temp2);
  *final_be = *be_temp2;
  mat_mult(y, 1, thin_plate_matrix, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, y, num_landmarks, 1, be_temp2);
  *final_be += *be_temp2;

  kill_curve(true_curve);
  free_array((void *) be_temp2);
  free_array((void *) be_temp);
  free_array((void *) x);
  free_array((void *) y);
  
}

/*******************************************************************************************************************************
 * open_slide_a_step(true_target, num_landmarks, target_landmarks, tps_Matrix, num_landmarks, offset, bending_energy, error)   *
 * --                                                                                                                          *
 *  slides the landmarks in target along the tangent directions of true_target, leaving the first and last fixed               *
 *******************************************************************************************************************************/
void open_slide_a_step(double *true_target, int true_num_landmarks, double *target_offset, int num_landmarks, double *thin_plate_matrix, double *init_seg_length, double shape_rep_lambda, double *final_be, int *error) {
static slide_num = 0;
  int i, j, k;
  float proj_error = .1; //projected error, subject to change
  float S = .5; //default Catmull-Rom spline tension
  //double shape_rep_lambda = 1e-5 / num_landmarks;
  
  double *curve_radius = (double *) malloc(sizeof(double) * num_landmarks);
  double *Tan_x = (double *) calloc(num_landmarks * num_landmarks, sizeof(double)); //Tan_x matrix with x tangent values on the diagonal. Px in paper
  double *Tan_y = (double *) calloc(num_landmarks * num_landmarks, sizeof(double)); //Tan_y matrix with y tangent values on the diagonal. Py in paper
  double *landmarks_x = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double *landmarks_y = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double *slide = (double *) malloc(sizeof(double) * num_landmarks);
  
  point *pts;
  curve *true_curve = open_curve_length(true_target, true_num_landmarks);

  /*********************************
   * calculate the current slide   *
   * points locations, tangents    *
   * and curvatures then normalize *
   * the tangents                  *
   *********************************/
  double length;
  for (i = 0; i < num_landmarks; ++i) {
    pts = open_locate_point(true_target, true_num_landmarks, target_offset[i]/**init_offset + ((i == 0) ? (0.0) : (target_curve->accum_length[i-1]))*/);
    length = hypot(pts->x_tangent, pts->y_tangent);

    curve_radius[i] = 1/fabs(pts->curvature);
  
    //this is to initialize our Tan_x and Tan_y for
    //with x and y values being diagonally dominant
    //so Tan_x == Tan_x' and Tan_y == Tan_y'
    Tan_x[i*(num_landmarks+1)] = pts->x_tangent/length;
    Tan_y[i*(num_landmarks+1)]= pts->y_tangent/length;
    landmarks_x[i] = pts->x_coord;
    landmarks_y[i] = pts->y_coord;
    slide[i] = 0.0;
    free_array((void *)pts);
  }

  double *temp = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp2 = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp3 = (double *) malloc(sizeof(double) * num_landmarks * num_landmarks);
  double *temp4 = (double *) malloc(sizeof(double) * num_landmarks * 1);
  double gamma[num_landmarks], phi[num_landmarks], qpy[0], z[num_landmarks], lambda[num_landmarks], pi[num_landmarks], objval;
  int ierr;
  double *H, *f, *topology_matrix, *topology_vector, *lower_bound, *upper_bound, *endpoint_matrix, *endpoint_vector, *clow;
  char *valid_lower_bound, *valid_upper_bound, *iclow, *valid_topology;

  //allocate space for Quadratic Programming parameters
  //the two dummy variables are there bequase OOQP requires them but they arent
  //used for our case
  //newQpGenDense(&f, num_landmarks, &H, &lower_bound, &valid_lower_bound, &upper_bound, &valid_upper_bound, &DUMMY, 0, &dummy, &A, num_landmarks, &clow, &iclow, &b, &valid_b, &ierr);
  H = (double *) calloc(num_landmarks * num_landmarks, sizeof(double));
  f = (double *) calloc(num_landmarks, sizeof(double));
  topology_matrix = (double *) calloc((num_landmarks-1) * num_landmarks, sizeof(double));
  topology_vector = (double *) calloc((num_landmarks-1), sizeof(double));
  valid_topology = (char *) calloc((num_landmarks-1), sizeof(char));
  clow = (double *) calloc((num_landmarks-1), sizeof(double));
  iclow = (char *) calloc((num_landmarks-1), sizeof(char));
  lower_bound = (double *) calloc(num_landmarks, sizeof(double));
  upper_bound = (double *) calloc(num_landmarks, sizeof(double));
  valid_lower_bound = (char *) calloc(num_landmarks, sizeof(char));
  valid_upper_bound = (char *) calloc(num_landmarks, sizeof(char));
  endpoint_matrix = (double *) calloc(2 * num_landmarks, sizeof(double));
  endpoint_vector = (double *) calloc(2, sizeof(double));
 
  /******************************************************************
   * In paper H = (Px'*L*Px + Py'*L*Py) and f = (Px'*L*x + Py'*L*y) *
   * Px and Py are both diagonal matrices so Px' = Px and Py' = Py  *
   ******************************************************************/
  //Do Px'*L*Px
  // Tan_x' * thin_plate_matrix
  mat_mult(Tan_x, num_landmarks, thin_plate_matrix, num_landmarks, num_landmarks, temp);
  // (Tan_x' * thin_plate_matrix) * Tan_x
  mat_mult(temp, num_landmarks, Tan_x, num_landmarks, num_landmarks, H);
  
  //Do Py'*L*Py
  // Tan_y' * thin_plate_matrix
  mat_mult(Tan_y, num_landmarks, thin_plate_matrix, num_landmarks, num_landmarks, temp2);
  // (Tan_y' * thin_plate_matrix) * Tan_y
  mat_mult(temp2, num_landmarks, Tan_y, num_landmarks, num_landmarks, temp3);

  free_array((void *)Tan_x);
  free_array((void *)Tan_y);

  //Do Px'*L*x
  mat_mult(temp, num_landmarks, landmarks_x, num_landmarks, 1, f);
  //Do Py'*L*y
  mat_mult(temp2, num_landmarks, landmarks_y, num_landmarks, 1, temp4);

  free_array((void *)landmarks_x);
  free_array((void *)landmarks_y);

  //this yields (Tan_x * thin_plate_matrix * Tan_x) + (Tan_y * thin_plate_matrix * Tan_y) our H
  for (i = 0; i < num_landmarks; ++i) {
      f[i] += temp4[i];
    for (j = 0; j < num_landmarks; ++j) {
      H[(i*num_landmarks)+j] += temp3[(i*num_landmarks)+j];
    }
  }
  
  free_array((void *)temp);
  free_array((void *)temp2);
  free_array((void *)temp3);
  free_array((void *)temp4);

  //this is our topology preservation
  //quadratic programing will solve with the constraint
  //Ax <= b
  //so set up such that landmarks cant slide past their neighbors
  double landmark_rep_error;
  for (i = 0; i < (num_landmarks-1); ++i) {
    topology_matrix[i*(num_landmarks+1)] = 1.0;
    topology_matrix[i*(num_landmarks+1)+1] = -1.0;
    topology_vector[i] = fmod(true_curve->curve_length + target_offset[(i+1)%num_landmarks] - target_offset[i], true_curve->curve_length);
    valid_topology[i] = 1;
    clow[i] = 0.0;
    iclow[i] = 0;

    //add landmark representation error into H and f
    landmark_rep_error = shape_rep_lambda / (init_seg_length[i]*init_seg_length[i]);
    H[i*(num_landmarks+1)] += landmark_rep_error;
    H[(i+1)*(num_landmarks+1)] += landmark_rep_error;
    H[(i*num_landmarks)+(i+1)] -= landmark_rep_error;
    H[(i+1)*(num_landmarks)+i] -= landmark_rep_error;
    f[i] -= 2 * (topology_vector[i] - init_seg_length[i]) * landmark_rep_error;
    f[i+1] += 2 * (topology_vector[i] - init_seg_length[i]) * landmark_rep_error;
    
    //error constraint from projection
    upper_bound[i] = (sqrt((2*proj_error*curve_radius[i])+(proj_error*proj_error)))*.5;
    valid_upper_bound[i] = 1;
    lower_bound[i] = -upper_bound[i];
    valid_lower_bound[i] = 1;
  }

  //add landmark representation error into H and fa
  H[i*(num_landmarks+1)] += landmark_rep_error;
  H[0] += landmark_rep_error;
  H[(i*num_landmarks)] -= landmark_rep_error;
  H[i] -= landmark_rep_error;
  f[i] -= 2 * (topology_matrix[i] - init_seg_length[i]) * landmark_rep_error;
  f[0] += 2 * (topology_vector[i] - init_seg_length[i]) * landmark_rep_error;

  upper_bound[i] = lower_bound[i] = upper_bound[0] = lower_bound[0] = 0.0;
  valid_upper_bound[i] = valid_lower_bound[i] = 1;
  free_array((void *) curve_radius);

  //dont let the first and last landmarks slide
  //endpoint_matrix[0] = endpoint_matrix[(2 * num_landmarks) -1] = 1.0;
  //b_equal[0] = b_equal[1] = 0.0; //b is calloc'ed so shouldnt need this
  
  //do Quadratic Programming
  //min  0.5 x'*H*x + f'*x
  //Subject to A*x <= b
  //        low <= x <= up
  qpsolvede(f, num_landmarks, H, lower_bound, valid_lower_bound, upper_bound, valid_upper_bound, endpoint_matrix, 0, endpoint_vector, topology_matrix, num_landmarks-1, clow, iclow, topology_vector, valid_topology, slide, gamma, phi, qpy, z, lambda, pi, &objval, 0, error);
 //for getting landmarks before error 
 if (*error) {
   for (i = 0; i < num_landmarks; ++i) {
     //slide[i] = 0.0;
     fprintf(stderr, "[%lf]", slide[i]);
   }
   fprintf(stderr, "\n");
 }
    
  //free up space allocated to Quadratic Programming parameters
  //freeQpGenDense(&f, &H, &lower_bound, &valid_lower_bound, &upper_bound, &valid_upper_bound, &DUMMY, &dummy, &A, &clow, &iclow, &b, &valid_b);
  free_array((void *)H);
  free_array((void *)f);
  free_array((void *)topology_matrix);
  free_array((void *)topology_vector);
  free_array((void *)clow);
  free_array((void *)lower_bound);
  free_array((void *)upper_bound);
  free_array((void *)valid_topology);
  free_array((void *)iclow);
  free_array((void *)valid_lower_bound);
  free_array((void *)valid_upper_bound);
  free_array((void *)endpoint_matrix);
  free_array((void *)endpoint_vector);
  
  double *x = (double *) malloc(sizeof(double) * num_landmarks * 1);//in 1d array x = x'
  double *y = (double *) malloc(sizeof(double) * num_landmarks * 1);//in 1d array y = y'
  double *be_temp = (double *) malloc(sizeof(double) * 1 * num_landmarks);
  double *be_temp2 = (double *) malloc(sizeof(double));

  for (i = 0; i < num_landmarks; ++i) {
    point *pts, *pts_ooqp;
    target_offset[i] = target_offset[i] + slide[i];
    pts = open_locate_point(true_target, true_num_landmarks, target_offset[i]/**init_offset + slide[i] + ((i == 0) ? (0.0) : (target_curve->accum_length[i-1]))*/);
    x[i] = pts->x_coord;
    y[i] = pts->y_coord;
    free_array((void *)pts);
  }

  free_array((void *)slide);
  
  //be = x'Lx + y'Ly
  mat_mult(x, 1, thin_plate_matrix, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, x, num_landmarks, 1, be_temp2);
  *final_be = *be_temp2;
  mat_mult(y, 1, thin_plate_matrix, num_landmarks, num_landmarks, be_temp);
  mat_mult(be_temp, 1, y, num_landmarks, 1, be_temp2);
  *final_be += *be_temp2;

  kill_curve(true_curve);
  free_array((void *) be_temp2);
  free_array((void *) be_temp);
  free_array((void *) x);
  free_array((void *) y);
  //fprintf(stderr, "lnd: %d|%d[%p|%p]\n", num_landmarks, true_num_landmarks, &num_landmarks, &true_num_landmarks);  
}


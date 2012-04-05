/*************************************************************
 * shape_utils.c                                             *
 *-----------------------------------------------------------*
 *  Code for testing correspondance algorithm and additional *
 *  utilities                                                *
 *-----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang       *  
 *  Last modified: October 26, 2006                          *
 *************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include "cdf.h"
#include "shape.h"

/***************************************************************
 * store_data_cdf(cdf, landmarks, zvar, majority, number)      *
 * --                                                          *
 *  Store the landmarks in a new zvariable in the CDF file     *
 ***************************************************************/
void store_data_cdf(CDFid id, double *data, long zVar, long majority, int rows, int cols) {
  int i, j;
  long pos[2] = {0L, 0L};

  CDFsetzVarSeqPos(id, zVar, 0, pos);
  if (majority == COLUMN_MAJOR) {

    for (i = 0; i < cols; ++i) {
      for (j = 0; j < rows; ++j) {
        CDFputzVarSeqData(id, zVar, data+(j*cols)+i);
      }
    }
    
  }
  else if (majority = ROW_MAJOR) {

    for (i = 0; i < rows; ++i) {
      for (j = 0; j < cols; ++j) {
        CDFputzVarSeqData(id, zVar, data+(i*cols)+j);
      }
    }

  }
  else {
    error("Invalid majority for CDF file.\n");
  }
}

/***************************************************************
 * load_data_cdf(cdf, landmarks, zvar, majority, number)       *
 * --                                                          *
 *  Store the landmarks in a new zvariable in the CDF file     *
 ***************************************************************/
double * load_data_cdf(CDFid id, long zVar, long majority, int rows, int cols) {
  int i, j;
  long pos[2] = {0L, 0L};
  double *data = (double *) malloc(sizeof(double) * rows * cols);

  CDFsetzVarSeqPos(id, zVar, 0, pos);

  if (majority == COLUMN_MAJOR) {

    for (i = 0; i < cols; ++i) {
      for (j = 0; j < rows; ++j) {
        CDFgetzVarSeqData(id, zVar, data+(j*cols)+i);
      }
    }
    
  }
  else if (majority = ROW_MAJOR) {

    for (i = 0; i < rows; ++i) {
      for (j = 0; j < cols; ++j) {
        CDFgetzVarSeqData(id, zVar, data+(i*cols)+j);
      }
    }

  }
  else {
    error("Invalid majority for CDF file.\n");
  }

  return data;
}

/*********************************************
 * free_matrix(matrix, rows)                 *
 * --                                        *
 *  free memory allocated for an array of ** *
 *********************************************/
void free_matrix(void **m, int rows) {
  int i;
  //free column space for each row
  for (i = 0; i < rows; i++) {
    free_array((void *) m[i]);
    m[i] = NULL;
  }
  //free rows
  free_array((void *) m);
  m = NULL;
}

inline void free_array(void *a) {
  free(a);
  a = NULL;
}


/*
 */
inline void kill_curve(curve *dead_curve) {
  free_array((void *) dead_curve->seg_length);
  free_array((void *) dead_curve->accum_length);
  free_array((void *) dead_curve);
}

/********************************************
 * init_tangent(cubic, tension)             *
 * --                                       *
 *  Initialize matrix with cubic derivative *
 *   over u.                                *
 ********************************************/
void init_tangent(double *pm, float S) {
  int i;
  long double u;
  //poly = 3u^2 + 2u + 1 + 0
  for (u = 0.0; u <= 1.01; u += 0.01) {
    i = u*100;
    pm[i*4] = S*3*u*u;
    pm[(i*4)+1] = S*2*u;
    pm[(i*4)+2] = S*1;
    pm[(i*4)+3] = 0;
  }
}

/***************************************************************
 * mat_mult(matrix1, rows1, matrix2, col1/rows2, col2, return) *
 * --                                                          *
 *  Multiplies matrix1 by matrix2. Puts the result in return.  *
 ***************************************************************/
void mat_mult(double *mat1, int r1, double *mat2, int c1r2, int c2, double *ret) {
  int i, j, k;
  for (i = 0; i < r1; ++i) {
    for (j = 0; j < c2; ++j) {
      register double ret_reg = 0;//suggest this be a register
      for (k = 0; k < c1r2; ++k) {
	      ret_reg += mat1[(i*c1r2)+k] * mat2[(k*c2)+j];
      }
      ret[(i*c2)+j] = ret_reg;
    }
  }
}

/**************************************
 * transpose(matrix, rows, columns)   *
 * --                                 *
 *  Returns the transpose of a matrix *
 **************************************/
inline double *transpose(double *m, int rows, int cols) {
  int i, j;
  double *m_ = (double *) malloc(sizeof(double) * rows * cols);

  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      m_[(j*rows)+i] = m[(i*cols)+j];
    }
  }
  
  return m_;
}

/******************************************
 * sum_l2norm(vector_matrix, rows)        *
 * --                                     *
 *  Calculates the L2 norm of the vectors *
 *  and sums them.                        *
 ******************************************/
inline double sum_l2norm(double *V, int rows) {
  int i;
  register double sum = 0.0;
  for (i = 0; i < rows; i++) {
    sum += hypot(V[i*2], V[(i*2)+1]);
  }
  return sum;
}

/*******************************************
 * invert(matrix, size)                    *
 * --                                      *
 *  Calculates the inverse of matrix using *
 *  Partial-Pivoting Gaussian Elimination *
 *******************************************/
double *invert(double *m, int n) {
  int i,j,k;
  int *pivot = (int *) malloc(sizeof(int)*n);
  double *identity = (double *) malloc(sizeof(double) * n * n);
  double *inverse = (double *) malloc(sizeof(double) * n * n);
  double *m_prime = (double *) malloc(sizeof(double) * n * n);

  //setup identity matrix and pivot
  //copy m into m_prime
  for (i = 0; i < n; ++i) {
    identity[i*(n+1)] = 1.0;
    m_prime[i*(n+1)] = m[i*(n+1)];
    pivot[i] = i;
    for (j = i+1; j < n; ++j) {
      identity[(j*n)+i] = identity[(i*n)+j] = 0.0;
      m_prime[(i*n)+j] = m[(i*n)+j];
      m_prime[(j*n)+i] = m[(j*n)+i];
    }
  }

  gauss_elim(m_prime, pivot, n);

  for (i = 0; i < n-1; ++i) {
    for (j = i+1; j < n; ++j) {
      for (k = 0; k < n; ++k) {
        identity[(pivot[j]*n)+k] -= m_prime[(pivot[j]*n)+i] * identity[(pivot[i]*n)+k];
      }
    }
  }

  for (i = 0; i < n; ++i) {
    inverse[((n-1)*n)+i] = identity[(pivot[n-1]*n)+i] / m_prime[(pivot[n-1]*n)+n-1];
    
    for (j = n-2; j >= 0; j = j-1) {
      inverse[(j*n)+i] = identity[(pivot[j]*n)+i];
      
      for (k = j+1; k < n; ++k) {
        inverse[(j*n)+i] -= m_prime[(pivot[j]*n)+k] * inverse[(k*n)+i];
      }
      
      inverse[(j*n)+i] /= m_prime[(pivot[j]*n)+j];
    }
  }
  
  free_array((void *)pivot);
  free_array((void *)identity);
  free_array((void *)m_prime);

  return inverse;
}

/************************************************
 * gauss_elim(matrix, pivots, size)             *
 * --                                           *
 *  Performs Gaussian Elimination on matrix     *
 *  sizeXsize, storing the pivot order in pivot *
 ************************************************/
void gauss_elim(double *m, int *pivot, int n) {
  int i, j, k, max, temp;
  double s, s_max, pivot_el, pivot_el_max;
  double *scale = (double *) malloc(sizeof(double)*n);

  //print_matrix(stdout, m, n, n);
  
  //find largest value in each row
  for (i = 0; i < n; ++i) {
    s = s_max = 0;
    for (j = 0; j < n; ++j) {
      if ((s = fabs(m[(i*n)+j])) > s_max) {
        s_max = s;
      }
    }
    scale[i] = s_max;
  }

  //get largest element from each column
  for (j = 0; j < n-1; ++j) {
    pivot_el_max = -10e37;
    for (i = j; i < n; ++i) {
      pivot_el = fabs(m[(pivot[i]*n)+j]) / scale[pivot[i]];
      if (pivot_el > pivot_el_max) {
        pivot_el_max = pivot_el;
        max = i;
      }
    }
    //Interchange Row op
    temp = pivot[j];
    pivot[j] = pivot[max];
    pivot[max] = temp;
    for (i = j+1; i < n; ++i) {
      m[(pivot[i]*n)+j] /= m[(pivot[j]*n)+j];

      //other elements are affected
      for (k = j+1; k < n; ++k) {
        m[(pivot[i]*n)+k] -= (m[(pivot[i]*n)+j] * m[(pivot[j]*n)+k]);
      }
    }
  }
  
  free_array((void *)scale);
}

/*********************************************
 * general_procrustes(landmark_array)        *
 * --                                        *
 *  Performs a general 2D Procrustes fit for *
 *  all shapes in the array                  *
 *********************************************/
double **general_procrustes(double **shapes, int num_shapes, int num_landmarks, double *translate, double *scale, double *rotate) {
  int i, j, k, num_new_rotations;
  double new_rotation, new_scale;
  double *shape_, *shape_j_t, *shape_t, *temp, *diag, *right_ortho, *R;
  double **shapes_fit = (double **) malloc(sizeof(double *) * num_shapes);

  for (i = 0; i < num_shapes; ++i) {
    shapes_fit[i] = (double *) malloc(sizeof(double) * num_landmarks * 2);
    translate[(i*2)] = translate[(i*2)+1] = 0.0; 
 
    //find the mean and scaling factor of the shape
    for (j = 0; j < num_landmarks; ++j) {
      translate[i*2] += shapes[i][j*2];
      translate[(i*2)+1] += shapes[i][(j*2)+1];
      scale[i] += (shapes[i][j*2]*shapes[i][j*2]) + (shapes[i][(j*2)+1]*shapes[i][(j*2)+1]);
    } 
    translate[i*2] /= num_landmarks;
    translate[(i*2)+1] /= num_landmarks;
    scale[i] = 1/scale[i];

    //remove translation and scaling from the shape
    for (j = 0; j < num_landmarks; ++j) {
      shapes_fit[i][j*2] = (shapes[i][j*2] - translate[i*2]) * scale[i];
      shapes_fit[i][(j*2)+1] = (shapes[i][(j*2)+1] - translate[(i*2)+1]) * scale[i];
    }

  }

  diag = (double *) calloc(4, sizeof(double));
  right_ortho = (double *) calloc(4, sizeof(double));
  R = (double *) calloc(4, sizeof(double));

  for (i = 0; i < 100; ++i) {

    //find mean across shapess
    shape_ = (double *) calloc(num_landmarks * 2, sizeof(double));
    for (j = 0; j < num_landmarks; ++j) {
      for (k = 0; k < num_shapes; ++k) {
        shape_[j*2] += shapes_fit[k][j*2];
        shape_[(j*2)+1] += shapes_fit[k][(j*2)+1];
      }
      shape_[j*2] /= num_shapes;
      shape_[(j*2)+1] /= num_shapes;
    }

    num_new_rotations = 0;
    for (j = 0; j < num_shapes; ++j) {


      temp = (double *) malloc(sizeof(double) * 4);
      shape_j_t = transpose(shapes_fit[j], num_landmarks, 2);
      mat_mult(shape_j_t, 2, shape_, num_landmarks, 2, temp);
      svd(temp, 2, 2, diag, right_ortho);
      
      //2x2 matrices just do explicit multiply
      R[0] = temp[0]*right_ortho[0] + temp[1]*right_ortho[1];
      R[1] = temp[0]*right_ortho[2] + temp[1]*right_ortho[3];
      R[2] = temp[2]*right_ortho[0] + temp[3]*right_ortho[1];
      R[3] = temp[2]*right_ortho[2] + temp[3]*right_ortho[3];

      //determine if rotation can be removed
      new_rotation = acos(R[0]);
      if (new_rotation > 10e-6) {
        rotate[j] += new_rotation;
        ++num_new_rotations;
      }

      //check for new scaling after previous rotation
      new_scale = 0.0;
      for (k = 0; k < num_landmarks; ++k) {
        new_scale += shapes_fit[j][k*2]*shapes_fit[j][k*2] + shapes_fit[j][(k*2)+1]*shapes_fit[j][(k*2)+1];
      }

      shape_t = transpose(shape_, num_landmarks, 2);
      mat_mult(shape_t, 2, shapes_fit[j], num_landmarks, 2, temp);
      new_scale = ((temp[0]*R[0] + temp[1]*R[2]) + (temp[2]*R[1] + temp[3]*R[3])) / new_scale;
      if (new_scale < 0.0) {
        fprintf(stderr, "YIPPEE\n");
      }
      scale[j] *= new_scale;

      free_array((void *) temp);
      temp = (double *) malloc(sizeof(double) * num_landmarks * 2);

      //remove scaling transformation
      for (k = 0; k < num_landmarks; ++k) {
        shapes_fit[j][k*2] *= new_scale;
        shapes_fit[j][(k*2)+1] *= new_scale;
      }
      //remove rotation
      mat_mult(shapes_fit[j], num_landmarks, R, 2, 2, temp);
      for (k = 0; k < (num_landmarks * 2); ++k) {
        shapes_fit[j][k] = temp[k];
      }

      free_array((void *) temp);
      free_array((void *) shape_j_t);
      free_array((void *) shape_t);
    }

    free_array((void *) shape_);

    if (num_new_rotations == 0) {
      break;
    }
  }
  free_array((void *) diag);
  free_array((void *) right_ortho);
  free_array((void *) R);
  
  return shapes_fit;
}

/***************************************************
 * covariance                                      *
 * --                                              *
 *  Determines the mean shape for the shape space. *
 *  Calculates teh covariance matrix using:        *
 *          1                                      *
 *  cov = ----- = sum(dx[i] * dx[i]')              *
 *        (N-1)                                    *
 *  Where dx[i] = x[i] - mean_x                    *
 *  If bias is one then divide by N instead of N-1 *
 ***************************************************/
double *covariance(double **shapes, double *shape_space_mean, int num_landmarks, int num_shapes, int bias) {
  int i, j, k;
  double *shape = (double *) malloc(sizeof(double) * num_landmarks * 2);
  double *covariance_matrix = (double *) malloc(sizeof(double) * 4 * num_landmarks * num_landmarks);
  double *temp_matrix = (double *) malloc(sizeof(double) * 4 * num_landmarks * num_landmarks);
  
  //find mean shape
  for (i = 0; i < num_shapes; ++i) {
    for (j = 0; j < num_landmarks; ++j) {
      shape_space_mean[j*2] += (shapes[i][j*2] / num_shapes);
      shape_space_mean[(j*2)+1] += (shapes[i][(j*2)+1] / num_shapes);
    }
  }

  //        1 
  //cov = ----- sum(dx*dx')
  //      (N-1)

  //subtract out the mean
  for (j = 0; j < (num_landmarks*2); ++j) {
    shape[j] = shapes[0][j] - shape_space_mean[j];
  }
  //multiply dx*dx'
  //shape = shape' the way we have it set up, just indexed differently
  mat_mult(shape, num_landmarks*2, shape, 1, num_landmarks*2, covariance_matrix);
  
  
  for (i = 1; i < num_shapes; ++i) {
    //subtract out the mean
    for (j = 0; j < num_landmarks*2; ++j) {
      shape[j] = shapes[i][j] - shape_space_mean[j];
    }
    //multiply dx*dx'
    //shape = hape' the way we have it set up, just indexed differently
    mat_mult(shape, num_landmarks*2, shape, 1, num_landmarks*2, temp_matrix);

    //sum into the existing covariance
    for (j = 0; j < (num_landmarks*num_landmarks*4); ++j) {
      covariance_matrix[j] += temp_matrix[j];
    }
  }

  //divide by N or N-1 depending
  for (i = 0; i < (num_landmarks*num_landmarks*2); ++i) {
    covariance_matrix[i] /= (num_shapes - ((bias) ? 0 : 1));
  }

  free_array((void *)shape);
  free_array((void *)temp_matrix);

  return covariance_matrix; 
}

/***************************
 * pythag(double, double)  *
 * --                      *
 ***************************/
double pythag(double a, double b) {
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt) {
      ct = bt / at; result = at * sqrt(1.0 + ct * ct);
    }
    else if (bt > 0.0) {
      ct = at / bt; result = bt * sqrt(1.0 + ct * ct);
    }
    else {
      result = 0.0;
    }
    
    return(result);
}

/********************************************************
 * svd(matrix, rows, columns, diagonal, matrix)         *
 * --                                                   *
 *  Performs the Single Value Decomposition of a matrix *
 *  a = udv'.  Here a is overwritten with u, and w is d *
 ********************************************************/
//This code was taken from http://www.public.iastate.edu/~dicook/JSS/paper/code/svd.c
//This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is 
//code from Numerical Recipes adapted by Luke Tierney and David Betz.
//it has been modified slightly

int svd(double *a, int m, int n, double *w, double *v) {
  int flag, i, its, j, jj, k, l, nm;
  double c, f, h, s, x, y, z;
  double anorm = 0.0, g = 0.0, scale = 0.0;
  double *rv1;
  
  if (m < n) {
    fprintf(stderr, "#rows must be > #cols \n");
    return(0);
  }
  
  rv1 = (double *) malloc(n * sizeof(double));

//Householder reduction to bidiagonal form 
  for (i = 0; i < n; ++i) {
    //left-hand reduction 
    l = i + 1;
    rv1[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m) {
      for (k = i; k < m; ++k) {
        scale += fabs(a[(k*n)+i]);
      }
      if (scale) {
        for (k = i; k < m; ++k) {
          a[(k*n)+i] = a[(k*n)+i]/scale;
          s += a[(k*n)+i] * a[(k*n)+i];
        }
        f = a[(i*n)+i];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[(i*n)+i] = (f - g);
        if (i != n - 1) {
          for (j = l; j < n; ++j) {
            for (s = 0.0, k = i; k < m; ++k) {
              s += a[(k*n)+i] * a[(k*n)+j];
            }
            f = s / h;
            for (k = i; k < m; ++k) {
              a[(k*n)+j] += f * a[(k*n)+i];
            }
          }
        }
        for (k = i; k < m; ++k) {
          a[(k*n)+i] = a[(k*n)+i]*scale;
        }
      }
    }
    w[i] = (scale * g);
    
    //right-hand reduction 
    g = s = scale = 0.0;
    if (i < m && i != n - 1) {
      for (k = l; k < n; ++k) { 
        scale += fabs(a[(i*n)+k]);
      }
      if (scale) {
        for (k = l; k < n; ++k) {
          a[(i*n)+k] = a[(i*n)+k]/scale;
          s += a[(i*n)+k] * a[(i*n)+k];
        }
        f = a[(i*n)+l];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[(i*n)+l] = (f - g);
        for (k = l; k < n; ++k) {
          rv1[k] = a[(i*n)+k] / h;
        }
        if (i != m - 1) {
          for (j = l; j < m; ++j) {
            
            for (s = 0.0, k = l; k < n; ++k) {
              s += a[(j*n)+k] * a[(i*n)+k];
            }
            for (k = l; k < n; ++k) {
              a[(j*n)+k] += s * rv1[k];
            }
            
          }
        }
        for (k = l; k < n; ++k) {
          a[(i*n)+k] = a[(i*n)+k]*scale;
        }
      }
    }
    anorm = MAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
  }
  
  //accumulate the right-hand transformation 
  for (i = n - 1; i >= 0; --i) {
    if (i < n - 1) {
      if (g) {
        for (j = l; j < n; ++j) {
          v[(j*n)+i] = a[(i*n)+j] / a[(i*n)+l] / g;
        }
        //double division to avoid underflow 
        for (j = l; j < n; ++j) {
          
          for (s = 0.0, k = l; k < n; ++k) {
            s += a[(i*n)+k] * v[(k*n)+j];
          }
          for (k = l; k < n; ++k) {
            v[(k*n)+j] += s * v[(k*n)+i];
          }
        }
      }
      for (j = l; j < n; ++j) {
        v[(i*n)+j] = v[(j*n)+i] = 0.0;
      }
    }
    v[(i*n)+i] = 1.0;
    g = rv1[i];
    l = i;
  }
  
  //accumulate the left-hand transformation 
  for (i = n - 1; i >= 0; --i) {
    l = i + 1;
    g = w[i];
    if (i < n - 1) {
      for (j = l; j < n; ++j) {
        a[(i*n)+j] = 0.0;
      }
    }
    if (g) {
      g = 1.0 / g;
      if (i != n - 1) {
        for (j = l; j < n; ++j) {
          
          for (s = 0.0, k = l; k < m; ++k) {
            s += a[(k*n)+i] * a[(k*n)+j];
          }
          f = (s / a[(i*n)+i]) * g;
          for (k = i; k < m; ++k) {
            a[(k*n)+j] += f * a[(k*n)+i];
          }
        }
      }
      for (j = i; j < m; ++j) {
        a[(j*n)+i] = a[(j*n)+i]*g;
      }
    }
    else {
      for (j = i; j < m; ++j) {
        a[(j*n)+i] = 0.0;
      }
    }
    ++a[(i*n)+i];
  }

  //diagonalize the bidiagonal form 
  for (k = n - 1; k >= 0; k--) { //loop over singular values 
    for (its = 0; its < 30; ++its) { //loop over allowed iterations 
      flag = 1;
      for (l = k; l >= 0; --l) { //test for splitting 
        nm = l - 1;
        if (fabs(rv1[l]) + anorm == anorm) {
          flag = 0;
          break;
        }
        if (fabs(w[nm]) + anorm == anorm) {
          break;
        }
      }
      if (flag) {
        c = 0.0;
        s = 1.0;
        for (i = l; i <= k; ++i) {
          f = s * rv1[i];
          if (fabs(f) + anorm != anorm) {
            g = w[i];
            h = pythag(f, g);
            w[i] = h; 
            h = 1.0 / h;
            c = g * h;
            s = (- f * h);
            for (j = 0; j < m; ++j) {
              y = a[(j*n)+nm];
              z = a[(j*n)+i];
              a[(j*n)+nm] = (y * c + z * s);
              a[(j*n)+i] = (z * c - y * s);
            }
          }
        }
      }
      z = w[k];
      if (l == k) { //convergence 
        if (z < 0.0) { //make singular value nonnegative 
          w[k] = (-z);
          for (j = 0; j < n; ++j) {
            v[(j*n)+k] = (-v[(j*n)+k]);
          }
        }
        break;
      }
      if (its >= 30) {
        free_array((void*) rv1);
        fprintf(stderr, "No convergence after 30,000! iterations \n");
        return(0);
      }
    
      //shift from bottom 2 x 2 minor 
      x = w[l];
      nm = k - 1;
      y = w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = pythag(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
      //next QR transformation 
      c = s = 1.0;
      for (j = l; j <= nm; ++j) {
        i = j + 1;
        g = rv1[i];
        y = w[i];
        h = s * g;
        g = c * g;
        z = pythag(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = x * c + g * s;
        g = g * c - x * s;
        h = y * s;
        y = y * c;
        for (jj = 0; jj < n; ++jj) {
          x = v[(jj*n)+j];
          z = v[(jj*n)+i];
          v[(jj*n)+j] = (x * c + z * s);
          v[(jj*n)+i] = (z * c - x * s);
        }
        z = pythag(f, h);
        w[j] = z;
        if (z) {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = (c * g) + (s * y);
        x = (c * y) - (s * g);
        for (jj = 0; jj < m; ++jj) {
          y = a[(jj*n)+j];
          z = a[(jj*n)+i];
          a[(jj*n)+j] = (y * c + z * s);
          a[(jj*n)+i] = (z * c - y * s);
        }
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = x;
    }
  }

  free_array((void*) rv1);
  return(1);
}

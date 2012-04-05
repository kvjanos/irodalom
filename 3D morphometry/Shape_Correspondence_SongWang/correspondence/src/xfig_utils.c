/************************************************************
 * xfig_utils.c                                             *
 *----------------------------------------------------------*
 *  Uility functions for drawing SDI data to xFigures       *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: October 26, 2006                         *
 ************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"
#include "xfig_object.h"

/***********************************************************
 * find_normalizing_factors(landmarks, number, xmin, ymin) *
 * --                                                      *
 *  determines normalizing conditions do that the curve    *
 *  coordinates fall between 0 and 1.  Also modifies the   *
 *  xmin and ymin values so that the curve is centered     *
 ***********************************************************/
void find_normalizing_factors(double *landmarks, int num_landmarks, double *bulk, double *xmin, double *ymin) {
  int i;
  double xmax, ymax;
  xmax = ymax = -9999.9999;
  *ymin = *xmin = 9999.9999;

  for (i = 0; i < num_landmarks; ++i) {
    if (landmarks[i*2] > xmax) {
      xmax = landmarks[i*2];
    }
    if (landmarks[(i*2)+1] > ymax) {
      ymax = landmarks[(i*2)+1];
    }
    if (landmarks[i*2] < *xmin) {
      *xmin = landmarks[i*2];
    }
    if (landmarks[(i*2)+1] < *ymin) {
      *ymin = landmarks[(i*2)+1];
    }
  }

  //determine which direction the curve lies 
  //need to center on axis that has the '
  //ensure it is centered by solving for min'
  //  (min[x,y] + .5(max[x,y] - min[x,y])) - min'[x,y]    1
  //  ------------------------------------------------  = -
  //               (max[y,x] - min[y,x])                  2
  
  if ((xmax - *xmin) < (ymax - *ymin)) {
    *bulk = (ymax - *ymin);
    *ymin -= *bulk * 0.05;
    *xmin = *xmin + 0.5*(xmax - *xmin) - 0.5*(ymax - *ymin);
  }
  else {
    *bulk = (xmax - *xmin);
    *xmin -= *bulk * 0.05;
    *ymin = *ymin + 0.5*(ymax - *ymin) - 0.5*(xmax - *xmin);
  }
}

/***************************************************
 * draw_xfig_shapes                                *
 * --                                              *
 *  Function for putting shape info into xfigs     *
 *   Constants and macros defined in xfig_object.h *
 ***************************************************/
void closed_draw_xfig_shapes(XFIG *xfig, char *shape_name, double bending_energy, double *landmarks, int num_landmarks, float dim_x, float dim_y, int fig) {
  int i, dir;
  double bulk, xmin, ymin;
  int *xfig_landmarks = (int *) malloc(sizeof(int) * num_landmarks * 2);
  int *xfig_control = (int *) malloc(sizeof(int) * num_landmarks);
  int box[10];
  box[0] = 0;
  box[1] = 1200 * dim_y * fig;
  box[2] = 1200 * dim_x;
  box[3] = 1200 * dim_y * fig;
  box[4] = 1200 * dim_x;
  box[5] = 1200 * dim_y * (fig + 1);
  box[6] = 0;
  box[7] = 1200 * dim_y * (fig + 1);
  box[8] = 0;
  box[9] = 1200 * dim_y * fig;
    
  find_normalizing_factors(landmarks, num_landmarks, &bulk, &xmin, &ymin);
  bulk *= 1.1;
  
  fprintf(xfig, "######################################## %s ###############################################\n", shape_name);
  fprintf(xfig, "#Bending energy: %.4lf\n", bending_energy);
  XFIG_START_COMPOUND(xfig, 0, 1200*dim_y*fig, 1200*dim_x, 1200*dim_y*(fig+1));
  XFIG_POLYLINE(xfig, i, BOX_POLYLINE, SOLID, 1, BLACK, BLACK, 1, NO_FILL, 0.00, MITER, BUTT_CAP, -1, 0, 0, 5, box, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  xfig_landmarks[0] = (1200 * dim_x) * (landmarks[0] - xmin) / bulk;
  xfig_landmarks[1] = (1200 * dim_y) * (fig +((landmarks[1] - ymin) / bulk));
  xfig_control[0] = -1;
  XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLUE, 1, TINT0, 0.00, 0.00, xfig_landmarks[0], xfig_landmarks[1], 75, 75, xfig_landmarks[0], xfig_landmarks[1], xfig_landmarks[0]+75, xfig_landmarks[1]+75);
  
  for (i = 1; i < num_landmarks; ++i) {
    xfig_landmarks[i*2] = (1200 * dim_x) * (landmarks[i*2] - xmin) / bulk;
    xfig_landmarks[(i*2)+1] = (1200 * dim_y) * (fig +((landmarks[(i*2)+1] - ymin) / bulk));
    xfig_control[i] = -1;
    XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLACK, 1, BLACK_BLACK, 0.00, 0.00, xfig_landmarks[i*2], xfig_landmarks[(i*2)+1], 50, 50, xfig_landmarks[i*2], xfig_landmarks[(i*2)+1], xfig_landmarks[i*2]+50, xfig_landmarks[(i*2)+1]+50);
  }

  XFIG_SPLINE(xfig, i, CLOSED_INTERPOLATED_SPLINE, SOLID, 1, RED, RED, 2, NO_FILL, 0.00, BUTT_CAP, 0, 0, num_landmarks, xfig_landmarks, xfig_control, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  //keeps seg faulting on text, cant figure out why
  //XFIG_TEXT(xfig, CENTER, BLACK, 1, TIMES_ROMAN, 12, 0.00, POSTSCRIPT_TEXT, 135, length, 600 * dim_x, 1200 * dim_y * (fig+1), shape_name);
  XFIG_END_COMPOUND(xfig);

  free_array((void *) xfig_landmarks);
  free_array((void *) xfig_control);
 
}

void open_draw_xfig_shapes(XFIG *xfig, char *shape_name, double bending_energy, double *landmarks, int num_landmarks, float dim_x, float dim_y, int fig) {
  int i, dir;
  double bulk, xmin, ymin;
  int *xfig_landmarks = (int *) malloc(sizeof(int) * num_landmarks * 2);
  int *xfig_control = (int *) malloc(sizeof(int) * num_landmarks);
  int box[10];
  box[0] = 0;
  box[1] = 1200 * dim_y * fig;
  box[2] = 1200 * dim_x;
  box[3] = 1200 * dim_y * fig;
  box[4] = 1200 * dim_x;
  box[5] = 1200 * dim_y * (fig + 1);
  box[6] = 0;
  box[7] = 1200 * dim_y * (fig + 1);
  box[8] = 0;
  box[9] = 1200 * dim_y * fig;
    
  find_normalizing_factors(landmarks, num_landmarks, &bulk, &xmin, &ymin);
  bulk *= 1.1;
  
  fprintf(xfig, "######################################## %s ###############################################\n", shape_name);
  fprintf(xfig, "#Bending energy: %.4lf\n", bending_energy);
  XFIG_START_COMPOUND(xfig, 0, 1200*dim_y*fig, 1200*dim_x, 1200*dim_y*(fig+1));
  XFIG_POLYLINE(xfig, i, BOX_POLYLINE, SOLID, 1, BLACK, BLACK, 1, NO_FILL, 0.00, MITER, BUTT_CAP, -1, 0, 0, 5, box, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  xfig_landmarks[0] = (1200 * dim_x) * (landmarks[0] - xmin) / bulk;
  xfig_landmarks[1] = (1200 * dim_y) * (fig +((landmarks[1] - ymin) / bulk));
  xfig_control[0] = -1;
  XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLUE, 1, TINT0, 0.00, 0.00, xfig_landmarks[0], xfig_landmarks[1], 75, 75, xfig_landmarks[0], xfig_landmarks[1], xfig_landmarks[0]+75, xfig_landmarks[1]+75);
  
  for (i = 1; i < num_landmarks; ++i) {
    xfig_landmarks[i*2] = (1200 * dim_x) * (landmarks[i*2] - xmin) / bulk;
    xfig_landmarks[(i*2)+1] = (1200 * dim_y) * (fig +((landmarks[(i*2)+1] - ymin) / bulk));
    xfig_control[i] = -1;
    XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLACK, 1, BLACK_BLACK, 0.00, 0.00, xfig_landmarks[i*2], xfig_landmarks[(i*2)+1], 50, 50, xfig_landmarks[i*2], xfig_landmarks[(i*2)+1], xfig_landmarks[i*2]+50, xfig_landmarks[(i*2)+1]+50);
  }

  XFIG_SPLINE(xfig, i, OPEN_INTERPOLATED_SPLINE, SOLID, 1, RED, RED, 2, NO_FILL, 0.00, BUTT_CAP, 0, 0, num_landmarks, xfig_landmarks, xfig_control, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  //keeps seg faulting on text, cant figure out why
  //XFIG_TEXT(xfig, CENTER, BLACK, 1, TIMES_ROMAN, 12, 0.00, POSTSCRIPT_TEXT, 135, length, 600 * dim_x, 1200 * dim_y * (fig+1), shape_name);
  XFIG_END_COMPOUND(xfig);

  free_array((void *) xfig_landmarks);
  free_array((void *) xfig_control);
 
}

/********************************************
 * draw_shape_space                         *
 * --                                       *
 *  Draws the points in the shape space and *
 *  the mean shape of the space             *
 ********************************************/
void closed_draw_shape_space(XFIG *xfig, double **shape_space, double *shape_space_mean, int num_shapes, int num_landmarks) {
  double bulk;
  int i, j, x, y;
  int *xfig_control = (int *) malloc(sizeof(int) * num_landmarks);
  int *mean_shape = (int *) malloc(sizeof(int) * num_landmarks * 2);
  double xmin = 9999.9999, xmax = -9999.9999, ymin = 9999.9999, ymax = -9999.9999;
  
  for (i = 0; i < num_shapes; ++i) {
    for (j = 0; j < num_landmarks; ++j) {
      if (shape_space[i][j*2] < xmin) {
        xmin = shape_space[i][j*2];
      }
      if (shape_space[i][j*2] > xmax) {
        xmax = shape_space[i][j*2];
      }
      if (shape_space[i][(j*2)+1] < ymin) {
        ymin = shape_space[i][(j*2)+1];
      }
      if (shape_space[i][(j*2)+1] > ymax) {
        ymax = shape_space[i][(j*2)+1];
      }
    }
  }
  
  if ((ymax - ymin) < (xmax - xmin)) {
    bulk = xmax - xmin;
    ymin = ymin + 0.5*(ymax - ymin) - 0.5*bulk;
  }
  else {
    bulk = ymax - ymin;
    xmin = xmin + 0.5*(xmax - xmin) + 0.5*bulk;
  }
  bulk *= 1.1;
  
  XFIG_START_COMPOUND(xfig, 0, 0, 12000, 12000);
  for (i = 0; i < num_shapes; ++i) {
      x = (int) fabs(12000 * (shape_space[i][0] - xmin) / bulk);
      y = (int) fabs(12000 * (shape_space[i][1] - ymin) / bulk);
      XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLUE, 1, FULL, 0.00, 0.00, x, y, 15, 15, x, y, x+25, y+25);
    for (j = 1; j < num_landmarks; ++j) {
      x = (int) fabs(12000 * (shape_space[i][j*2] - xmin) / bulk);
      y = (int) fabs(12000 * (shape_space[i][(j*2)+1] - ymin) / bulk);
      XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLACK, 1, BLACK_BLACK, 0.00, 0.00, x, y, 15, 15, x, y, x+25, y+25);
    }
  }
  for (i = 0; i < num_landmarks; ++i) {
    mean_shape[i*2] = (int) fabs(12000 * (shape_space_mean[i*2] - xmin) / bulk);
    mean_shape[(i*2)+1] = (int) fabs(12000 * (shape_space_mean[(i*2)+1] - ymin) / bulk);
    xfig_control[i] = -1;
  }
  XFIG_SPLINE(xfig, i, CLOSED_INTERPOLATED_SPLINE, SOLID, 1, RED, RED, 0, NO_FILL, 0.00, BUTT_CAP, 0, 0, num_landmarks, mean_shape, xfig_control, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  XFIG_END_COMPOUND(xfig);

  free_array((void *)mean_shape);
  free_array((void *)xfig_control);
}

void open_draw_shape_space(XFIG *xfig, double **shape_space, double *shape_space_mean, int num_shapes, int num_landmarks) {
  double bulk;
  int i, j, x, y;
  int *xfig_control = (int *) malloc(sizeof(int) * num_landmarks);
  int *mean_shape = (int *) malloc(sizeof(int) * num_landmarks * 2);
  double xmin = 9999.9999, xmax = -9999.9999, ymin = 9999.9999, ymax = -9999.9999;
  
  for (i = 0; i < num_shapes; ++i) {
    for (j = 0; j < num_landmarks; ++j) {
      if (shape_space[i][j*2] < xmin) {
        xmin = shape_space[i][j*2];
      }
      if (shape_space[i][j*2] > xmax) {
        xmax = shape_space[i][j*2];
      }
      if (shape_space[i][(j*2)+1] < ymin) {
        ymin = shape_space[i][(j*2)+1];
      }
      if (shape_space[i][(j*2)+1] > ymax) {
        ymax = shape_space[i][(j*2)+1];
      }
    }
  }
  
  if ((ymax - ymin) < (xmax - xmin)) {
    bulk = xmax - xmin;
    ymin = ymin + 0.5*(ymax - ymin) - 0.5*bulk;
  }
  else {
    bulk = ymax - ymin;
    xmin = xmin + 0.5*(xmax - xmin) + 0.5*bulk;
  }
  bulk *= 1.1;
  
  XFIG_START_COMPOUND(xfig, 0, 0, 12000, 12000);
  for (i = 0; i < num_shapes; ++i) {
      x = (int) fabs(12000 * (shape_space[i][0] - xmin) / bulk);
      y = (int) fabs(12000 * (shape_space[i][1] - ymin) / bulk);
      XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLUE, 1, FULL, 0.00, 0.00, x, y, 15, 15, x, y, x+25, y+25);
    for (j = 1; j < num_landmarks; ++j) {
      x = (int) fabs(12000 * (shape_space[i][j*2] - xmin) / bulk);
      y = (int) fabs(12000 * (shape_space[i][(j*2)+1] - ymin) / bulk);
      XFIG_ELLIPSE(xfig, CIRCLE_RADIUS, SOLID, 1, BLACK, BLACK, 1, BLACK_BLACK, 0.00, 0.00, x, y, 15, 15, x, y, x+25, y+25);
    }
  }
  for (i = 0; i < num_landmarks; ++i) {
    mean_shape[i*2] = (int) fabs(12000 * (shape_space_mean[i*2] - xmin) / bulk);
    mean_shape[(i*2)+1] = (int) fabs(12000 * (shape_space_mean[(i*2)+1] - ymin) / bulk);
    xfig_control[i] = -1;
  }
  XFIG_SPLINE(xfig, i, OPEN_INTERPOLATED_SPLINE, SOLID, 1, RED, RED, 0, NO_FILL, 0.00, BUTT_CAP, 0, 0, num_landmarks, mean_shape, xfig_control, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  XFIG_END_COMPOUND(xfig);

  free_array((void *)mean_shape);
  free_array((void *)xfig_control);
}

/**********************************************************
 * create_xfig                                            *
 * --                                                     *
 *  Creates a new xfigure file, with the given attributes *
 *  returns a FILe pointer to the file                    *
 **********************************************************/
XFIG *create_xfig(char *filename, char *orientation, char *justification, char *units, char *papersize, float magnification, char *multiple_page, int transparent_color) {
  XFIG *xfig;

  if ((xfig = fopen(filename, "w")) == NULL) {
    return NULL;
  }

  XFIG_COMMENT(xfig, "FIG 3.2");
  fprintf(xfig, "%s\n%s\n%s\n%s\n%lf\n%s\n%d\n1200 2\n", orientation, justification, units, papersize, magnification, multiple_page, transparent_color);

  return xfig;
}

void close_xfig(XFIG *xfig) {
  if (xfig != NULL) {
    fclose(xfig);
  }
}


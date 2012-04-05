#include <stdio.h>
#include <stdlib.h>
#include "cdf.h"
#include "shape.h"

int main(int argc, char*argv[]) {
  FILE *fp;
  double * landmarks;
  char *filename, *cdf_filename;
  char *shape_name = (char *) malloc(sizeof(256));
  int i, j, k, dim_sizes[2], indices[2];
  double val;
  
  CDFstatus status;
  CDFid cdf;
  long zvar_num, majority;
  static long REC_VARY = {VARY};
  static long DIM_VARY[2] = {VARY, VARY};

  if (argc < 3) {
    fprintf(stderr, "Invalid use of a2cdf.\na2cdf input_file output_file.\n");
    exit(0);
  }
  filename = argv[1];
  cdf_filename = argv[2];
  
  if((fp = fopen(filename, "r")) == NULL)
    fprintf(stderr, "error opening %s for reading...\n", filename);

  if ((status = CDFcreateCDF(cdf_filename, &cdf)) != CDF_OK) {
    fprintf(stderr, "Could not create %s for output.\n", cdf_filename);
    exit(0);
  }
  CDFgetMajority(cdf, &majority);

  for (i = 0; !feof(fp); ++i) {
    fscanf(fp, "%s %d %d", shape_name, dim_sizes, (dim_sizes + 1));
    landmarks = (double *) malloc(sizeof(double) * dim_sizes[0] * 2);

    for (j = 0; j < dim_sizes[0]; ++j) {
      indices[0] = j;
      for (k = 0; k < dim_sizes[1]; ++k) {
        indices[1] = k;
        fscanf(fp, "%lp", &val);
        CDFputzVarData(cdf, i, 0, indices, &val);
      }
    }
   
    /*
    for (i = 0; i < dim_sizes[0]; i++) {
      fscanf(fp, "%lf %lf", (landmarks+(i*2)), (landmarks+(i*2)+1));
    }
    fscanf(fp, " ");
    
    //write to cdf
    if ((status = CDFcreatezVar(cdf, shape_name, CDF_DOUBLE, 1, 2, dim_sizes, REC_VARY, DIM_VARY, &zvar_num)) != CDF_OK) {
      fprintf(stderr, "Could not create a variable for %s in the output file. %s will not be included in the output.\n", shape_name, shape_name);
    }
    else {
      //write corresponded shape to output *.cdf
      store_data_cdf(cdf, landmarks, zvar_num, majority, dim_sizes[0], 2);
    }
    free((void *) landmarks);
    */
  }
  CDFclose(cdf);

  return 0;
}

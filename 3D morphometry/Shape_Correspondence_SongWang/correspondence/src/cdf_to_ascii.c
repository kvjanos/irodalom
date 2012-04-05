#include <stdio.h>
#include <stdlib.h>
#include "cdf.h"
#include "shape.h"

int main(int argc, char*argv[]) {
  FILE *fp;
  char *filename, *cdf_filename;
  char *shape_name = (char *) malloc(sizeof(256));
  int dim_sizes[2], indices[2], i, j, k, num_vars;
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
  cdf_filename = argv[1];
  filename = argv[2];
  
  if((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error opening %s for reading...\n", filename);
    exit(0);
  }

  if ((status = CDFopenCDF(cdf_filename, &cdf)) != CDF_OK) {
    fprintf(stderr, "Could not open %s for output.\n", cdf_filename);
    exit(0);
  }
  CDFgetMajority(cdf, &majority);

  CDFgetNumzVars(cdf, &num_vars);
  for (i = 0; i < num_vars; ++i) {
    CDFgetzVarDimSizes(cdf, i, dim_sizes);
    CDFgetzVarName(cdf, i, shape_name);

    fprintf(fp, "%s %d %d\n", shape_name, dim_sizes[0], dim_sizes[1]);
    for (j = 0; j < dim_sizes[0]; ++j) {
      indices[0] = j;
      for (k = 0; k < dim_sizes[1]; ++k) {
        indices[1] = k;
        CDFgetzVarData(cdf, i, 0, indices, &val);
        fprintf(fp, "%le ", val);
      }
      fprintf(fp, "\n");
    }
  }
  CDFclose(cdf);
  fclose(fp);
}


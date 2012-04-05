/******************************************************************************
*
*  NSSDC/CDF                  Quick Start Test Program (INTERNAL interface/C).
*
*  Version 1.11, 13-Jun-96, Hughes STX.
*
******************************************************************************/

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
* Main.
******************************************************************************/

int main () {
  int imax;
  int ino;
  char buf[81];
  int i123 = -12345678;
  float ifl = -123.45678;
  float a1,a3;
  double a2,a4;
  strcpy(buf, "%8.2f");
  ino = sscanf(buf+1, "%d", &imax);
  printf("8.2: max=%d\n",imax);
  strcpy(buf, "%3d");
  ino = sscanf(buf+1, "%d", &imax);
  printf("3: max=%d\n",imax);
  printf("12345678: %6d %1d %d\n",i123,i123,i123);
  printf("-123.45678: %3.3e %6.3e\n",ifl,ifl);
  sscanf("23.12345678", "%f", &a1);
  sscanf("23.12345678", "%lf", &a2);
  printf("a1(f)=%f (g)=%g (lf)=%lf (lg)=%lg (2.8f)=%2.8f\n",a1,a1,a1,a1,a1);
  printf("a2(f)=%f (g)=%g (lf)=%lf (lg)=%lg (2.8f)=%2.8f\n",a2,a2,a2, a2,a2);
  sscanf("23.12345678", "%g", &a3);
  sscanf("23.12345678", "%lg", &a4);
  printf("a3(f)=%f (g)=%g (lf)=%lf (lg)=%lg (2.8f)=%2.8f\n",a3,a3,a3,a3,a3);
  printf("a4(f)=%f (g)=%g (lf)=%lf (lg)=%lg (2.8f)=%2.8f\n",a4,a4,a4, a4,a4);

  exit (0);
}

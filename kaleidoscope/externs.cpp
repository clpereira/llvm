#include <stdio.h>

extern "C" double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

extern "C" double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

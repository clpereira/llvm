#ifndef _KCOMP_H_
#define _KCOMP_H_

#include "k_llvm.h"
#include "kcomp_config.h"
#include "codegen.h"
#include "parser.h"

class KCompiler {
public:
  static void initialize_and_run();
};

#endif

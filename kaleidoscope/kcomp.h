#ifndef _KCOMP_H_
#define _KCOMP_H_

#include "k_llvm.h"
#include "kcomp_config.h"
#include "codegen.h"
#include "parser.h"

class KCompiler {
private:
  static std::unique_ptr<llvm::orc::KaleidoscopeJIT> jit;

public:
  static void initialize_and_run();
};

#endif

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "k_llvm.h"
#include <map>
#include <string>

class Codegen {
 public:
  static llvm::LLVMContext the_context;
  static llvm::IRBuilder<> builder;
  static std::unique_ptr<llvm::Module> the_module;
  static std::map<std::string, llvm::Value *> named_values;
  static std::unique_ptr<llvm::legacy::FunctionPassManager>fpm;
  static std::unique_ptr<llvm::orc::KaleidoscopeJIT> jit;

  static void initializeModuleAndPassManager();
};

#endif

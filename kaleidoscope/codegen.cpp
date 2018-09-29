#include "codegen.h"

llvm::LLVMContext the_context;
llvm::IRBuilder<> builder(the_context);
std::unique_ptr<llvm::Module> the_module;
std::map<std::string, llvm::Value *> named_values;

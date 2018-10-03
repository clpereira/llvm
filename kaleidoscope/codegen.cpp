#include "codegen.h"

llvm::LLVMContext Codegen::the_context;
llvm::IRBuilder<> Codegen::builder(the_context);
llvm::Module * Codegen::the_module;
std::map<std::string, llvm::Value *> Codegen::named_values;

#include "codegen.h"

llvm::LLVMContext Codegen::the_context;
llvm::IRBuilder<> Codegen::builder(the_context);
std::unique_ptr<llvm::Module> Codegen::the_module;
std::map<std::string, llvm::Value *> Codegen::named_values;
std::unique_ptr<llvm::legacy::FunctionPassManager> Codegen::fpm;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> Codegen::jit;

void Codegen::initializeModuleAndPassManager()
{
  // open a new module
  the_module = llvm::make_unique<llvm::Module>("my cool jit", the_context);
  the_module->setDataLayout(jit->getTargetMachine().createDataLayout());
  fpm = llvm::make_unique<llvm::legacy::FunctionPassManager>(the_module.get());
  
  fpm->add(llvm::createInstructionCombiningPass());
  fpm->add(llvm::createReassociatePass());
  fpm->add(llvm::createGVNPass());
  fpm->add(llvm::createCFGSimplificationPass());
  
  fpm->doInitialization();
}

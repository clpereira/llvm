#include "kcomp.h"

std::unique_ptr<llvm::orc::KaleidoscopeJIT> KCompiler::jit;

void KCompiler::initialize_and_run()
{
  std::cout << "Kaleidoscope compiler version: " 
			<< kcomp_VERSION_MAJOR << "." 
			<< kcomp_VERSION_MINOR << std::endl;

  Codegen::initializeModuleAndPassManager();
  Parser::parse();

  Codegen::the_module->print(llvm::errs(), nullptr);
}

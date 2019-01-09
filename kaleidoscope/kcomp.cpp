#include "kcomp.h"

void KCompiler::initialize_and_run()
{
  std::cout << "Kaleidoscope compiler version: " 
			<< kcomp_VERSION_MAJOR << "." 
			<< kcomp_VERSION_MINOR << std::endl;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  std::cerr << "ready> ";
  Parser::getNextToken();

  Codegen::jit = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
  Codegen::initializeModuleAndPassManager();
  Parser::parse();
}

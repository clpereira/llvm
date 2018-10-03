#include <fstream>
#include <iostream>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "kcomp_config.h"
#include "codegen.h"

int main()
{
  std::cout << "Kaleidoscope compiler version: " 
			<< kcomp_VERSION_MAJOR << "." 
			<< kcomp_VERSION_MINOR << std::endl;

  Codegen::the_module = llvm::make_unique<llvm::Module>("my cool jit", 
														Codegen::the_context);
  Parser::parse();

  Codegen::the_module->print(llvm::errs(), nullptr);

  return 0;
}

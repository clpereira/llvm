#include <fstream>
#include <iostream>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "kcomp_config.h"

int main()
{
  std::cout << "Kaleidoscope compiler version: " << kcomp_VERSION_MAJOR << "." 
                                                 << kcomp_VERSION_MINOR << std::endl;
#if 0
  Lexer lexer;
  while (lexer.getToken() != tok_eof)
  {
    lexer.printToken();
  }
#else
  Parser::parse();
#endif
  return 0;
}

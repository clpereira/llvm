#include <iostream>

#include "lexer.h"
#include "ast.h"
#include "kcomp_config.h"

int main()
{
  Lexer lexer;

  std::cout << "Kaleidoscope compiler version: " << kcomp_VERSION_MAJOR << "." 
                                                 << kcomp_VERSION_MINOR << std::endl;
  while (lexer.getToken() != tok_eof)
  {
    lexer.printToken();
  }
  return 0;
}

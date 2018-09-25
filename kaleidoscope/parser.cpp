#include "parser.h"
#include <iostream>

// Parser for Kaleidoscope
int Parser::cur_tok;
Lexer Parser::lexer;
  
std::unique_ptr<ExprAST> Parser::logError(const char * str)
{
  std::cerr << "LogError: " << str << std::endl;
  return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::logErrorP(const char * str)
{
  logError(str);
  return nullptr;
}

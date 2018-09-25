#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>

#include "lexer.h"
#include "ast.h"

// Parser for Kaleidoscope
class Parser {
private:
  static int cur_tok;
  static Lexer lexer;
  
 public:
  static int getNextToken() { return lexer.getToken(); }

  static std::unique_ptr<ExprAST> parseNumberExpr();

  // error handling
  static std::unique_ptr<ExprAST> logError(const char * str);
  static std::unique_ptr<PrototypeAST> logErrorP(const char * str);
};

#endif

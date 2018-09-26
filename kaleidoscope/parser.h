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
  
 private:
  static int getNextToken() { return lexer.getToken(); }

  // numberexpr ::= number
  static std::unique_ptr<ExprAST> parseNumberExpr();
  // parenexpr ::= '(' expression ')'
  static std::unique_ptr<ExprAST> parseParenExpr();
  // identifierexpr
  //   ::= identifier
  //   ::= identifier '(' expression * ')'
  static std::unique_ptr<ExprAST> parseIdentifierExpr();
  // expression 
  //   ::= binary binoprhs
  static std::unique_ptr<ExprAST> parseExpression();
  // primary
  //   ::= identifierexpr
  //   ::= numberexpr
  //   ::= parenexpr
  static std::unique_ptr<ExprAST> parsePrimary();

  // error handling
  static std::unique_ptr<ExprAST> logError(const char * str);
  static std::unique_ptr<PrototypeAST> logErrorP(const char * str);
};

#endif

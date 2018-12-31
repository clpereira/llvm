#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include <map>

#include "lexer.h"
#include "ast.h"

typedef std::map<char, int> binop_precedence_t;

class BinopPrecedenceConstructor
{
public:
  BinopPrecedenceConstructor();
};

// Parser for Kaleidoscope
class Parser {
private:
  friend class BinopPrecedenceConstructor; // needs to acess the precedence table

  static int cur_tok;
  static binop_precedence_t binop_precedence; // bin op precendence table
  static BinopPrecedenceConstructor binop_precedence_constructor;
  
private:
  // numberexpr ::= number
  static std::unique_ptr<ExprAST> parseNumberExpr();
  // ifexpr ::= 'if' expression 'then' expression 'else' expression
  static std::unique_ptr<ExprAST> parseIfExpr();
 
  // parenexpr ::= '(' expression ')'
  static std::unique_ptr<ExprAST> parseParenExpr();
  // identifierexpr
  //   ::= identifier
  //   ::= identifier '(' expression * ')'
  static std::unique_ptr<ExprAST> parseIdentifierExpr();
  // expression 
  //   ::= binary binoprhs
  static std::unique_ptr<ExprAST> parseExpression();
  // binoprhs
  //   ::= (['+'|'-'|'<'|'*']primary)*
  static std::unique_ptr<ExprAST> parseBinOpRHS(int expr_prec, 
                                                std::unique_ptr<ExprAST> lhs);
  // primary
  //   ::= identifierexpr
  //   ::= numberexpr
  //   ::= parenexpr
  static std::unique_ptr<ExprAST> parsePrimary();

  // prototype
  //   ::= id '(' id* ')'
  static std::unique_ptr<PrototypeAST> parsePrototype();

  // definition
  //   ::= 'def' prototype expression
  static std::unique_ptr<FunctionAST> parseDefinition();

  // external 
  //   ::= 'extern' prototype
  static std::unique_ptr<PrototypeAST> parseExtern();

  // toplevelexpr
  //   ::= expression
  static std::unique_ptr<FunctionAST> parseTopLevelExpr();

  static void handleDefinition();
  static void handleExtern();
  static void handleTopLevelExpression();

  // get the precedence given a binary operator
  static int getTokPrecedence();

  static void mainloop();

 public:
  static int getNextToken();
  static void parse();
};

#endif

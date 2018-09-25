#include "llvm/ADT/STLExtras.h"
#include "parser.h"
#include <iostream>

// Parser for Kaleidoscope
int Parser::cur_tok;
Lexer Parser::lexer;
  
std::unique_ptr<ExprAST> Parser::parseNumberExpr()
{
  auto result = llvm::make_unique<NumberExprAST>(lexer.numVal);
  getNextToken(); // consume the number
  return std::move(result);
}

std::unique_ptr<ExprAST> Parser::parseParenExpr()
{
  getNextToken(); // consume the '('
  auto V = parseExpression();
  if (!V)
  {
	return nullptr;
  }
  if (cur_tok != ')')
  {
	return logError("expected ')'");
  }
  getNextToken(); // eat next ')'
  return V;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr()
{
  std::string id_name = Lexer::identifierStr;

  getNextToken(); // consume the identifier
  
  if (cur_tok != '(') // simple variable reference (not a function call)
  {
	return llvm::make_unique<VariableExprAST>(id_name);
  }
  // call
  getNextToken(); // eat the '('
  expr_ast_vector_t args;
  if (cur_tok != ')') 
  {
	while (1) 
	{
	  if (auto arg = parseExpression())
	  {
		args.push_back(std::move(arg));
	  }
	  else
	  {
		return nullptr;
	  }
	  if (cur_tok == ')')
	  {
		break;
	  }
	  if (cur_tok != ',')
	  {
		return logError("Expected ')' or ',' in argument list");
	  }
	  getNextToken();
	}
  }
  // consume next ')'
  getNextToken();
  return llvm::make_unique<CallExprAST>(id_name, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parseExpression()
{
  return nullptr;
}

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

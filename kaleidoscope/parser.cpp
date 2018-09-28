#include "llvm/ADT/STLExtras.h"
#include "parser.h"
#include <iostream>
#include <cctype>

// Parser for Kaleidoscope
int Parser::cur_tok;
Lexer Parser::lexer;
binop_precedence_t Parser::binop_precedence;
BinopPrecedenceConstructor Parser::binop_precedence_constructor;

// Fills in the precendence table
BinopPrecedenceConstructor::BinopPrecedenceConstructor()
{
  // list of binary operators and their precendence
  Parser::binop_precedence['<'] = 10;
  Parser::binop_precedence['+'] = 20;
  Parser::binop_precedence['-'] = 30;
  Parser::binop_precedence['*'] = 40;  // highest priority
}
  
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
  auto lhs = parsePrimary();
  if (!lhs)
  {
	return nullptr;
  }
  return parseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int expr_prec,
											   std::unique_ptr<ExprAST> lhs)
{
  // keep parsing until we run out of bin ops or until the precendence of the 
  // next bin op is too low
  while (1)
  {
	int tok_prec = getTokPrecedence();

	// precedence is lower than the left side, return the left hand side
	if (tok_prec < expr_prec)
	{
	  return lhs;
	}
	int bin_op = cur_tok;
	getNextToken(); // eat the binary operator

	// parse the next expression, to the right of the bin op
	auto rhs = parsePrimary();
	if (!rhs)
	{
	  return nullptr;
	}

	int next_prec = getTokPrecedence();
	if (tok_prec < next_prec)
	{
	  rhs = parseBinOpRHS(tok_prec+1, std::move(rhs));
	  if (!rhs)
	  {
		return nullptr;
	  }
	}
	// put together the right and left hand sides
	lhs = llvm::make_unique<BinaryExprAST>(bin_op, 
										   std::move(lhs), std::move(rhs));
  }
}

std::unique_ptr<ExprAST> Parser::parsePrimary()
{
  switch (cur_tok)
  {
  default: 
  {
    return logError("Unknown token when expecting an expression");
  }
  case tok_identifier:
  {
    return parseIdentifierExpr();
  }
  case tok_number:
  {
    return parseNumberExpr(); 
  }
  case '(':
  {
    return parseParenExpr();
  }
  }
}

std::unique_ptr<ExprAST> Parser::parsePrototype()
{
  return nullptr;
}

std::unique_ptr<FunctionAST> Parser::parseDefinition()
{
  return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExtern()
{
  return nullptr;
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr()
{
  return nullptr;
}

void Parser::handleDefinition()
{
}

void Parser::handleExtern()
{
}

void Parser::handleTopLevelExpression()
{
}

// get the precedence given a binary operator
int Parser::getTokPrecedence()
{
  int tok_prec;
  // invalid token
  if (!isascii(cur_tok))
  {
    return -1;
  }
  // try finding the operator in the table
  tok_prec = binop_precedence[cur_tok];
  // not found
  if (tok_prec <= 0)
  {
    return -1;
  }
  return tok_prec;
}

void Parser::mainloop()
{
  while (1)
  {
	switch (cur_tok)
	{
	case tok_eof:
	{
	  return;
	}
	case ';':
	{
	  getNextToken();
	  break;
	}
	case tok_def:
	{
	  break;
	}
	case tok_extern:
	{
	  break;
	}
	default:
	{
	  break;
	}
	}
  }
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

void Parser::parse()
{
  getNextToken();

  mainloop();
}

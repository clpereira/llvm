#include "llvm/ADT/STLExtras.h"
#include "parser.h"
#include <iostream>
#include <cctype>
#include <string>

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

int Parser::getNextToken() 
{ 
  cur_tok = lexer.getToken(); 
  //lexer.printToken();
  if (cur_tok == tok_special_char)
  {
	cur_tok = lexer.getLastSpecialChar();
  }
  return cur_tok;
}
  
std::unique_ptr<ExprAST> Parser::parseNumberExpr()
{
  auto result = llvm::make_unique<NumberExprAST>(Lexer::numVal);
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
  default: 
  {
    return logError("Unknown token when expecting an expression");
  }
  }
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype()
{
  if (cur_tok != tok_identifier)
  {
	return logErrorP("Expected function name in prototype");
  }
  std::string fn_name = Lexer::identifierStr;
  getNextToken();

  if (cur_tok != '(')
  {
	return logErrorP("Expected '(' ("+
					 std::to_string(static_cast<int>('('))+
					 ") in prototype, but found: '"+
					 std::string(1, static_cast<char>(cur_tok))+
					 "' ("+std::to_string(cur_tok)+")");
  }

  string_vector_t arg_names;
  while (getNextToken() == tok_identifier)
  {
	arg_names.push_back(Lexer::identifierStr);
  }
  if (cur_tok != ')')
  {
	return logErrorP("Expected ')') in prototype");
  }
  getNextToken(); // eat ')'
  
  return llvm::make_unique<PrototypeAST>(fn_name, std::move(arg_names));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition()
{
  getNextToken(); // eat 'def'
  auto proto = parsePrototype();
  if (!proto)
  {
	return nullptr;
  }
  // the body
  if (auto e = parseExpression())
  {
	return llvm::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExtern()
{
  getNextToken(); // eat 'extern'
  return parsePrototype();
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr()
{
  if (auto e = parseExpression())
  {
	// make an anonymous prototype
	auto proto = llvm::make_unique<PrototypeAST>("__anon_expr",
												 std::vector<std::string>());
	return llvm::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}

void Parser::handleDefinition()
{
  if (parseDefinition())
  {
	std::cout << "Parsed a function definition" << std::endl;
  }
  else
  {
	getNextToken();
  }
}

void Parser::handleExtern()
{
  if (parseExpression())
  {
	std::cout << "Parsed an extern" << std::endl;
  }
  else
  {
	getNextToken();
  }
}

void Parser::handleTopLevelExpression()
{
  // evaluate the top-level expression into an anonymous functions
  if (parseTopLevelExpr())
  {
	std::cout << "Parsed a top-level expr" << std::endl;
  }
  else
  {
	getNextToken();
  }
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
	  handleDefinition();
	  break;
	}
	case tok_extern:
	{
	  handleExtern();
	  break;
	}
	default:
	{
	  handleTopLevelExpression();
	  break;
	}
	}
  }
}

std::unique_ptr<ExprAST> Parser::logError(const std::string str)
{
  std::cerr << "LogError: " << str << std::endl;
  return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::logErrorP(const std::string str)
{
  logError(str);
  return nullptr;
}

void Parser::parse()
{
  getNextToken();
  mainloop();
}

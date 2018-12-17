#include "parser.h"
#include "codegen.h"
#include "error.h"
#include <iostream>
#include <cctype>
#include <string>

// Parser for Kaleidoscope
int Parser::cur_tok;
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
  cur_tok = Lexer::instance()->getToken(); 
  //Lexer::instance()->printToken();
  if (cur_tok == tok_special_char)
  {
	cur_tok = Lexer::instance()->getLastSpecialChar();
  }
  return cur_tok;
}
  
std::unique_ptr<ExprAST> Parser::parseNumberExpr()
{
  auto result = llvm::make_unique<NumberExprAST>(Lexer::instance()->numVal);
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
	return Error::log("expected ')'");
  }
  getNextToken(); // eat next ')'
  return V;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr()
{
  std::string id_name = Lexer::instance()->identifierStr;

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
		return Error::log("Expected ')' or ',' in argument list");
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
    return Error::log("Unknown token when expecting an expression");
  }
  }
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype()
{
  if (cur_tok != tok_identifier)
  {
	return Error::logP("Expected function name in prototype");
  }
  std::string fn_name = Lexer::instance()->identifierStr;
  getNextToken();

  if (cur_tok != '(')
  {
	return Error::logP("Expected '(' ("+
					 std::to_string(static_cast<int>('('))+
					 ") in prototype, but found: '"+
					 std::string(1, static_cast<char>(cur_tok))+
					 "' ("+std::to_string(cur_tok)+")");
  }

  string_vector_t arg_names;
  while (getNextToken() == tok_identifier)
  {
	arg_names.push_back(Lexer::instance()->identifierStr);
  }
  if (cur_tok != ')')
  {
	return Error::logP("Expected ')') in prototype");
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
  if (auto fn_ast = parseDefinition())
  {
	if (auto * fn_ir = fn_ast->codegen())
	{
	  std::cout << "Parsed a function definition:" << std::endl;
	  fn_ir->print(llvm::errs());
	  std::cout << std::endl;
	}
  }
  else
  {
	getNextToken();
  }
}

void Parser::handleExtern()
{
  if (auto proto_ast = parseExtern())
  {
	if (auto * fn_ir = proto_ast->codegen())
	{
	  std::cout << "Parsed an extern:" << std::endl;
	  fn_ir->print(llvm::errs());
	  std::cout << std::endl;
	}
  }
  else
  {
	getNextToken();
  }
}

void Parser::handleTopLevelExpression()
{
  // evaluate the top-level expression into an anonymous functions
  if (auto fn_ast = parseTopLevelExpr())
  {
    if (fn_ast->codegen())
    {
      auto h = Codegen::jit->addModule(std::move(Codegen::the_module));
      Codegen::initializeModuleAndPassManager();

      // Search the JIT for the __anon_expr symbol
      auto ExprSymbol = Codegen::jit->findSymbol("__anon_expr");
      assert(ExprSymbol && "Function not found");

      // get the symbols' address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function
      double (*fp)() = (double(*)())(intptr_t)cantFail(ExprSymbol.getAddress());
      std::cerr << "Function address: " << fp << std::endl;
      std::cerr << "Evaluated to " << fp() << std::endl;

      // Delete the anonymous expression module from the JIT
      Codegen::jit->removeModule(h);
    }
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

void Parser::parse()
{
  getNextToken();
  mainloop();
}

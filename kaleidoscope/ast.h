#ifndef _AST_H_
#define _AST_H_

#include <memory>
#include <string>
#include <vector>

// ExprAST Base class for all expression nodes of the tree
class ExprAST {
 public:
  virtual ~ExprAST() {}
};

// Number ExprAST - Expression class for numeric literals
class NumberExprAST : public ExprAST {
 private:
  double val;	

 public:
  NumberExprAST(double val) : val(val) {}
};

// VariableExprAST - Expression class for variables 
class VariableExprAST: public ExprAST {
private:
  std::string name;

public:
  VariableExprAST(const std::string &name) : name(name) {}
};

// BinaryExprAST - Expression class for a binary operator
class BinaryExprAST : public ExprAST {
private:
  char op;
  std::unique_ptr<ExprAST> lhs, rhs;

public:
  BinaryExprAST(char op, 
		std::unique_ptr<ExprAST> lhs,
		std::unique_ptr<ExprAST> rhs) 
	:
	op(op),
	lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

typedef std::vector<std::unique_ptr<ExprAST>> expr_ast_vector_t;

// CallExprAST - This class represents a function call
class CallExprAST : public ExprAST {
private:
  std::string callee;
  expr_ast_vector_t args;

public:
  CallExprAST(const std::string &callee,
              expr_ast_vector_t args)
	:
	callee(callee),
	args(std::move(args)) {}
};


typedef std::vector<std::string> string_vector_t;

// PrototypeAST - This class represents the prototype of a function,
// which captures its name, and its argument names
class PrototypeAST {
private:
  std::string name;
  string_vector_t args;

public:
  PrototypeAST(const std::string &name, string_vector_t args)
	:
	name(name), args(std::move(args)) {}

	const std::string &getname() const { return name; }
};

// FunctionAST - This calss represents a function definition itself
class FunctionAST {
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
			  std::unique_ptr<ExprAST> body)
	:
	proto(std::move(proto)), body(std::move(body)) {}
};

#endif

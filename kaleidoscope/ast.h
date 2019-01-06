#ifndef _AST_H_
#define _AST_H_

#include <memory>
#include <string>
#include <vector>

#include "k_llvm.h"

// ExprAST Base class for all expression nodes of the tree
class ExprAST {
 public:
  virtual ~ExprAST() {}
  virtual llvm::Value * codegen() = 0;
};

// Number ExprAST - Expression class for numeric literals
class NumberExprAST : public ExprAST {
 private:
  double val;	

 public:
  NumberExprAST(double val) : val(val) {}
  double getVal() const { return val; }
  llvm::Value * codegen() override;
};

class IfExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Cond, Then, Else;

  public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, 
              std::unique_ptr<ExprAST> Then, 
              std::unique_ptr<ExprAST> Else)
      :
      Cond(std::move(Cond)),
      Then(std::move(Then)),
      Else(std::move(Else))
    {}
    llvm::Value *codegen() override;
};

class ForExprAST : public ExprAST {
  std::string var_name;
  std::unique_ptr<ExprAST> start, end, step, body;

 public:
  ForExprAST(const std::string &var_name, std::unique_ptr<ExprAST> start,
	     std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step,
	     std::unique_ptr<ExprAST> body)
    :
  var_name(var_name), start(std::move(start)), end(std::move(end)),
    step(std::move(step)), body(std::move(body)) {}

  llvm::Value * codegen() override;
};

// VariableExprAST - Expression class for variables 
class VariableExprAST: public ExprAST {
private:
  std::string name;

public:
  VariableExprAST(const std::string &name) : name(name) {}
  llvm::Value * codegen() override;
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

	llvm::Value * codegen() override;
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

	llvm::Value * codegen() override;
};


typedef std::vector<std::string> string_vector_t;

// PrototypeAST - This class represents the prototype of a function,
// which captures its name, and its argument names
class PrototypeAST { 
private:
  std::string name;
  string_vector_t args;

public:
  static std::map<std::string, std::unique_ptr<PrototypeAST>> function_protos;

public:
  PrototypeAST(const std::string &name, string_vector_t args)
	:
	name(name), args(std::move(args)) {}

	const std::string &getname() const { return name; }
	llvm::Function * codegen();

  static llvm::Function * getFunction(std::string name);
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

	llvm::Function * codegen();
};

#endif

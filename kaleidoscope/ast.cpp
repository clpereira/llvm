#include "ast.h"
#include "codegen.h"
#include "error.h"

llvm::Value * NumberExprAST::codegen()
{
  return llvm::ConstantFP::get(Codegen::the_context, llvm::APFloat(val));
}

llvm::Value * VariableExprAST::codegen()
{
  // look this variable up in the function
  llvm::Value * v = Codegen::named_values[name];
  if (!v)
  {
	Error::logV("Unknown variable name");
  }
  return v;
}

llvm::Value * BinaryExprAST::codegen()
{
  return nullptr;
}

llvm::Value * CallExprAST::codegen()
{
  return nullptr;
}

llvm::Function * PrototypeAST::codegen()
{
  return nullptr;
}

llvm::Function * FunctionAST::codegen()
{
  return nullptr;
}


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
  llvm::Value * l = lhs->codegen();
  llvm::Value * r = rhs->codegen();
  if (!l || !r)
  {
	return nullptr;	
  }
  
  switch(op)
  {
  case '+':
  {
	return Codegen::builder.CreateFAdd(l, r, "addtmp");
  }
  case '-':
  {
	return Codegen::builder.CreateFSub(l, r, "subtmp");
  }
  case '*':
  {
	return Codegen::builder.CreateFMul(l, r, "multmp");
  }
  case '<':
  {
	l = Codegen::builder.CreateFCmpULT(l, r, "cmptmp");
	// convert bool 0/1 to double 0.0 or 1.0
	return Codegen::
	  builder.CreateUIToFP(l, 
						   llvm::Type::getDoubleTy(Codegen::the_context), 
						   "booltmp");
  }
  default: 
  {
	return Error::logV("Invalid binary operator");
  }
  }
}

llvm::Value * CallExprAST::codegen()
{
  // look up the name in the global module table
  llvm::Function * caleef = Codegen::the_module->getFunction(callee);
  if (!caleef)
  {
	return Error::logV("Unknown function referenced");
  }
  // if argument mismatches
  if (caleef->arg_size() != args.size())
  {
	return Error::logV("Incorrect # of arguments passed");
  }
  std::vector<llvm::Value *> args_v;
  for (unsigned i = 0, e = args.size(); i!= e; ++i)
  {
	args_v.push_back(args[i]->codegen());
	if (!args_v.back())
	{
	  return nullptr;
	}
  }
  return Codegen::builder.CreateCall(caleef, args_v, "calltmp");
}

llvm::Function * PrototypeAST::codegen()
{
  return nullptr;
}

llvm::Function * FunctionAST::codegen()
{
  return nullptr;
}


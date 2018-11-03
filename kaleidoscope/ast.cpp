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
  // make the function type: double(double,double) etc...
  std::vector<llvm::Type *> doubles(args.size(), 
      llvm::Type::getDoubleTy(Codegen::the_context));
  llvm::FunctionType * ft = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(Codegen::the_context),
	  doubles, false);
  llvm::Function * f = llvm::Function::Create(ft, 
      llvm::Function::ExternalLinkage, 
      name, Codegen::the_module.get());

  // set names for all arguments
  unsigned idx = 0;
  for (auto &arg : f->args())
  {
	arg.setName(args[idx++]);
  }
  return f;
}

llvm::Function * FunctionAST::codegen()
{
  llvm::Function * the_function = 
	Codegen::the_module->getFunction(proto->getname());
  
  if (!the_function)
  {
	the_function = proto->codegen();
  }
  if (!the_function)
  {
	return nullptr;
  }
  if (!the_function->empty())
  {
	return (llvm::Function *)Error::logV("Function cannot be redefined");
  }
  // create a new basic block to start insertion into 
  llvm::BasicBlock * bb = 
	llvm::BasicBlock::Create(Codegen::the_context,
		                 "entry",
			         the_function);
  Codegen::builder.SetInsertPoint(bb);
  
  // record the function arguments in the named values map
  Codegen::named_values.clear();
  for (auto &arg : the_function->args())
  {
	Codegen::named_values[arg.getName()] = &arg;
  }
  if (llvm::Value * ret_val = body->codegen())
  {
	// finish off the function
	Codegen::builder.CreateRet(ret_val);
	//validate the generated code, checking for consistency
	llvm::verifyFunction(*the_function);
	// optimize the funciton
	Codegen::fpm.run(*the_function);

	return the_function;
  }
  the_function->eraseFromParent();
  return nullptr;
}


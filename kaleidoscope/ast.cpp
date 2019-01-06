#include "ast.h"
#include "codegen.h"
#include "error.h"

llvm::Value * NumberExprAST::codegen()
{
  return llvm::ConstantFP::get(Codegen::the_context, llvm::APFloat(val));
}

llvm::Value * IfExprAST::codegen()
{
  llvm::Value * condv = Cond->codegen();
  if (!condv)
  {
    return nullptr;
  }
  // Convert condition to a bool by comarison non-equal to 0.0 
  condv = Codegen::builder.CreateFCmpONE(condv, 
    llvm::ConstantFP::get(Codegen::the_context, llvm::APFloat(0.0)), "ifcond");

  llvm::Function * the_function = Codegen::builder.GetInsertBlock()->getParent();
  // create blocks for the then and the else cases. Insert the 'then' block at
  // the end of the function
  llvm::BasicBlock * then_bb = llvm::BasicBlock::Create(Codegen::the_context,
                                                        "then",
                                                        the_function);
  llvm::BasicBlock * else_bb = llvm::BasicBlock::Create(Codegen::the_context,
                                                        "else");
  llvm::BasicBlock * merge_bb = llvm::BasicBlock::Create(Codegen::the_context,
                                                         "ifcont");
  Codegen::builder.CreateCondBr(condv, then_bb, else_bb);

  // emit the then value
  Codegen::builder.SetInsertPoint(then_bb);
  llvm::Value * then_v = Then->codegen();
  if (!then_v)
  {
    return nullptr;
  }
  Codegen::builder.CreateBr(merge_bb);
  // codegen of 'then' can change the current block, update elseBB for the phi
  then_bb = Codegen::builder.GetInsertBlock();

  // emit the 'else' block
  the_function->getBasicBlockList().push_back(else_bb);
  Codegen::builder.SetInsertPoint(else_bb);
  llvm::Value *else_v = Else->codegen();
  if (!else_v)
  {
    return nullptr;
  }
  Codegen::builder.CreateBr(merge_bb);
  // codegen of else can change the current block, update elsebb for the phi
  else_bb = Codegen::builder.GetInsertBlock();

  // emit the merge block
  the_function->getBasicBlockList().push_back(merge_bb);
  Codegen::builder.SetInsertPoint(merge_bb);
  llvm::PHINode * pn = 
    Codegen::builder.CreatePHI(llvm::Type::getDoubleTy(Codegen::the_context), 
			       2, "iftmp");
  pn->addIncoming(then_v, then_bb);
  pn->addIncoming(else_v, else_bb);
  return pn;
}

llvm::Value * ForExprAST::codegen()
{
  // emit the start code first, without 'variable' in scope
  llvm::Value * start_val = start->codegen();
  if (!start_val)
  {
    return nullptr;
  }
  llvm::Function * the_function = 
    Codegen::builder.GetInsertBlock()->getParent();
  llvm::BasicBlock * pre_header_bb = Codegen::builder.GetInsertBlock();
  llvm::BasicBlock * loop_bb = 
    llvm::BasicBlock::Create(Codegen::the_context, "loop", the_function);

  // insert an explicit fall though from the curent block to the loop bb
  Codegen::builder.CreateBr(loop_bb);

  // start insertion in loop_bb
  Codegen::builder.SetInsertPoint(loop_bb);
  
  // start the PHI node with an entry for start
  llvm::PHINode * variable = 
    Codegen::builder.CreatePHI(llvm::Type::getDoubleTy(Codegen::the_context),
			       2, var_name.c_str());
  variable->addIncoming(start_val, pre_header_bb);

  // Within the loop, the variable is defined equal to the PHI node.  If it
  // shadows an existing variable, we have to restore it, so save it now.
  llvm::Value * oldval = Codegen::named_values[var_name];
  Codegen::named_values[var_name] = variable;

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  if (!body->codegen())
  {
    return nullptr;
  }

  // emit the step value
  llvm::Value * step_val = nullptr;
  if (step)
  {
    step_val = step->codegen();
    if (!step_val)
    {
      return nullptr;
    }
    else
    {
      // if not specified, use 1.0 as the step
      step_val = llvm::ConstantFP::get(Codegen::the_context, 
				       llvm::APFloat(1.0));
    }
  }
  llvm::Value *next_var = 
    Codegen::builder.CreateFAdd(variable, step_val, "nextvar");

  // compute the end condition
  llvm::Value * end_cond = end->codegen();
  if (!end_cond)
  {
    return nullptr;
  }
  
  // convert condition to a bool by comparing non-equal to 0.0
  end_cond = 
    Codegen::builder.CreateFCmpONE(end_cond,
				   llvm::ConstantFP::get(Codegen::the_context,
							 llvm::APFloat(0.0)), 
				   "loopcond");
  // create the "after loop" block and insert it
  llvm::BasicBlock * loop_end_bb = Codegen::builder.GetInsertBlock();
  llvm::BasicBlock * after_bb = 
    llvm::BasicBlock::Create(Codegen::the_context,
			     "afterloop",
			     the_function);
  // insert the conditional branch into the end of the loop_end_bb
  Codegen::builder.CreateCondBr(end_cond, loop_bb, after_bb);

  // any new code will be inserted in after_bb
  Codegen::builder.SetInsertPoint(after_bb);

  // add a new entry to the PHI node for the backedge
  variable->addIncoming(next_var, loop_end_bb);

  // restore the unshadowed variable
  if (oldval)
  {
    Codegen::named_values[var_name] = oldval;
  }
  else
  {
    Codegen::named_values.erase(var_name);
  }

  // for expr always returns 0.0
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(Codegen::the_context));
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
  llvm::Function * caleef = PrototypeAST::getFunction(callee);
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

std::map<std::string, std::unique_ptr<PrototypeAST>> PrototypeAST::function_protos;

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

llvm::Function * PrototypeAST::getFunction(std::string name)
{
  // try to find an existing function with this name in the current module
  if (auto * f = Codegen::the_module->getFunction(name))
  {
    return f;
  }
  // if not check whether we can codegen the declaraction from the existing prototypes
  auto fi = function_protos.find(name);
  if (fi != function_protos.end())
  {
    return fi->second->codegen();
  }
  // if no prototype exists, then there is no function defined, return null
  return nullptr;
}

llvm::Function * FunctionAST::codegen()
{
  auto &p = *proto;
  PrototypeAST::function_protos[proto->getname()] = std::move(proto);
  llvm::Function * the_function = PrototypeAST::getFunction(p.getname());
  if (!the_function)
  {
    return nullptr;
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
    Codegen::fpm->run(*the_function);
    
    return the_function;
  }
  the_function->eraseFromParent();
  return nullptr;
}


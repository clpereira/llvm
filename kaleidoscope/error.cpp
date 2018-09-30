#include "error.h"
#include "ast.h"
#include <iostream>
#include <memory>

std::unique_ptr<ExprAST> Error::log(const std::string str)
{
  std::cerr << "LogError: " << str << std::endl;
  return nullptr;
}

std::unique_ptr<PrototypeAST> Error::logP(const std::string str)
{
  log(str);
  return nullptr;
}

llvm::Value * Error::logV(const std::string str)
{
  log(str);
  return nullptr;
}

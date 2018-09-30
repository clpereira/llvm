#ifndef _ERROR_H_
#define _ERROR_H_

#include "ast.h"
#include "k_llvm.h"
#include <string>

class Error {
 public:
  static std::unique_ptr<ExprAST> log(const std::string str);
  static std::unique_ptr<PrototypeAST> logP(const std::string str);
  static llvm::Value * logV(const std::string str);
};

#endif

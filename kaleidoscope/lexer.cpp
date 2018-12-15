#include "lexer.h"

Lexer * Lexer::p_instance = nullptr;

Lexer * Lexer::instance()
{
  if (!p_instance)
  {
	p_instance = new Lexer();
  }
  return p_instance;
}

#ifndef _LEXER_H_
#define _LEXER_H_

#include <string>
#include <cstdio>
#include <cctype>
#include <cerrno>
#include <iostream>

// definition of various tokens
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  tok_if = -6,
  tok_then = -7,
  tok_else = -8,

  tok_for = -9,
  tok_in = -10,

  tok_special_char = -11,
  tok_invalid = -12
};

class Parser;

class Lexer {
 private: 
  int lastChar = ' ';
  char lastSpecialChar = ' ';
  std::string identifierStr;          // Filled in for tok_identifier
  double numVal;                      // Filler in for tok_number
  enum Token lastToken = tok_invalid; // last processed token

  static Lexer * p_instance; // singleton, only one instance of lexer allowed

  friend class Parser; // allow Parser to access the private members of the lexer

 public:
  static Lexer * instance();

  void printLastToken()
  {
    std::cout << "Lexer::lastToken: ";
    switch(lastToken)
    {
      case tok_eof:
	{
	  std::cout << "EOF" << std::endl;
	  break;
	}
    case tok_def:
	{
	  std::cout << "def" << std::endl;
	  break;
	}
    case tok_extern:
	{
	  std::cout << "extern" << std::endl;
	  break;
	}
    case tok_if:
	{
	  std::cout << "if" << std::endl;
	  break;
	}
    case tok_then:
	{
	  std::cout << "then" << std::endl;
	  break;
	}
    case tok_else:
	{
	  std::cout << "else" << std::endl;
	  break;
	}
    case tok_for:
	{
	  std::cout << "for" << std::endl;
	  break;
	}
    case tok_in:
	{
	  std::cout << "in" << std::endl;
	  break;
	}
    case tok_identifier:
	{
	  std::cout << "identifier: " << identifierStr << std::endl;
	  break;
	}
    case tok_number:
	{
	  std::cout << "number: " << std::dec << numVal << std::endl;
	  break;
	}
    case tok_invalid:
	{
	  std::cout << "invalid" << std::endl;
	  break;
	}
      default:
	{
	  std::cout << "special character: " << char(lastSpecialChar) << std::endl;
	}
    }
  }

  // 
  //  Read in a token and return its type
  //
  int getToken()
  {
    // skip whitespaces
    while (isspace(lastChar))
    {
      lastChar = getchar();
    }
    // reads in an identifier
    if (isalpha(lastChar)) // identifier: [a-zA-Z][a-zA-Z0-9]*
    {
      identifierStr = lastChar;

      // read until a non alpha or number is found
      while(isalnum((lastChar = getchar())))
      {
	identifierStr += lastChar;
      }
      if (identifierStr == "def")
      {
	lastToken = tok_def;
	return tok_def;
      }
      if (identifierStr == "if")
      {
	lastToken = tok_if;
	return tok_if;
      }
      if (identifierStr == "then")
      {
	lastToken = tok_then;
	return tok_then;
      }
      if (identifierStr == "else")
      {
	lastToken = tok_else;
	return tok_else;
      }
      if (identifierStr == "for")
      {
	lastToken = tok_for;
	return tok_for;
      }
      if (identifierStr == "in")
      {
	lastToken = tok_in;
	return tok_in;
      }
      if (identifierStr == "extern")
      {
	lastToken = tok_extern;
	return tok_extern;
      }
      // not a keywork, must be an identifier
      lastToken = tok_identifier;
      return tok_identifier;
    }
    // reads in a number
    if (isdigit(lastChar))  // numbers: [0-9.]+
    {
      std::string numStr;
      do {
	numStr += lastChar;
	lastChar = getchar();
      } while (isdigit(lastChar) || lastChar == '.');
      
      numVal = strtod(numStr.c_str(), 0);
      if (errno != 0 && numVal == 0.0)
      {
	emitError("Invalid number: "+numStr);
      }
      lastToken = tok_number;
      return tok_number;
    }
    if (lastChar == '#')
    {
      do {
	lastChar = getchar();
      } while(lastChar != EOF && lastChar != '\n' && lastChar != '\r');
      // anything but EOF, read in the next token
      if (lastChar != EOF)
      {
	return getToken();
      }
    }
    // check for EOF
    if (lastChar == EOF)
    {
      lastToken = tok_eof;
      return tok_eof;
    }                    
    lastSpecialChar = lastChar;
    lastChar = getchar();
    lastToken = tok_special_char;
    return tok_special_char; 
  }
  int getLastSpecialChar() const { return lastSpecialChar; }

 private:
  void emitError(std::string message)
  {
    std::cerr << message << std::endl;
    exit(-1);
  }
};

#endif

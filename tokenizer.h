#pragma once
#include <string>

using namespace std;

enum Symbol {
    SELECTSYM, FROMSYM, WHERESYM,
    ANDSYM, ORSYM,
    EQUALS, NOTEQUAL, LT, LTE, GT, GTE,
    STAR, COMMA,
    OPENPAREN, CLOSEPAREN,
    ID, NUMBER, STRING,
    TRUE_LIT, FALSE_LIT,
    EOL, ERROR
};

void print(Symbol sym);
void syntaxError(string msg);
Symbol getNext(string &s);
Symbol peekNext(string &s);
void setFilename(string s);
string getParsedId();
string getParsedLiteral();

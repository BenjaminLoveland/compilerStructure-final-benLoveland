#include <iostream>
#include <regex>
#include <cctype>
#include "tokenizer.h"

using namespace std;

int lineNum = 1;
string filename = "";
string parsedId;
string parsedLiteral;

string getParsedId() { return parsedId; }
string getParsedLiteral() { return parsedLiteral; }

void print(Symbol sym) {
    switch(sym) {
        case SELECTSYM: cout << "SELECT keyword"; break;
        case FROMSYM: cout << "FROM keyword"; break;
        case WHERESYM: cout << "WHERE keyword"; break;
        case ANDSYM: cout << "AND operator"; break;
        case ORSYM: cout << "OR operator"; break;
        case EQUALS: cout << "= symbol"; break;
        case NOTEQUAL: cout << "!= symbol"; break;
        case LT: cout << "< symbol"; break;
        case LTE: cout << "<= symbol"; break;
        case GT: cout << "> symbol"; break;
        case GTE: cout << ">= symbol"; break;
        case STAR: cout << "* symbol"; break;
        case COMMA: cout << ", symbol"; break;
        case OPENPAREN: cout << "Open paren ("; break;
        case CLOSEPAREN: cout << "Close paren )"; break;
        case ID: cout << "Identifier: " << parsedId; break;
        case NUMBER: cout << "Number literal: " << parsedLiteral; break;
        case STRING: cout << "String literal: " << parsedLiteral; break;
        case TRUE_LIT: cout << "true literal"; break;
        case FALSE_LIT: cout << "false literal"; break;
        case EOL: cout << "End of line"; break;
        case ERROR: cout << "Unrecognized symbol"; break;
    }
}

void syntaxError(string msg) {
    cerr << filename << ':' << lineNum << ' ' << msg << endl;
}

Symbol peekNext(string &s) {
    int tempLineCount = lineNum;
    string temp = s;
    Symbol sym = getNext(temp);
    lineNum = tempLineCount;
    return sym;
}

Symbol getNext(string &s) {
    int pos = 1;
    Symbol retVal = ERROR;

    while (!s.empty() && (s[0] == ' ' || s[0] == '\t' || s[0] == '\n')) {
        if (s[0] == '\n') lineNum++;
        s = s.substr(1);
    }
    if (s.empty()) return EOL;

    smatch sm;
    parsedId = "";
    parsedLiteral = "";

    regex idExp("^[A-Za-z_][A-Za-z0-9_]*");
    if (regex_search(s, sm, idExp)) {
        string token = sm[0].str();
        string upperToken = token;
        for (auto &c : upperToken) c = toupper(c);

        if (upperToken == "SELECT") retVal = SELECTSYM;
        else if (upperToken == "FROM") retVal = FROMSYM;
        else if (upperToken == "WHERE") retVal = WHERESYM;
        else if (upperToken == "AND") retVal = ANDSYM;
        else if (upperToken == "OR") retVal = ORSYM;
        else if (upperToken == "TRUE") retVal = TRUE_LIT;
        else if (upperToken == "FALSE") retVal = FALSE_LIT;
        else {
            retVal = ID;
            parsedId = token;
        }
        pos = token.length();
    }

    if (s.substr(0,2) == "!=") { retVal = NOTEQUAL; pos = 2; }
    else if (s.substr(0,2) == "<=") { retVal = LTE; pos = 2; }
    else if (s.substr(0,2) == ">=") { retVal = GTE; pos = 2; }
    else if (s[0] == '=') { retVal = EQUALS; pos = 1; }
    else if (s[0] == '<') { retVal = LT; pos = 1; }
    else if (s[0] == '>') { retVal = GT; pos = 1; }
    else if (s[0] == '*') { retVal = STAR; pos = 1; }
    else if (s[0] == ',') { retVal = COMMA; pos = 1; }
    else if (s[0] == '(') { retVal = OPENPAREN; pos = 1; }
    else if (s[0] == ')') { retVal = CLOSEPAREN; pos = 1; }

    regex numExp("^[0-9]+(\\.[0-9]+)?");
    if (retVal == ERROR && regex_search(s, sm, numExp)) {
        parsedLiteral = sm[0].str();
        pos = parsedLiteral.length();
        retVal = NUMBER;
    }

    regex strExp("^\"[^\"]*\"");
    if (retVal == ERROR && regex_search(s, sm, strExp)) {
        parsedLiteral = sm[0].str();
        pos = parsedLiteral.length();
        retVal = STRING;
    }

    s = s.substr(pos);
    return retVal;
}

void setFilename(string s) { filename = s; }

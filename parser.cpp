#include "parser.h"
#include <sstream>

using namespace std;

static void indent(ostream& os, int n) {
    for (int i=0;i<n;i++) os << ' ';
}

static bool accept(string &s, Symbol sym) {
    Symbol got = peekNext(s);
    if (got == sym) { (void)getNext(s); return true; }
    return false;
}

static bool expect(string &s, Symbol sym, const char* err) {
    if (accept(s, sym)) return true;
    stringstream ss;
    ss << "Expected " ;
    ss << err;
    syntaxError(ss.str());
    return false;
}

static bool parseFieldList(string &s, Query &q);
static bool parseBoolExpr(string &s, shared_ptr<BoolExpr>& out);
static bool parseBoolTerm(string &s, shared_ptr<BoolExpr>& out);
static bool parseBoolFactor(string &s, shared_ptr<BoolExpr>& out);
static bool parsePredicate(string &s, shared_ptr<BoolExpr>& out);

bool parseQuery(string &s, Query &out) {
    if (!expect(s, SELECTSYM, "SELECT")) return false;

    if (!parseFieldList(s, out)) return false;

    if (!expect(s, FROMSYM, "FROM")) return false;

    if (peekNext(s) != ID) {
        syntaxError("Expected identifier after FROM");
        return false;
    }
    (void)getNext(s);
    out.fromIdent = getParsedId();

    if (accept(s, WHERESYM)) {
        shared_ptr<BoolExpr> where;
        if (!parseBoolExpr(s, where)) return false;
        out.where = where;
    }

    return true;
}

static bool parseFieldList(string &s, Query &q) {
    Symbol sym = peekNext(s);
    if (sym == STAR) {
        (void)getNext(s);
        q.selectAll = true;
        return true;
    }
    if (sym == ID) {
        (void)getNext(s);
        q.fields.push_back(getParsedId());
        while (accept(s, COMMA)) {
            if (peekNext(s) != ID) {
                syntaxError("Expected identifier after ',' in field list");
                return false;
            }
            (void)getNext(s);
            q.fields.push_back(getParsedId());
        }
        q.selectAll = false;
        return true;
    }
    syntaxError("Expected '*' or identifier list after SELECT");
    return false;
}

static bool parseBoolExpr(string &s, shared_ptr<BoolExpr>& out) {
    shared_ptr<BoolExpr> first;
    if (!parseBoolTerm(s, first)) return false;

    if (!accept(s, ORSYM)) {
        out = first;
        return true;
    }

    auto node = make_shared<OrExpr>();
    node->terms.push_back(first);

    do {
        shared_ptr<BoolExpr> t;
        if (!parseBoolTerm(s, t)) return false;
        node->terms.push_back(t);
    } while (accept(s, ORSYM));

    out = node;
    return true;
}

static bool parseBoolTerm(string &s, shared_ptr<BoolExpr>& out) {
    // BOOL_TERM := BOOL_FACTOR ( "AND" BOOL_FACTOR )*
    shared_ptr<BoolExpr> first;
    if (!parseBoolFactor(s, first)) return false;

    if (!accept(s, ANDSYM)) {
        out = first;
        return true;
    }

    auto node = make_shared<AndExpr>();
    node->factors.push_back(first);

    do {
        shared_ptr<BoolExpr> f;
        if (!parseBoolFactor(s, f)) return false;
        node->factors.push_back(f);
    } while (accept(s, ANDSYM));

    out = node;
    return true;
}

static bool parseBoolFactor(string &s, shared_ptr<BoolExpr>& out) {
    // BOOL_FACTOR := "(" BOOL_EXPR ")" | PREDICATE
    if (accept(s, OPENPAREN)) {
        shared_ptr<BoolExpr> inner;
        if (!parseBoolExpr(s, inner)) return false;
        if (!expect(s, CLOSEPAREN, "')'")) return false;
        auto p = make_shared<ParenExpr>();
        p->inner = inner;
        out = p;
        return true;
    }
    return parsePredicate(s, out);
}

static bool isCompOp(Symbol sym) {
    return sym==EQUALS || sym==NOTEQUAL || sym==LT || sym==LTE || sym==GT || sym==GTE;
}

static bool isLiteral(Symbol sym) {
    return sym==NUMBER || sym==STRING || sym==TRUE_LIT || sym==FALSE_LIT;
}

static bool parsePredicate(string &s, shared_ptr<BoolExpr>& out) {
    if (peekNext(s) != ID) {
        syntaxError("Expected identifier at start of predicate");
        return false;
    }
    (void)getNext(s);
    string ident = getParsedId();

    Symbol op = peekNext(s);
    if (!isCompOp(op)) {
        syntaxError("Expected comparison operator (=, !=, <, <=, >, >=)");
        return false;
    }
    (void)getNext(s);

    Symbol litSym = peekNext(s);
    if (!isLiteral(litSym)) {
        syntaxError("Expected literal (number, string, true, false)");
        return false;
    }
    (void)getNext(s);
    string lit = getParsedLiteral();
    if (lit.empty()) {
        if (litSym == TRUE_LIT) lit = "true";
        else if (litSym == FALSE_LIT) lit = "false";
    }

    auto p = make_shared<Predicate>();
    p->ident = ident;
    p->op = op;
    p->literalText = lit;
    p->literalKind = litSym;
    out = p;
    return true;
}

void OrExpr::print(ostream& os, int indentLvl) const {
    indent(os, indentLvl); os << "OR\n";
    for (auto& t : terms) {
        t->print(os, indentLvl + 2);
    }
}

void AndExpr::print(ostream& os, int indentLvl) const {
    indent(os, indentLvl); os << "AND\n";
    for (auto& f : factors) {
        f->print(os, indentLvl + 2);
    }
}

void Predicate::print(ostream& os, int indentLvl) const {
    indent(os, indentLvl);
    os << "PREDICATE " << ident << ' ';
    switch (op) {
        case EQUALS: os << "="; break;
        case NOTEQUAL: os << "!="; break;
        case LT: os << "<"; break;
        case LTE: os << "<="; break;
        case GT: os << ">"; break;
        case GTE: os << ">="; break;
        default: os << "?"; break;
    }
    os << ' ' << literalText << "\n";
}

void ParenExpr::print(ostream& os, int indentLvl) const {
    indent(os, indentLvl); os << "(\n";
    if (inner) inner->print(os, indentLvl + 2);
    indent(os, indentLvl); os << ")\n";
}

void Query::print(ostream& os) const {
    os << "QUERY\n";
    os << "  SELECT ";
    if (selectAll) {
        os << "*\n";
    } else {
        for (size_t i=0;i<fields.size();++i) {
            os << fields[i];
            if (i+1<fields.size()) os << ", ";
        }
        os << "\n";
    }
    os << "  FROM " << fromIdent << "\n";
    if (where) {
        os << "  WHERE\n";
        where->print(os, 4);
    }
}

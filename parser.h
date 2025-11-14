#pragma once
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "tokenizer.h"

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::vector;

struct BoolExpr {
    virtual ~BoolExpr() = default;
    virtual void print(std::ostream& os, int indent=0) const = 0;
};

struct OrExpr : BoolExpr {
    vector<shared_ptr<BoolExpr>> terms;
    void print(std::ostream& os, int indent=0) const override;
};

struct AndExpr : BoolExpr {
    vector<shared_ptr<BoolExpr>> factors;
    void print(std::ostream& os, int indent=0) const override;
};

struct Predicate : BoolExpr {
    string ident;
    Symbol op;
    string literalText;
    Symbol literalKind;
    void print(std::ostream& os, int indent=0) const override;
};

struct ParenExpr : BoolExpr {
    shared_ptr<BoolExpr> inner;
    void print(std::ostream& os, int indent=0) const override;
};

struct Query {
    bool selectAll = false;
    vector<string> fields;
    string fromIdent;
    shared_ptr<BoolExpr> where;

    void print(std::ostream& os) const;
};

bool parseQuery(string &s, Query &out);

inline std::ostream& operator<<(std::ostream& os, const Query& q) {
    q.print(os);
    return os;
}

#include "symbol_table.h"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

static bool checkBoolExpr(const BoolExpr* expr, const SymbolTable& schema);
static bool checkPredicate(const Predicate* pred, const SymbolTable& schema);

static const char* fieldTypeName(FieldType t) {
    switch (t) {
        case FT_NUMBER: return "number";
        case FT_STRING: return "string";
        case FT_BOOL:   return "bool";
        default:        return "unknown";
    }
}

static const char* literalTypeName(Symbol s) {
    switch (s) {
        case NUMBER:    return "number literal";
        case STRING:    return "string literal";
        case TRUE_LIT:  return "true literal";
        case FALSE_LIT: return "false literal";
        default:        return "unknown literal";
    }
}

bool checkQuerySemantics(const Query& q, const SymbolTable& schema) {
    bool ok = true;

    if (!q.selectAll) {
        for (const auto& f : q.fields) {
            if (!schema.hasField(f)) {
                cerr << "Semantic error: unknown field in SELECT list: '"
                     << f << "'\n";
                ok = false;
            }
        }
    }

    if (q.where) {
        if (!checkBoolExpr(q.where.get(), schema)) {
            ok = false;
        }
    }

    return ok;
}

static bool checkBoolExpr(const BoolExpr* expr, const SymbolTable& schema) {
    if (auto orExpr = dynamic_cast<const OrExpr*>(expr)) {
        bool ok = true;
        for (auto& t : orExpr->terms) {
            if (!checkBoolExpr(t.get(), schema)) ok = false;
        }
        return ok;
    } else if (auto andExpr = dynamic_cast<const AndExpr*>(expr)) {
        bool ok = true;
        for (auto& f : andExpr->factors) {
            if (!checkBoolExpr(f.get(), schema)) ok = false;
        }
        return ok;
    } else if (auto pred = dynamic_cast<const Predicate*>(expr)) {
        return checkPredicate(pred, schema);
    } else if (auto par = dynamic_cast<const ParenExpr*>(expr)) {
        if (!par->inner) return true;
        return checkBoolExpr(par->inner.get(), schema);
    }
    return true;
}

static bool checkPredicate(const Predicate* pred, const SymbolTable& schema) {
    bool ok = true;

    if (!schema.hasField(pred->ident)) {
        cerr << "Semantic error: unknown field in WHERE: '"
             << pred->ident << "'\n";
        return false;
    }

    FieldType ft = schema.getFieldType(pred->ident);

    if (ft == FT_NUMBER && pred->literalKind != NUMBER) {
        cerr << "Semantic error: field '" << pred->ident
             << "' is a number, but compared with "
             << literalTypeName(pred->literalKind) << "\n";
        ok = false;
    }

    if (ft == FT_STRING && pred->literalKind != STRING) {
        cerr << "Semantic error: field '" << pred->ident
             << "' is a string, but compared with "
             << literalTypeName(pred->literalKind) << "\n";
        ok = false;
    }

    if (ft == FT_BOOL &&
        !(pred->literalKind == TRUE_LIT || pred->literalKind == FALSE_LIT)) {
        cerr << "Semantic error: field '" << pred->ident
             << "' is a bool, but compared with "
             << literalTypeName(pred->literalKind) << "\n";
        ok = false;
    }

    bool relational = (pred->op == LT || pred->op == LTE ||
                       pred->op == GT || pred->op == GTE);

    if (relational && ft != FT_NUMBER) {
        cerr << "Semantic error: relational operator (<, <=, >, >=) used on non-number field '"
             << pred->ident << "' of type " << fieldTypeName(ft) << "\n";
        ok = false;
    }

    return ok;
}

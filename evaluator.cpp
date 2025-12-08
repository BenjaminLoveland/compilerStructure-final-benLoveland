#include "evaluator.h"
#include <cstdlib>
#include <cctype>

static bool evalBoolExpr(const BoolExpr* expr, const Row& row);
static bool evalOrExpr(const OrExpr* expr, const Row& row);
static bool evalAndExpr(const AndExpr* expr, const Row& row);
static bool evalPredicate(const Predicate* expr, const Row& row);
static bool evalParen(const ParenExpr* expr, const Row& row);

static bool parseNumber(const string& s, double& out) {
    char* endptr = nullptr;
    out = std::strtod(s.c_str(), &endptr);
    return endptr != s.c_str() && *endptr == '\0';
}

static int compareLiterals(const string& a, const string& b) {
    double da, db;
    bool na = parseNumber(a, da);
    bool nb = parseNumber(b, db);

    if (na && nb) {
        if (da < db) return -1;
        if (da > db) return 1;
        return 0;
    }
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static string normalizeLiteral(const Predicate* p) {
    if (p->literalKind == TRUE_LIT)  return "true";
    if (p->literalKind == FALSE_LIT) return "false";
    if (p->literalKind == STRING) {
        if (!p->literalText.empty() &&
            p->literalText.front() == '"' &&
            p->literalText.back() == '"') {
            return p->literalText.substr(1, p->literalText.size()-2);
        }
    }
    return p->literalText;
}

static bool evalBoolExpr(const BoolExpr* expr, const Row& row) {
    if (auto orExpr = dynamic_cast<const OrExpr*>(expr)) {
        return evalOrExpr(orExpr, row);
    } else if (auto andExpr = dynamic_cast<const AndExpr*>(expr)) {
        return evalAndExpr(andExpr, row);
    } else if (auto pred = dynamic_cast<const Predicate*>(expr)) {
        return evalPredicate(pred, row);
    } else if (auto par = dynamic_cast<const ParenExpr*>(expr)) {
        return evalParen(par, row);
    }
    return false;
}

static bool evalOrExpr(const OrExpr* expr, const Row& row) {
    for (auto& term : expr->terms) {
        if (evalBoolExpr(term.get(), row)) return true;
    }
    return false;
}

static bool evalAndExpr(const AndExpr* expr, const Row& row) {
    for (auto& f : expr->factors) {
        if (!evalBoolExpr(f.get(), row)) return false;
    }
    return true;
}

static bool evalParen(const ParenExpr* expr, const Row& row) {
    if (!expr->inner) return false;
    return evalBoolExpr(expr->inner.get(), row);
}

static bool evalPredicate(const Predicate* expr, const Row& row) { 
    auto it = row.find(expr->ident);
    if (it == row.end()) {
        return false;
    }
    string left = it->second;
    string right = normalizeLiteral(expr);

    int cmp = compareLiterals(left, right);

    switch (expr->op) {
        case EQUALS:   return cmp == 0;
        case NOTEQUAL: return cmp != 0;
        case LT:       return cmp < 0;
        case LTE:      return cmp <= 0;
        case GT:       return cmp > 0;
        case GTE:      return cmp >= 0;
        default:       return false;
    }
}

Table evaluateQuery(const Query& q, const Table& input) {
    Table result;

    for (const Row& r : input) {
        bool keep = true;
        if (q.where) {
            keep = evalBoolExpr(q.where.get(), r);
        }
        if (!keep) continue;

        Row outRow;
        if (q.selectAll) {
            outRow = r;
        } else {
            for (const auto& field : q.fields) {
                auto it = r.find(field);
                if (it != r.end()) {
                    outRow[field] = it->second;
                } else {
                    outRow[field] = "";
                }
            }
        }
        result.push_back(outRow);
    }

    return result;
}

#include <iostream>
#include "tokenizer.h"
#include "parser.h"
#include "evaluator.h"
#include "symbol_table.h"

using namespace std;

static void printTable(const Table& t) {
    if (t.empty()) {
        cout << "(no rows)\n";
        return;
    }
    // Print header from first row's keys
    const Row& first = t[0];
    for (auto it = first.begin(); it != first.end(); ++it) {
        cout << it->first;
        if (next(it) != first.end()) cout << "\t";
    }
    cout << "\n";

    for (const Row& r : t) {
        for (auto it = first.begin(); it != first.end(); ++it) {
            auto jt = r.find(it->first);
            if (jt != r.end()) cout << jt->second;
            if (next(it) != first.end()) cout << "\t";
        }
        cout << "\n";
    }
}

int main() {
    Table customers = {
        { {"name","Alice"}, {"age","25"}, {"status","vip"}, {"active","true"} },
        { {"name","Ben"}, {"age","22"}, {"status","regular"}, {"active","true"} },
        { {"name","Bob"},   {"age","19"}, {"status","regular"}, {"active","true"} },
        { {"name","Carol"}, {"age","42"}, {"status","regular"}, {"active","false"} },
        { {"name","Ava"}, {"age","19"}, {"status","vip"}, {"active","false"} },
    };

    cout << "Enter query:\n";
    string input, line;
    while (true) {
        std::getline(cin, line);
        if (line.empty()) break;
        input += line + "\n";
    }

    setFilename("stdin");

    Query q;
    if (!parseQuery(input, q)) {
        cerr << "Parse failed.\n";
        return 1;
    }

    SymbolTable schema;
    schema.addField("name",   FT_STRING);
    schema.addField("age",    FT_NUMBER);
    schema.addField("status", FT_STRING);
    schema.addField("active", FT_BOOL);

    if (!checkQuerySemantics(q, schema)) {
        cerr << "Semantic check failed. Aborting.\n";
        return 1;
    }

    Table out = evaluateQuery(q, customers);
    printTable(out);
}

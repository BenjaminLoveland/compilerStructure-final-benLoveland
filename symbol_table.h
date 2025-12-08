#pragma once
#include <string>
#include <map>
#include "parser.h"

using std::string;
using std::map;

enum FieldType {
    FT_NUMBER,
    FT_STRING,
    FT_BOOL
};

struct FieldInfo {
    FieldType type;
};

class SymbolTable {
public:
    void addField(const string& name, FieldType t) {
        fields[name] = FieldInfo{t};
    }

    bool hasField(const string& name) const {
        return fields.find(name) != fields.end();
    }

    FieldType getFieldType(const string& name) const {
        auto it = fields.find(name);
        if (it == fields.end()) {
            return FT_STRING;
        }
        return it->second.type;
    }

private:
    map<string, FieldInfo> fields;
};

bool checkQuerySemantics(const Query& q, const SymbolTable& schema);

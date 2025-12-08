#pragma once
#include <string>
#include <vector>
#include <map>
#include "parser.h"

using std::string;
using std::vector;
using std::map;

using Row = map<string,string>;
using Table = vector<Row>;

Table evaluateQuery(const Query& q, const Table& input);

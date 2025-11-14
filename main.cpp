#include <iostream>
#include "tokenizer.h"
#include "parser.h"

using namespace std;

int main() {
    string s =
        "SELECT name, age FROM Customers "
        "WHERE (age >= 21 AND active = true) OR status = \"vip\" \n";

    setFilename("demo.sql");
    Query q;
    if (parseQuery(s, q)) {
        cout << q << std::endl;
    } else {
        cerr << "Parse failed.\n";
    }
}

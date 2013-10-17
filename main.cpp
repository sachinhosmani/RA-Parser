#include <iostream>
#include <string>
#include "parser.h"
#include "table.h"

using namespace std;

map<string, Table> ENV;

int main() {
	/*map<string, string> attr_types;
	attr_types.insert(pair<string, string>("ID", "int"));
	attr_types.insert(pair<string, string>("Name", "varchar"));
	Table t(attr_types);
	map<string, boost::any> m;
	m.insert(pair<string, int>("ID", 2));
	m.insert(pair<string, string>("Name", "John"));
	t.insert_tuple(m);
	m.clear();
	m.insert(pair<string, int>("ID", 5));
	m.insert(pair<string, string>("Name", "Rambo"));
	t.insert_tuple(m);
	m.clear();
	m.insert(pair<string, int>("ID", 23));
	m.insert(pair<string, string>("Name", "X"));
	t.insert_tuple(m);
	t.print();*/
	string s;
	string query = "";
	do {
		if (query.length() == 0)
			cout << ">> ";
		else
			cout << "   ";
		getline(cin ,s);
		if (cin.eof())
			break;
		query += s;
		if (s[s.length() - 1] == ';') {
			try {
				Table t = parse(query.substr(0, query.length() - 1));
				t.print();
				ENV.insert(pair<string, Table>(t.name, t));
			} catch (SYNTAX_ERROR e) {
				cerr << "Syntax Error in statement :: " << e.stmt << " :: " <<
				e.msg << endl;
			}
			query = "";
		} else {
			query += " ";
		}

	} while(true);
	return 0;
}

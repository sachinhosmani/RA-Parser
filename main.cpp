#include <iostream>
#include <string>
#include "parser.h"
#include "table.h"

using namespace std;

map<string, Table> ENV;

int main() {
	vector<string> attr_names;
	attr_names.push_back("Name");
	attr_names.push_back("ID");
	vector<string> attr_types;
	attr_types.push_back("varchar");
	attr_types.push_back("int");
	Table t("table1", attr_names, attr_types);
	ENV.insert(pair<string, Table>("table1", t));
	t = Table("table2", attr_names, attr_types);
	ENV.insert(pair<string, Table>("table2", t));

	Tuple t1;
	t1.insert(pair<string, boost::any>("Name", string("A")));
	t1.insert(pair<string, boost::any>("ID", 6));
	ENV["table1"].insert_tuple(t1);
	Tuple t2;
	t2.insert(pair<string, boost::any>("Name", string("B")));
	t2.insert(pair<string, boost::any>("ID", 5));
	ENV["table2"].insert_tuple(t2);

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

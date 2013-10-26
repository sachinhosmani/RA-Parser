#include <iostream>
#include <string>
#include "parser.h"
#include "table.h"
#include "predicate.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

map<string, Table> ENV;

void init();

int main() {
	/*vector<string> attr_names;
	attr_names.push_back("Name");
	attr_names.push_back("ID");
	attr_names.push_back("Phone");
	vector<string> attr_types;
	attr_types.push_back("varchar");
	attr_types.push_back("int");
	attr_types.push_back("int");
	Table t("table1", attr_names, attr_types);
	ENV.insert(pair<string, Table>("table1", t));
	t = Table("table2", attr_names, attr_types);
	ENV.insert(pair<string, Table>("table2", t));

	Tuple t1;
	t1.insert(pair<string, boost::any>("Name", string("A")));
	t1.insert(pair<string, boost::any>("ID", 6));
	t1.insert(pair<string, boost::any>("Phone", 6514654));
	ENV["table1"].insert_tuple(t1);
	t1.clear();
	t1.insert(pair<string, boost::any>("Name", string("G")));
	t1.insert(pair<string, boost::any>("ID", 45));
	t1.insert(pair<string, boost::any>("Phone", 3462344));
	ENV["table1"].insert_tuple(t1);
	t1.clear();
	t1.insert(pair<string, boost::any>("Name", string("Q")));
	t1.insert(pair<string, boost::any>("ID", 3));
	t1.insert(pair<string, boost::any>("Phone", 54675877));
	ENV["table1"].insert_tuple(t1);
	t1.clear();
	t1.insert(pair<string, boost::any>("Name", string("R")));
	t1.insert(pair<string, boost::any>("ID", 6));
	t1.insert(pair<string, boost::any>("Phone", 6514654));
	ENV["table1"].insert_tuple(t1);
	t1.clear();
	t1.insert(pair<string, boost::any>("Name", string("G")));
	t1.insert(pair<string, boost::any>("ID", 5));
	t1.insert(pair<string, boost::any>("Phone", 3462344));
	ENV["table1"].insert_tuple(t1);
	t1.clear();
	t1.insert(pair<string, boost::any>("Name", string("A")));
	t1.insert(pair<string, boost::any>("ID", 5));
	t1.insert(pair<string, boost::any>("Phone", 54675877));
	ENV["table1"].insert_tuple(t1);
	Tuple t2;
	t2.insert(pair<string, boost::any>("Name", string("B")));
	t2.insert(pair<string, boost::any>("ID", 4));
	t2.insert(pair<string, boost::any>("Phone", 67777775));
	ENV["table2"].insert_tuple(t2);
	t2.clear();
	t2.insert(pair<string, boost::any>("Name", string("Y")));
	t2.insert(pair<string, boost::any>("ID", 12));
	t2.insert(pair<string, boost::any>("Phone", 45645545));
	ENV["table2"].insert_tuple(t2);
	t2.clear();
	t2.insert(pair<string, boost::any>("Name", string("E")));
	t2.insert(pair<string, boost::any>("ID", 1));
	t2.insert(pair<string, boost::any>("Phone", 56767867));
	ENV["table2"].insert_tuple(t2);*/

	init();
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

void init() {
	path p("./db/");
	directory_iterator end;
	for (directory_iterator it(p); it != end; ++it) {
		if (is_directory(*it) && it->path().filename() != "tmp") {
			directory_iterator it2(it->path());
			string file = (it2->path()).string();
			string main_file;
			string md_file;
			if (file.find("_metadata") != string::npos)
				md_file = file;
			else
				main_file = file;
			it2++;
			file = (it2->path()).string();
			if (md_file == "")
				md_file = file;
			else
				main_file = file;
			Table t(main_file, md_file);
			ENV.insert(pair<string, Table>(t.name, t));
		}
	}
}

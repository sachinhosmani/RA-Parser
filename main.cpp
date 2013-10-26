#include <iostream>
#include <string>
#include "parser.h"
#include "table.h"
#include "predicate.h"
#include <boost/filesystem.hpp>
#include <csignal>

using namespace std;
using namespace boost::filesystem;

map<string, Table> ENV;

void exit_handler(int a_signal);
void init();

int main() {
	srand(time(NULL));
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);
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

void exit_handler(int a_signal) {
	if (a_signal == SIGINT || a_signal == SIGTERM) {
		ENV.clear();
		path p("./db/tmp");
		directory_iterator end;
		for (directory_iterator it(p); it != end; ++it) {
			remove(it->path().string().c_str());
		}
		exit(0);
	}
}

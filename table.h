#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <vector>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include "predicate.h"
#include <fstream>
#include <cstdio>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

typedef map<string, boost::any> Tuple;

class Table {
	vector<Tuple> tuples;
	int tuples_read;
	ifstream *f_read;
	ifstream *md_read;
	ofstream *f_write;
	ofstream *md_write;
	bool eof;
	path file;
	path md_file;
	Tuple line_to_tuple(const string &line);
public:
	vector<string> attr_names;
	vector<string> attr_types;
	map<string, string> attr_type_map;
	string name;
	Table();
	Table(const string &a_name, const vector<string> &a_attr_names, const vector<string> &a_attr_types);
	Table(const path &a_file, const path &a_md_file);
	Tuple next_tuple();
	void insert_tuple(const Tuple &t);
	void insert_tuple(const vector<string> &values);
	Table cross(Table t);
	Table project(const vector<string> &a_attr_names);
	Table select(Predicate *p);
	void rename(const string &, const vector<string> &);
	Table theta_join(Table t, Predicate *p);
	Table natural_join(Table t);
	Table aggregate(const vector<string> &group_attrs, const vector<string> &funcs, const vector<string> &attrs);
	Table order_by(const vector<string> &attrs);
	Table union_(Table t);
	Table intersection(Table t);
	Table set_difference(Table t);
	void print();
	void reset();
	bool compatible(Table t);
	static bool satisfies(Predicate *p, const Tuple &t);
	static bool satisfies(Simple_Predicate *p, const Tuple &t);
	static boost::any parse_e_tree(Expression_Tree *e, const Tuple &t);
	bool buffer_empty();
	bool end_of_table();
	void make_permanent();
	string get_file_path();
	string get_md_file_path();
};

class TABLE_ERROR {
public:
	string msg;
	inline TABLE_ERROR(const string &a_msg):msg(a_msg){};
};

#endif

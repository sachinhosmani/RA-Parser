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

using namespace std;

typedef map<string, boost::any> Tuple;

class Table {
	vector<Tuple> tuples;
	int tuples_read;
	ifstream *f_read;
	ifstream *md_read;
	ofstream *f_write;
	ofstream *md_write;
	bool eof;
	string file;
	string md_file;
	Tuple line_to_tuple(const string &line);
public:
	vector<string> attr_names;
	vector<string> attr_types;
	map<string, string> attr_type_map;
	string name;
	Table();
	Table(const string &a_name, const vector<string> &a_attr_names, const vector<string> &a_attr_types);
	Table(const string &a_file, const string &a_md_file);
	Tuple next_tuple();
	void insert_tuple(const Tuple &t);
	void insert_tuple(const vector<string> &values);
	Table cross(Table t);
	Table project(const vector<string> &a_attr_names);
	Table select(Predicate *p);
	void rename(const string &, const vector<string> &);
	void print();
	void reset();
	static bool satisfies(Predicate *p, const Tuple &t);
	static bool satisfies(Simple_Predicate *p, const Tuple &t);
	static boost::any parse_e_tree(Expression_Tree *e, const Tuple &t);
	bool buffer_empty();
	bool end_of_table();
};

class TABLE_ERROR {
public:
	string msg;
	inline TABLE_ERROR(const string &a_msg):msg(a_msg){};
};

static bool check_truth(boost::any a, boost::any b, int cond);
static bool is_string_literal(const string &s);
static bool is_int_literal(const string &s);
#endif

#ifndef PREDICATE_H
#define PREDICATE_H

#include "tokenizer.h"
#include <iostream>

using namespace std;

const string PREDICATE_OP_SYMBOLS[] = {"or", "and"};
const int PREDICATE_OP_COUNT = 2;

const string COND_OP_SYMBOLS[] = {"<", "<=", ">", ">=", "=="};
const int COND_OP_COUNT = 5;

const string EXPRESSION_OP_SYMBOLS[] = {"+", "-", "/", "*", "%%"};
const int EXPRESSION_OP_COUNT = 5;

struct Expression_Tree {
	Expression_Tree *left;
	string data;
	Expression_Tree *right;
};

struct Simple_Predicate {
	int cond;
	Expression_Tree *left;
	Expression_Tree *right;
};

struct Predicate {
	int op;
	Predicate *left;
	Predicate *right;
	Simple_Predicate *sp;
};

int predicate_op_sym_to_code(const string &op_sym);
int cond_op_sym_to_code(const string &op_sym);
Predicate *create_predicate(const string &p_query);
Simple_Predicate *create_simple_predicate(const string &sp_query);
Expression_Tree *create_expression_tree(const string &e_query);

class PARSE_TREE_ERROR {
public:
	string msg;
	inline PARSE_TREE_ERROR(const string &a_msg):msg(a_msg){};
};

#endif

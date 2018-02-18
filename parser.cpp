//	compile: g++ parser.cpp -o parser
//	run: ./parser

// To test with another grammar,
// maps and lists inside initialize()
// can be filled with corresponding data;
// but it is important that starting symbol
// should be "Prog".

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <stack>
using namespace std;

typedef map<string, map<string, list<string> > > table_type;
typedef map<string, list<string> > map_type;
typedef list<string> list_type;

class Symbols {
private:
	list_type nonterminals;
	list_type terminals;
public:
	Symbols(){}
	list_type& getNonterminals() { return nonterminals; }
	list_type& getTerminals() { return terminals; }
	~Symbols(){}
};

Symbols symbol_list;	// store terminals and nonterminals
map_type production_map;	// store production rules
map_type first_map;		// store first sets
map_type follow_map;	// store follow sets
table_type parsing_table;	// store parsing table
void initialize();		// fill production, first, follow maps
void find_first();		// calculate first sets
void find_follow();		// calculate follow sets
void find_table();		// calculate parsing table
void parse();			// parse
int find_type(const string&);	// decide symbol if nonterminal or terminal
int store(list_type&, const string&);	// store given symbol at given container
string reverse_string(const string&);	// reverse given string to store in stack

int main() {
	initialize();
	find_first();
	find_follow();
	find_table();
	parse();
	return 0;
}

void initialize() {
	production_map["Prog"].push_back("{ Deyimler } Eof");	// initialize production rules
	production_map["Deyimler"].push_back("Deyim Deyimler");
	production_map["Deyimler"].push_back("epsilon");
	production_map["Deyim"].push_back("id = Exp ;");
	production_map["Deyim"].push_back("if ( Exp ) Deyim");
	production_map["Exp"].push_back("id Expt");
	production_map["Expt"].push_back("+ Exp");
	production_map["Expt"].push_back("- Exp");
	production_map["Expt"].push_back("epsilon");
	symbol_list.getNonterminals().push_back("Prog");		// initialize nonterminals
	symbol_list.getNonterminals().push_back("Deyimler");
	symbol_list.getNonterminals().push_back("Deyim");
	symbol_list.getNonterminals().push_back("Exp");
	symbol_list.getNonterminals().push_back("Expt");
	symbol_list.getTerminals().push_back("-");				// initialize terminals
	symbol_list.getTerminals().push_back("+");
	symbol_list.getTerminals().push_back("if");
	symbol_list.getTerminals().push_back("id");
	symbol_list.getTerminals().push_back("(");
	symbol_list.getTerminals().push_back(")");
	symbol_list.getTerminals().push_back("=");
	symbol_list.getTerminals().push_back(";");
	symbol_list.getTerminals().push_back("{");
	symbol_list.getTerminals().push_back("}");
	symbol_list.getTerminals().push_back("Eof");

	// grameri yazdir
	cout << "gramer:" << endl;
	list_type nonterminal_list = symbol_list.getNonterminals();
	for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
		for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
			cout << *it1 << " -> " << *it2 << endl;
		}
	}
	cout << endl;
}

void find_first() {
	bool changed;
	list_type nonterminal_list = symbol_list.getNonterminals();
	list_type terminal_list = symbol_list.getTerminals();

	// rule 1
	for (list_type::iterator it1 = terminal_list.begin(); it1 != terminal_list.end(); it1++) {
		store(first_map[*it1], *it1);
	}

	// rule 2
	for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
		string first_symbol;
		for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
			first_symbol = (*it2).substr(0, (*it2).find(' '));
			if (find_type(first_symbol) == 2) {		// check if symbol is terminal
				store(first_map[*it1], first_symbol);
			}
		}
	}

	// rule 3
	for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
		string first_symbol;
		for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
			first_symbol = (*it2).substr(0, (*it2).find(' '));
			if (find_type(first_symbol) == 0) {		// check if symbol is epsilon
				store(first_map[*it1], first_symbol);
			}
		}
	}

	// rule 4
	do {
		changed = false;
		for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
			for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
				string symbol1;		// previous symbol
				string symbol2;		// next symbol
				stringstream stream1(*it2);
				stringstream stream2(*it2);
				while (stream1 >> symbol1 && stream2 >> symbol2) {
					while (stream2 >> symbol2) {
						if (find(first_map[symbol2].begin(), first_map[symbol2].end(), "epsilon") != first_map[symbol2].end()) {
							// check if epsilon exists in first set of next symbol
							for (list_type::iterator it3 = first_map[symbol1].begin(); it3 != first_map[symbol1].end(); it3++) {
								// add first set of previous symbol to first set of nonterminal
								changed = changed || store(first_map[*it1], *it3);
							}
						}
					}
				}
			}
		}
	} while (changed);	// repeat rule if sets are changed

	return;
}

void find_follow() {
	bool changed;
	list_type nonterminal_list = symbol_list.getNonterminals();
	list_type terminal_list = symbol_list.getTerminals();

	// rule 1
	store(follow_map["Prog"], "Eof");

	// rule 2
	for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
		string previous_symbol;	// previous symbol
		string next_symbol;		// next symbol
		for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
			stringstream stream(*it2);
			stream >> previous_symbol;
			while (stream >> next_symbol) {
				if (find_type(previous_symbol) == 1) {	// check if previous symbol is nonterminal
					for (list_type::iterator it3 = first_map[next_symbol].begin(); it3 != first_map[next_symbol].end(); it3++) {
						if (*it3 != "epsilon") {	// add first set except epsilon of next symbol to follow set of previous symbol
							store(follow_map[previous_symbol], *it3);
						}
					}
				}
				previous_symbol = next_symbol;
			}
		}
	}

	// rule 3
	do {
		changed = false;
		for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
			string previous_symbol;	// previous symbol
			string next_symbol;		// next symbol
			for (list_type::iterator it2 = production_map[*it1].begin(); it2 != production_map[*it1].end(); it2++) {
				stringstream stream(*it2);
				stream >> previous_symbol;
				while (stream >> next_symbol) {
					if (find_type(previous_symbol) == 1 && find(first_map[next_symbol].begin(), first_map[next_symbol].end(), "epsilon") != first_map[next_symbol].end()) {
						// check previous symbol is nonterminal and epsilon in first set of next symbol
						for (list_type::iterator it3 = follow_map[*it1].begin(); it3 != follow_map[*it1].end(); it3++) {
							changed = changed || store(follow_map[previous_symbol], *it3);
						}
					}
					previous_symbol = next_symbol;
				}
				if (find_type(next_symbol) == 1) {	// check if last symbol is nonterminal
					for (list_type::iterator it3 = follow_map[*it1].begin(); it3 != follow_map[*it1].end(); it3++) {
						changed = changed || store(follow_map[next_symbol], *it3);
					}
				}
			}
		}
	} while (changed);	// repeat rule if sets are changed

	return;
}

void find_table() {
	list_type nonterminal_list = symbol_list.getNonterminals();
	list_type terminal_list = symbol_list.getTerminals();

	for (list_type::iterator it1 = nonterminal_list.begin(); it1 != nonterminal_list.end(); it1++) {
		string first_symbol;
		for (list_type::iterator it2 = terminal_list.begin(); it2 != terminal_list.end(); it2++) {
			for (list_type::iterator it3 = production_map[*it1].begin(); it3 != production_map[*it1].end(); it3++) {
				stringstream stream(*it3);
				stream >> first_symbol;

				// rule 1
				if (find(first_map[first_symbol].begin(), first_map[first_symbol].end(), *it2) != first_map[first_symbol].end()) {
					// store if current terminal in first set of nonterminal
					store((parsing_table[*it1])[*it2], *it3);
				}

				// rule 2
				if ((find(first_map[first_symbol].begin(), first_map[first_symbol].end(), "epsilon") != first_map[first_symbol].end() || first_symbol == "epsilon") && find(follow_map[*it1].begin(), follow_map[*it1].end(), *it2) != follow_map[*it1].end()) {
					// store if epsilon in first set of current nonterminal and current terminal in follow set of nonterminal
					store((parsing_table[*it1])[*it2], *it3);
				}
			}
		}
	}
}

void parse() {
	string sentence, symbol;
	cout << "enter sentence to parse:" << endl;
	getline(cin, sentence);
	stringstream stream1(sentence);
	stream1 >> symbol;	// start with first symbol of input

	stack<string> st;
	st.push("Eof");
	st.push("Prog");
	cout << "ilk durum" << endl;

	while (!st.empty()) {
		// check if top of stack is terminal
		if (find_type(st.top()) == 2 || find_type(st.top()) == 0) {
			// rule 1
			if (st.top() == "Eof" && symbol == "Eof") {
				cout << "parsing is successful" << endl;
				return;
			}

			// rule 2
			else {
				cout << "rule 2" << endl;
				st.pop();
				stream1 >> symbol;
			}
		}
		else {
			// rule 3
			if (!(parsing_table[st.top()])[symbol].empty()) {
				cout << "rule 3" << endl;
				string cell_info = (parsing_table[st.top()])[symbol].front();
				stringstream stream2(reverse_string(cell_info));

				st.pop();
				while (stream2 >> cell_info) {
					if (cell_info != "epsilon") {
						st.push(cell_info);
					}
				}
			}

			// error
			else {
				cout << "parsing is not successful" << endl;
				return;
			}
		}
	}
	if (st.empty()) {
		cout << "parsing is not successful" << endl;
	}
}

int find_type(const string &str) {
	list_type nonterminal_list = symbol_list.getNonterminals();
	list_type terminal_list = symbol_list.getTerminals();

	if (find(nonterminal_list.begin(), nonterminal_list.end(), str) != nonterminal_list.end()) {
		return 1;	// nonterminal
	}
	else if (find(terminal_list.begin(), terminal_list.end(), str) != terminal_list.end()) {
		return 2;	// terminal
	}
	else {
		return 0;	// epsilon
	}
}

int store(list_type &container, const string &value) {
	if (find(container.begin(), container.end(), value) != container.end()) {
		// check if value is already stored
		return 0;	// no change
	}
	else {
		container.push_back(value);
		return 1;	// change
	}
}

string reverse_string(const string &str) {
	stringstream stream(str);
	string reversed_string;
	stack<string> st;

	while (stream >> reversed_string) {
		st.push(reversed_string);	// store symbol in stack
	}
	reversed_string.clear();
	while (!st.empty()) {
		reversed_string = reversed_string + ' ' + st.top();
		st.pop();	// pop symbol as reverse order
	}
	return reversed_string;
}

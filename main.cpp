#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <regex>
#include <stack>

using namespace std;
using Cell = pair<string, int>;
using Formula = tuple<Cell, char, Cell>;
enum valueType {column, number, formula};

void get_column_names(ifstream*, vector<string>*);
void parse_lines(ifstream*, vector<string>*, vector<int>*, map<Cell, int>*, map<Cell, Formula>*);
Formula parse_formula(string);
int check_value(string);
void add_to_set(string, set<string>*);
void calculate_values(map<Cell, int>*, map<Cell, Formula>*);
int calculate(int, int, char);
void print_result(map<Cell, int>*, vector<string>*, vector<int>*);


int main(int argc, char* argv[]){
	
	//check if any arguments were written
	if (argc == 1){
		cout << "Select the file to parse"<<endl;
		return 0;
	}

	ifstream f(argv[1]);

	vector<string> columns;
	vector<int> rows;
	map<Cell, int> values;
	map<Cell, Formula> formulas;
	
	if (f.good()){
		try{
			get_column_names(&f,  &columns);
			parse_lines(&f, &columns, &rows, &values, &formulas);
			calculate_values(&values, &formulas);
			print_result(&values, &columns, &rows);
		}catch(char const* s){
			cout << s << endl;
		}
	}

	f.close();

	return 0;
}

void get_column_names(ifstream* f, vector<string>* v){
	/*
	input:
		*f - pointer to the opened file
		*v - pointer to the vector with column names
	description:
		read first line of csv file for getting column names
		if there is no elements or it has wrong format
		throwing error
	*/
	string temp, cols;
	size_t pos;
	bool empty_cell = false;
	set<string> columns;

	getline(*f, cols, '\n');

	if (cols.empty()){
		throw "There is no columns";
	}

	while(pos != string::npos){
		pos = cols.find(",");
		if (pos != string::npos){
			temp = cols.substr(0, pos);
			cols.erase(0, pos+1);

			if(!empty_cell && pos == 0){
				empty_cell = true;
				continue;
			}
		}
		else{
			temp = cols;
		}

		if (check_value(temp)==valueType::column){
			add_to_set(temp, &columns);
			v->push_back(temp);
		}
		else{
			throw "Incorrect column name";
		}
	}
}

void parse_lines(ifstream* f, vector<string>* v_c, vector<int>* v_r, map<Cell, int>* m_v, map<Cell, Formula>* m_f){
	/*
	input:
		*f - pointer to the opened file
		*v - pointer to the vector with column names
		*m_v - pointer to map of cells->values
		*m_f - pointer to map of cells->formulas
	description:
		read csv file to the end and fill the maps with cell address and its value
		if it not match format throwing error
	*/
	string temp, line;
	set<string> rows;

	while(getline(*f, line)){	
		int value, row, count=0;
		bool row_done = false;

		if (line.empty()){
			throw "An empty line";
		}
		size_t pos = line.find(",");

		while(pos != string::npos){
			pos = line.find(",");
			if (count > v_c->size()+1){
				throw "More values in the line than columns";
			}
			
			if (pos != string::npos){
				temp = line.substr(0, pos);
				line.erase(0, pos+1);

				if(!row_done){
					row_done = true;
					if (check_value(temp) != valueType::number){
						throw "Row name is not a number";
					}
					row = stoi(temp);
					if(row<1){
						throw "Row number cant be 0 or negative";
					}

					add_to_set(temp, &rows);
					v_r->push_back(row);

					count++;
					continue;
				}
			}
			else{
				temp = line;
			}

			switch(check_value(temp)){
				case valueType::number :
					value = stoi(temp);
					m_v->insert({{v_c->at(count-1), row}, value});
				break;
				case valueType::formula:
					m_f->insert({{v_c->at(count-1), row}, parse_formula(temp)});
				break;
				default:
					throw "Incorrect value";
				break;
			}
			count++;
		}
		if(count != v_c->size()+1)
			throw "Number of columns and values are different";
	}
}

int check_value(string str){
	/*
	input:
		str - value in string
	output:
		int - type
	description:
		using regular expression determine type of value in the cell
		it can be either number or formula
		otherwise throwing an error
	*/
	regex row("[1-9][0-9]*");
	regex number("-?[1-9][0-9]*|0");
	regex formula("=[a-zA-Z]+[1-9][0-9]*[///+-//*][a-zA-Z]+[1-9][0-9]*");
	regex column("[a-zA-Z]+");

	if (regex_match(str, column)){
		return valueType::column;
	}
	else if(regex_match(str, number)){
		return valueType::number;
	}
	else if(regex_match(str, formula)){
		return valueType::formula;
	}
	else{
		throw "Incorrect value in the table";
	}
}

Formula parse_formula(string str){
	/*
	input:
		str - formula in string
	output:
		Formula - tuple of 2 cells and an operation
	description:
		getting formula in string format
		determine cell parameters and operation
	*/
	int cell1 = 0, cell2, op = 0;
	for (int i=0; i<str.length(); i++){
		if (str[i]=='+' || str[i]=='-' || str[i]=='/' || str[i]=='*'){
			op = i;
		}
		if (str[i]>'0' && str[i]<='9')
		{
			if(cell1==0){
				cell1 = i;
			}
			else if(op!=0){
				cell2 = i;
				break;
			}
		}
	}
	string left_col, right_col;
	int left_row, right_row;

	left_col = str.substr(1, cell1-1);
	right_col = str.substr(op+1, cell2-op-1);
	left_row = stoi(str.substr(cell1, op-cell1));
	right_row = stoi(str.substr(cell2, str.length()-cell2));

	Cell left, right;
	left = make_pair(left_col, left_row);
	right = make_pair(right_col, right_row);

	return make_tuple(left, str[op], right);
}

void add_to_set(string str, set<string>* s){
	auto search = s->find(str);
	if (search != s->end()){
		throw "Value repeats (column or row name)";
	}
	s->insert(str);
}

void calculate_values(map<Cell, int>* m_v, map<Cell, Formula>* m_f){
	char op;
	int res;
	auto it = m_f->begin();
	set<Cell> safe;
	stack<Cell> st;

	while(m_f->size()){
		//search1, search2 - arguments in formula

		if(st.size()){
			it = m_f->find(st.top());
		}

		if(it == m_f->end()){
			it = m_f->begin();	
		}

		auto search1 = m_f->find(get<0>(it->second));
		auto search2 = m_f->find(get<2>(it->second));

		if(search1==m_f->end() && search2==m_f->end()){
			//case with no referencing to other formulas

			auto val1 = m_v->find(get<0>(it->second));
			auto val2 = m_v->find(get<2>(it->second));
			op = get<1>(it->second);

			if(val1==m_v->end() || val2==m_v->end()){
				cout<<it->first.first<<it->first.second<<": ";
				cout<<val1->first.first<<val1->first.second<<op;
				cout<<val2->first.first<<val2->first.second<<endl;
				throw "Incorrect reference in the formula";
			}
			res = calculate(val1->second, val2->second, op);

			if (safe.size()){
				safe.erase(it->first);
				st.pop();
			}

			m_v->insert({it->first, res});
			m_f->erase(it++);
		}
		else if(search1!=m_f->end()){
			if (!safe.insert(search1->first).second){
				cout<<search1->first.first<<search1->first.second<<endl;
				throw "Recursion in the formulas";
			}
			st.push(search1->first);
		}
		else{
			if (!safe.insert(search2->first).second){
				cout<<search2->first.first<<search2->first.second<<endl;
				throw "Recursion in the formulas";
			}
			st.push(search2->first);
		}

	}
}

int calculate(int arg1, int arg2, char op){
	switch(op){
		case '+':
			return arg1+arg2;
		break;
		case '-':
			return arg1-arg2;
		break;
		case '*':
			return arg1*arg2;
		break;
		case '/':
			if (arg2==0){
				throw "division by 0";
			}
			return arg1/arg2;
		break;
		default:
			throw "incorrect operation";
		break;
	}
}

void print_result(map<Cell, int>* m, vector<string>* v_c, vector<int>* v_r){
	for(auto it=v_c->begin();it!=v_c->end();it++){
		cout<<","<<*it;
	}
	cout << endl;
	for(auto i=v_r->begin();i!=v_r->end();i++){
		cout<<*i;
		for(auto j=v_c->begin();j!=v_c->end();j++){
			cout<<","<< m->find({*j, *i})->second;
		}		
		cout<<endl;
	}
}
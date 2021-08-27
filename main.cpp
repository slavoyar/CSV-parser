#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <map>
#include <regex>

using namespace std;
using Cell = pair<string, int>;
using Formula = tuple<Cell, char, Cell>;
enum valueType {column, number, formula};

void get_column_names(ifstream*, vector<string>*);
void parse_lines(ifstream*, vector<string>*, map<Cell, int>*, map<Cell, Formula>*);
Formula parse_formula(string);
int check_value(string);
void add_to_set(string, unordered_set<string>*);
void calculate_values(map<Cell, int>*, map<Cell, Formula>*);
void calculate(int, int, char);

int main(int argc, char* argv[]){
	
	//check if any arguments were written
	if (argc == 1){
		cout << "Select the file to parse"<<endl;
		return 0;
	}

	ifstream f(argv[1]);

	vector<string> columns;
	map<Cell, int> values;
	map<Cell, Formula> formulas;
	
	if (f.good()){
		try{
			get_column_names(&f,  &columns);
			parse_lines(&f, &columns, &values, &formulas);
		}catch(char const* s){
			cout << s << endl;
		}
	}

	cout<<"size of table is "<<(values.size()+formulas.size())<<endl;
	/*
	for(auto it=values.begin();it!=values.end();it++){
		cout<<it->first.first<<it->first.second<<": "<<it->second<<endl;
	}
	for(auto it=formulas.begin();it!=formulas.end();it++){
		cout<<it->first.first<<it->first.second<<": =";
		cout<<get<0>(it->second).first<<get<0>(it->second).second;
		cout <<get<1>(it->second);
		cout<<get<2>(it->second).first<<get<2>(it->second).second<<endl;
	}*/

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
	unordered_set<string> columns;

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

void parse_lines(ifstream* f, vector<string>* v, map<Cell, int>* m_v, map<Cell, Formula>* m_f){
	string temp, line;
	unordered_set<string> rows;

	while(getline(*f, line)){	
		int value, row, count=0;
		bool row_done = false;

		if (line.empty()){
			throw "An empty line";
		}
		size_t pos = line.find(",");
		

		while(pos != string::npos){
			pos = line.find(",");
			if (count > v->size()+1){
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
					m_v->insert({{v->at(count-1), row}, value});
				break;
				case valueType::formula:
					m_f->insert({{v->at(count-1), row}, parse_formula(temp)});
				break;
				default:
					throw "Incorrect value";
				break;
			}
			count++;
		}
		if(count != v->size()+1)
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
	int cell1 = 0, cell2, op;
	for (int i=0; i<str.length(); i++){
		if (str[i]=='+' || str[i]=='-' || str[i]=='/' || str[i]=='*'){
			op = i;
		}
		if (str[i]>'0' && str[i]<='9')
		{
			if(cell1==0){
				cell1 = i;
			}else{
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

void add_to_set(string str, unordered_set<string>* s){
	auto search = s->find(str);
	if (search != s->end()){
		throw "Value repeats (column or row name)";
	}
	s->insert(str);
}

void calculate_values(map<Cell, int>* m_v, map<Cell, Formula>* m_f){

}
void calculate(int arg1, int arg2, char op){

}
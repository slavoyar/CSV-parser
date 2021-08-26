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

void get_column_names(ifstream*, vector<string>*);
void parse_lines(ifstream*, map<Cell, int>*, vector<string>*);
void parse_formula(string);
void show_vector(vector<string>*);
void show_map(map<Cell, int>*);

bool check_if_column(string);
bool check_if_number(string);
int check_value(string);

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
			show_vector(&columns);
			parse_lines(&f, &values, &columns);
		}catch(char const* s){
			cout << s << endl;
		}
	}

	cout<<"size of table is "<<values.size()<<endl;
	for(auto it=values.begin();it!=values.end();it++){
		cout<<it->first.first<<it->first.second<<": "<<it->second<<endl;
	}

	f.close();

	parse_formula("=Cell1+Bell2");

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
	unordered_set<string> s;

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

		if (check_if_column(temp) && temp.size()>0){
			auto search = s.find(temp);
			if (search != s.end()){
				throw "Column names repeat";
			}
			s.insert(temp);
			v->push_back(temp);
		}
		else{
			throw "Incorrect column name";
		}
	}
}

bool check_if_column(string str){
	/*
	input:
		str - name of a column in string
	output:
		boolean 
	description:
		getting the name of a column
		if it consists of letters return true
		else false
	*/
	for (auto it=str.begin();it!=str.end();it++){
		if( (*it<'a' || *it>'z') && (*it<'A' || *it>'Z'))
		{
			return 0;
		}
	}
	return 1;
}

void parse_lines(ifstream* f, map<Cell, int>* m, vector<string>* v){
	string temp, line;

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
				throw "More values in a line than columns";
			}
			
			if (pos != string::npos){
				temp = line.substr(0, pos);
				line.erase(0, pos+1);

				if(!row_done){
					row_done = true;
					if (!check_if_number(temp)){
						throw "Row name is not a number";
					}
					row = stoi(temp);
					count++;
					continue;
				}
			}
			else{
				temp = line;
			}

			if (check_if_number(temp)){
				value = stoi(temp);
				//cout<<v->at(count-1)<<row<<": "<<value<<endl;
				m->insert({{v->at(count-1), row}, value});
			}
			else{
				throw "There is incorrect value in the table";
			}
			count++;
		}
		if(count != v->size()+1)
			throw "Number of columns and values are different";
	}
}

bool check_if_number(string str){
	for (auto it = str.begin(); it != str.end(); it++){
		if( *it < '0' || *it > '9')
		{
			return 0;
		}
	}
	return 1;
}

void show_vector(vector<string>* s){
	for (auto it=s->begin();it!=s->end();it++){
		cout<<", "<<*it;
	}
	cout<<endl;
}

int check_value(string str){
	/*
	input:
		str - value in string
	description:
		using regular expression determine type of value in the cell
		it can be either number or formula
		otherwise throwing an error
	*/
	regex number("-?[1-9][0-9]*|0");
	regex formula("=[a-zA-Z]+[1-9][0-9]*[///+-//*][a-zA-Z]+[1-9][0-9]*");

	if (regex_match(str, number)){
		return 1;
	}
	else if(regex_match(str, formula)){
		return 2;
	}
	else{
		throw "Incorrect value in the table";
	}
}

void parse_formula(string str){
	/*
	input:
		str - formula in string
		*m - pointer to map for formulas
	description:
		getting formula in string format
		determine cell parameters and operation
		inserting formula into map with formulas
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

	Formula formula = make_tuple(left, str[op], right);

}

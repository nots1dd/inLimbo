#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP

#include "toml.hpp"
#include <iostream>

using namespace std;

/* If you want to compile and run it for src/dirsort/test.cpp, uncomment the below line: */  
// #define FILENAME "../../src/parser/examples/config.toml"
#define FILENAME "src/parser/examples/config.toml"

auto config = toml::parse_file(FILENAME);

string_view parseField(string parent, string field) {
  return config[parent][field].value_or(""sv);
}

#endif


/*int main()*/
/*{*/
/*  cout << libName << endl << libPath << endl;*/
/**/
/*  return 0;*/
/*}*/


#include "toml.hpp"
#include <iostream>

using namespace std;

#define FILENAME "../../src/parser/examples/config.toml"

auto config = toml::parse_file(FILENAME);

string_view parseField(string parent, string field) {
  return config[parent][field].value_or(""sv);
}


/*int main()*/
/*{*/
/*  cout << libName << endl << libPath << endl;*/
/**/
/*  return 0;*/
/*}*/


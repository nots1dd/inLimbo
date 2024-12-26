#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP

#include "toml.hpp"
#include <iostream>

using namespace std;

// [TODO]: FIX THIS ISSUE OF COMPILATION ISSUES BY SETTING UP A LOCAL CONFIG.TOML $HOME/.config/inLimbo/ directory
/* If you want to compile and run it for src/dirsort/test.cpp, uncomment the below line: */  
#define FILENAME "../../src/parser/examples/config.toml"
// #define FILENAME "src/parser/examples/config.toml"

/* CURRENT PARENTS & FIELDS MACROS DEFINION */ 

// Section of config.toml with [library] -> parent with name library (idk what it is called)
#define PARENT_LIB "library"

/* PARENT_LIB's FIELDS or children */ 
#define PARENT_LIB_FIELD_NAME "name"
#define PARENT_LIB_FIELD_DIR "directory"

// FTP parent 
#define PARENT_FTP "ftp"

/* PARENT_FTP's FIELDS */
#define PARENT_FTP_FIELD_USER "username"
#define PARENT_FTP_FIELD_SALT "salt"
#define PARENT_FTP_FIELD_PWD_HASH "password_hash"

// DEBUG parent 
#define PARENT_DBG "debug"

/* PARENT_DBG's FIELDS */ 
#define PARENT_DBG_FIELD_PARSER_LOG "parser_log"

auto config = toml::parse_file(FILENAME);

string_view parseTOMLField(string parent, string field) {
  return config[parent][field].value_or(""sv);
}

#endif


/*int main()*/
/*{*/
/*  cout << libName << endl << libPath << endl;*/
/**/
/*  return 0;*/
/*}*/


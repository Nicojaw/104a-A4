//Nico Williams and Brandon Rullamas
//nijowill and brullama
//Assignment 4 - Symbols and Type Checking

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

class SymbolTable {

  int number;
  SymbolTable* parent;
  map<string,string> mapping;
  map<string,int> filenrs;
  map<string,int> linenrs;
  map<string,int> offsets;
  map<string,SymbolTable*> structs;
  map<string,SymbolTable*> subscopes;

public:

  SymbolTable(SymbolTable* parent);
  SymbolTable* enterBlock();
  SymbolTable* enterFunction(string name,
                             string signature, int filenr, int linenr, int offset);
  
  void addSymbol(string name, string type, int filenr, int linenr, int offset);

  void addStruct(string name);

  void dump(FILE* symfile, int depth);

  string lookup(string name);

  SymbolTable* lookup2(string name);


  string parentFunction(SymbolTable* innerScope);

  static int N;

  static vector<string> parseSignature(string signature);
};

#endif
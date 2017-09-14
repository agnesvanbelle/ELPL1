/*
 * File:   Grammar.h
 * Author: agnes
 *
 * Created on November 20, 2012, 6:15 PM
 */

#ifndef GRAMMAR_H
#define	GRAMMAR_H

#include <iostream>
#include <cstdio>
#include <utility>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <stack>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <cassert>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp> 
#include <boost/serialization/map.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;


class Grammar {
  
public:
  /* attributes and other stuff */
  typedef pair <string, int> stringAndInt;
  typedef pair <string, double> stringAndDouble;
  typedef pair <string, stringAndDouble> tableKeyAndValue;

  // a nonterminal specifier is added to all nonterminal symbols,
  // so we can distinguish between terminals and nonterminals
  // (is convenient for in CYKParser)
  static const string nonTerminalSymbol; /* nt_ */
  static const string specialUnarySymbol; /* %%%%% */
  static const string numberRHS; /* "<[(number)]>" */

  /* constructors */
  Grammar(string treeBankFile, bool smooth = true);
  Grammar(const Grammar& orig);
  virtual ~Grammar();

  /* methods */
  void getRHSs(string LHS, vector<stringAndDouble>&);
  void getLHSs(string RHS, vector<stringAndDouble>&, bool RHSisTerminal = false, bool RHSisFirstTerminal = false);

  void printL2rTable();
  void printR2lTable();
  void printUnknownProbTable();
  void init(bool print = true);

private:
  /* attributes and other stuff */
  typedef map<string, double> unknownProbLHS;

  const int capitalChoices; // = 3
  const int suffixChoices; //= 5;
  const int hyphenChoices; //= 2;

  bool smoothing;
  string archiveNameTreebank;
  string archiveNameProbTable;
  string treeBankFileName;

  multimap<string, stringAndDouble> l2rTable;
  multimap<string, stringAndDouble> r2lTable; // inverse ordering from what is "natural"
  pair<multimap<string, stringAndDouble>::iterator, multimap<string, stringAndDouble>::iterator> ruleRangeIterator;
  multimap<string, stringAndDouble>::iterator ruleIterator;

  map<string, int> lhsCountTable;
  map<string,int>::iterator lhsCountTableIterator;  
  
  unknownProbLHS::iterator unknownProbLHSiterator;
  unknownProbLHS * * * unknownProbTable;

  /* methods */
  void readGrammar(bool print);
  void processLine(string line);
  void processLineRecursively(const char * line, int linePos, stack <stringAndInt>, int level, bool firstTerm);
  void insertL2rTable(string key, string valueString);
  bool validCharacter(char nextChar);
  bool isNumber(string term);
  bool isWord(string term); 

  int getCapitalChoicesNumber(string term, bool firstTerm);
  int getSuffixChoicesNumber(string term);
  int getHyphenChoicesNumber(string term);
  void fillUnknownProbTableCount(string term);
  void insertUnknownProbTable(string nonTerm, string term, bool firstTerm);
  void insertNonTermUnknownProbTable(string nonTerm);
  void unknownProbTableCountToProbability();
  void l2rTableCountToProbability();
  void fillR2lTableFromL2rTable();

  void saveUnknownProbTable();
  void loadUnknownProbTable();
  void saveTreebankArchive();
  void loadTreebankArchive();
  bool archivesExists();
  
  void getLHSsUnknownTerm(string RHS, vector<stringAndDouble>& LHSs,  bool RHSisFirstTerminal );

};




#endif	/* GRAMMAR_H */


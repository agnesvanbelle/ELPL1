/*
 * File:   CYKParser.h
 * Author: agnes
 *
 * Created on November 24, 2012, 2:02 PM
 */

#ifndef SENTENCEPARSER_H
#define	SENTENCEPARSER_H

using namespace std;

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>

#include "Grammar.h"
#include "tree.hh"
#include <boost/unordered_map.hpp>


class SentenceParser {

  // forward declarations
  struct location;
  struct RHSEntry;

    public:
      /* attributes and other stuff */
      static const char termDelimiter = ' ';  // what seperates the terms/words in a sentence
      static const int  maxTerms = 16;

       /* constructors */
      SentenceParser(Grammar * aGrammar);
      SentenceParser(const SentenceParser &orig);
      virtual ~SentenceParser();

      /* methods */
      bool parseLine(const string givenLine);
      void reset();

      void printCYKTable();
      void printTOPs();
      void writeTOPs(string fileName);

      bool makeDerivationTreeFromCYKTable(tree<string>& myTree, tree<string>::iterator node, location locLHS, string lhsString);
      void makeDerivationTree();
      void makeFailureTree(tree<string>& myTree, tree<string>::iterator node, location locLHS, string lhsString);
      void getDerivationTree(tree<string>&);

    private:
      /* attributes and other stuff */       
      typedef pair<string, location> stringAndLocation;
      typedef boost::unordered_map<string, RHSEntry> tableEntryMap;
      typedef boost::unordered_map<string, RHSEntry>::iterator cellIterator;

      struct location {
        int i;
        int j;
      };
      struct RHSEntry {
        stringAndLocation RHS1;
        stringAndLocation RHS2;
        double prob;
        bool backIsTerminal;
      };
      
      const stringAndLocation emptyRHS;

      Grammar * myCFG;
      
      // will be changed per to-be-parsed line
      string line;
      vector<string> lineTerms;
      int nrTerms;
      tableEntryMap * * CYKTable;
      vector <pair<string, RHSEntry> > allTOPs;
      tree<string> myTree;

      /* methods */
      void splitHelper(const string line, vector<string> &terms);
      vector<string> split(const string line);

      void CYKLine();
      void CYKLineBaseCase() ;
      void CYKLineRecursiveCase();

};


#endif	/* SENTENCEPARSER_H */


/*
 * File:   Parser.h
 * Author: agnes
 *
 * Created on December 14, 2012, 7:24 PM
 */

#ifndef PARSER_H
#define	PARSER_H

#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>

#include "SentenceParser.h"
#include "TreeManager.h"
#include "tree.hh"

class Parser {

public:
   /* attributes and other stuff */
  typedef tree<string> DerivationTree;

  /* constructors */
  Parser(string treebankFileName, string testSentencesFileName, string testSentencesTreesFileName, string outputFileName, bool smoothing = true);
  Parser(const Parser& orig);
  virtual ~Parser();

  /* methods */
  void start();

private:
   /* attributes and other stuff */
  Grammar * grammar;
  SentenceParser * sentenceParser;

  string testSentencesFileName;
  string testSentencesTreesFileName;
  string newTestSentencesTreesFileName;
  string treebankFileName;
  string outputFileName;


  /* methods */
  void parseTestSentences();

  
};

#endif	/* PARSER_H */


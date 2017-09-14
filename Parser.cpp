/*
 * File:   Parser.cpp
 * Author: agnes
 *
 * Created on December 14, 2012, 7:24 PM
 */

#include "Parser.h"


Parser::Parser(string treebankFileName, string testSentencesFileName, string testSentencesTreesFileName, string outputFileName, bool smoothing  /* = true */) {
  this->treebankFileName = treebankFileName;
  this->testSentencesFileName = testSentencesFileName;
  this->outputFileName = outputFileName;
  this->testSentencesTreesFileName = testSentencesTreesFileName;
  newTestSentencesTreesFileName = testSentencesTreesFileName + "new";

  grammar = new Grammar(treebankFileName, smoothing);
  sentenceParser = new SentenceParser(grammar);
}

Parser::Parser(const Parser& orig) {
}

Parser::~Parser() {
}

void Parser::start() {
  grammar->init();
  parseTestSentences();
}

void Parser::parseTestSentences() {
  cout << "Parsing. " << endl;  
  ifstream inputFile(testSentencesFileName.c_str());
  ofstream outputFile(outputFileName.c_str());
  ifstream treeInputFile(testSentencesTreesFileName.c_str());
  ofstream newTestTreeFile(newTestSentencesTreesFileName.c_str());

  string sentence;
  string sentenceTreeTestData;
  DerivationTree sentenceDerivationTree;
  string derivationString;

  if (inputFile && treeInputFile) {
    int numberParsed = 0;
    int numberSkipped = 0;

    while (getline(inputFile, sentence) && getline(treeInputFile, sentenceTreeTestData )) {
      if (!sentence.empty()) {

        if (sentenceParser->parseLine(sentence)) { // if it was within size limit          
          sentenceParser->getDerivationTree(sentenceDerivationTree);
          derivationString = TreeManager::getPennWSJstring(sentenceDerivationTree);
          outputFile << derivationString << endl;
          newTestTreeFile << sentenceTreeTestData << endl;
          numberParsed++;
          cout << "Parsed " << numberParsed << " lines, skipped " << numberSkipped << " (total " << numberParsed+numberSkipped << ") " <<  endl;
        }
        else {
          numberSkipped++;
        }
      }
    }
    inputFile.close();
    treeInputFile.close();
    cout << "Processed " << numberParsed << " sentences." << endl;
    cout << "Skipped " << numberSkipped << " sentences (those had more than " << SentenceParser::maxTerms << " terms)." << endl;
    cout << "The corresponding test sentences file for \"" << outputFileName << "\" is \"" << newTestSentencesTreesFileName << "\". " << endl;
  }
  else {
    cerr << " !!! " << "Unable to open file " << testSentencesFileName << " or " << testSentencesTreesFileName << endl;
    cerr << " Exiting." << endl;
    exit(1);
  }
  
}




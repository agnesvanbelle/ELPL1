/* 
 * File:   main.cpp
 * Author: agnes
 *
 * Created on November 20, 2012, 5:53 PM
 */
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>

#include "Parser.h"

using namespace std;

/*** Main header ***/
class Main {
public:
  void testStuff1();
  void askTreebankFilename();
  void topsTestSentences();
  void assignment2();
  void assignment3();
  void askTestSentencesFilename();
  void askTestSentencesTreesFileName();
  void askSmoothing();

  void setTreebankFilename(const char * name);
  void setTestSentencesFilename(const char * name);
  void setNewTestSentencesTreesFileName(const char * name);
  void setSmoothing(char s);
  void setSmoothing(bool s);
  bool getSmoothing();

private:
  bool smoothing;
  string treebankFilename;
  string testSentencesFilename;
  string testSentencesTreesFilename;
  Grammar * myGrammar;
  SentenceParser * sentenceParser;
  Parser * parser;
};

/*** Main implementation ***/

void Main::setTreebankFilename(const char * name) {
  treebankFilename.assign(name);
}

void Main::setTestSentencesFilename(const char * name) {
  testSentencesFilename.assign(name);
}

void Main::setNewTestSentencesTreesFileName(const char * name) {
  testSentencesTreesFilename.assign(name);
}

void Main::setSmoothing(char s) {
  char smoothingChar = tolower(s);
  smoothing = (smoothingChar == 's');
}

void Main::setSmoothing(bool s) {
  smoothing = s;
}

bool Main::getSmoothing() {
  return smoothing;
}

void Main::topsTestSentences() {
  cout << endl << "will write to toptest.dat" << endl;
  ifstream myfile(testSentencesFilename.c_str());
  string line;

  if (myfile.is_open()) {
    int numberLines = 0;
    while (getline(myfile, line)) {
      if (!line.empty()) {
        sentenceParser->parseLine(line);
        sentenceParser->writeTOPs("toptest.dat");
        numberLines++;
        cout << "processed " << numberLines << " lines from test file" << endl;
      }
      line = "";
    }
    myfile.close();
  }
  else {
    cerr << " !!! " << "Unable to open test sentences file" << endl;
    cerr << " Exiting.";
    exit(1);
  }
}

void Main::assignment2() {
  myGrammar = new Grammar("treebank.dat");
  myGrammar->init();
  sentenceParser = new SentenceParser(myGrammar);
  topsTestSentences();
}

void Main::assignment3() {
  string outputFilename = testSentencesFilename + "_result";
  parser = new Parser(treebankFilename, testSentencesFilename, testSentencesTreesFilename, outputFilename, smoothing);
  parser->start();
}

void Main::askTreebankFilename() {
  cout << "A treebank filename was not specified." << endl;
  cout << "Enter the treebank filename. If you already have an "
    "archived treebank (.xml) file, enter the original treebank "
    "filename too, then the program will open the corresponding archive "
    "(e.g. for \"treebank.dat\" it will open \"treebank.dat_archive\"). "
    << endl;
  do {
    cin >> treebankFilename;
    cin.ignore(100, '\n');
    cin.clear();
  } while (cin.fail());
}

void Main::askTestSentencesFilename() {
  cout << "A filename for the file containing the test sentences was not specified." << endl;
  cout << "Enter the filename. " << endl;
  do {
    cin >> testSentencesFilename;
    cin.ignore(100, '\n');
    cin.clear();
  } while (cin.fail());
}

void Main::askTestSentencesTreesFileName() {
  cout << "A filename for the file containing the correct (gold standard) test sentences derivations (trees in Penn-WSJ format) was not specified." << endl;
  cout << "Enter the filename. " << endl;
  do {
    cin >> testSentencesTreesFilename;
    cin.ignore(100, '\n');
    cin.clear();
  } while (cin.fail());
}

void Main::askSmoothing() {
  char s;
  cout << "The smoothing option (\'s\' or \'n\') has not been given." << endl;
  cout << "Want to smooth? Type \'s\' if so, or any other letter  if not so. " << endl;
  cin >> s;
  setSmoothing(s);
}

int main(int argc, const char * argv[]) {
  Main * main = new Main();

  if (argc == 2 && (strcmp(argv[1], "default") == 0)) { // default parameter options
    main->setTreebankFilename("treebank.dat");
    main->setTestSentencesFilename("testsentences.dat");
    main->setNewTestSentencesTreesFileName("testsentencestrees.dat");
    main->setSmoothing(true);
  }
  else {
    if (argc < 2) {
      main->askTreebankFilename();
    }
    else {
      main->setTreebankFilename(argv[1]);
    }
    if (argc < 3) {
      main->askTestSentencesFilename();
    }
    else {
      main->setTestSentencesFilename(argv[2]);
    }
    if (argc < 4) {
      main->askTestSentencesTreesFileName();
    }
    else {
      main->setNewTestSentencesTreesFileName(argv[3]);
    }
    if (argc < 5) {
      main->askSmoothing();
    }
    else {
      main->setSmoothing(argv[4][0]);
      if (main->getSmoothing()) {
        cout << "Smoothing is on." << endl;
      }
    }
  }
  
  // main->assignment2();
  main->assignment3();


  return 0;
}


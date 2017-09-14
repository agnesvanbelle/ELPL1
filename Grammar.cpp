/*
 * File:   Grammar.cpp
 * Author: agnes
 *
 * Created on November 20, 2012, 6:15 PM
 */

#include "Grammar.h"

const string Grammar::nonTerminalSymbol = "nt_";
const string Grammar::specialUnarySymbol = "%%%%%";
const string Grammar::numberRHS = "<[(number)]>";

/**
 * Constructor
 */
Grammar::Grammar(string treeBankFile, bool smooth /*= true */) : capitalChoices(3), suffixChoices(5), hyphenChoices(2) {
  treeBankFileName = treeBankFile;
  archiveNameTreebank = treeBankFileName + "_archive_treebank.xml";
  archiveNameProbTable = treeBankFileName + "_archive_probtable.dat";
  smoothing = smooth;

  unknownProbTable = new unknownProbLHS**[capitalChoices];
  for (int i = 0; i < capitalChoices; i++) {
    unknownProbTable[i] = new unknownProbLHS*[suffixChoices];
    for (int j = 0; j < suffixChoices; j++) {
      unknownProbTable[i][j] = new unknownProbLHS[hyphenChoices];
    }
  }
}

/**
 * Copy constructor
 * @param orig
 */
Grammar::Grammar(const Grammar& orig) : capitalChoices(3), suffixChoices(5), hyphenChoices(2) {
}

/**
 * Destructor
 */
Grammar::~Grammar() {
  if (unknownProbTable != NULL) {
    for (int i = 0; i < capitalChoices; i++) {
      for (int j = 0; j < suffixChoices; j++) {
        for (int k = 0; k < hyphenChoices; k++) {
          unknownProbLHS().swap(unknownProbTable[i][j][k]);
        }
        delete[] unknownProbTable[i][j];
      }
      delete[] unknownProbTable[i];
    }
    delete[] unknownProbTable;
  }
  unknownProbTable = NULL;
}

/**
 * Start
 */
void Grammar::init(bool print /* = true */) {
  if (!archivesExists()) {
    cout << "Will read and process treebank file and save the grammar..." << endl;
    readGrammar(print);
    cout << "Nr. rules: " << l2rTable.size() << endl;
    l2rTableCountToProbability();
    unknownProbTableCountToProbability();
    cout << "Writing grammar data to the two archive files \"" << archiveNameTreebank << "\" and \"" << archiveNameProbTable << "\"..." << endl;
    saveTreebankArchive();
    saveUnknownProbTable();
    cout << "Done. " << endl;
    fillR2lTableFromL2rTable();
  }
  else {
    cout << "Will load existing grammar for treebank file from files \"" << archiveNameTreebank << "\" and \"" << archiveNameProbTable << "\"..." << endl;
    loadTreebankArchive();
    loadUnknownProbTable();
    cout << "Done loading." << endl;
    cout << "Nr. rules: " << l2rTable.size() << endl;
    fillR2lTableFromL2rTable();
  }
}

bool Grammar::archivesExists() {
  fstream file1, file2;
  file1.open(archiveNameTreebank.c_str(), ios_base::out | ios_base::in); // will not create file
  file2.open(archiveNameProbTable.c_str(), ios_base::out | ios_base::in);
  if (file1.is_open() && file2.is_open()) {
    file1.close();
    file2.close();
    return true;
  }
  else {
    file1.clear();
    return false;
  }
}

void Grammar::saveTreebankArchive() {  
  ofstream file(archiveNameTreebank.c_str());
  boost::archive::xml_oarchive outputArchive(file);
  // "&" has same effect as "<<" in outputArchive & BOOST_SERIALIZATION_NVP(l2rTable);
  // The macro BOOST_SERIALIZATION_NVP expands to  boost::serialization::make_nvp
  // see http://www.boost.org/doc/libs/1_44_0/libs/serialization/doc/wrappers.html#nvp
  outputArchive & BOOST_SERIALIZATION_NVP(l2rTable);
}

void Grammar::loadTreebankArchive() {
  ifstream file(archiveNameTreebank.c_str());
  boost::archive::xml_iarchive inputArchive(file);
  inputArchive & BOOST_SERIALIZATION_NVP(l2rTable);
}

/**
 * We can't use simple serialization because the array is dynamic but the size of
 * unknownProbTable is relatively small anyway
 */
void Grammar::saveUnknownProbTable() { 
  ofstream file(archiveNameProbTable.c_str());
  for (int i = 0; i < capitalChoices; i++) {
    for (int j = 0; j < suffixChoices; j++) {
      for (int k = 0; k < hyphenChoices; k++) {
        for (unknownProbLHSiterator = unknownProbTable[i][j][k].begin(); unknownProbLHSiterator != unknownProbTable[i][j][k].end(); unknownProbLHSiterator++) {
          file << i << " " << j << " " << k << " " << unknownProbLHSiterator->first << " " << unknownProbLHSiterator->second << endl;
        }
      }
    }
  }
}

void Grammar::loadUnknownProbTable() { 
  ifstream file(archiveNameProbTable.c_str());
  string line;
  istringstream iss;
  int i, j, k;
  string nonTerm;
  double prob;
  if (file) {
      while (getline(file, line)) {
        if (! line.empty()) {
          iss.clear();
          iss.str(line);
          iss >> i; iss >> j; iss >> k;
          iss >> nonTerm;
          iss >> prob;
          unknownProbTable[i][j][k].insert(stringAndDouble(nonTerm, prob));
        }
      }
      file.close();
    }
  else {
    cerr << " !!! " << "Unable to open file " << archiveNameProbTable << endl;
    cerr << " Exiting."<< endl;
    exit(1);
  }
}

/**
 *
 * @param string LHS
 * @return Vector with Pairs denoting <the right hand sides,  their accompanying probability (of LHS -> RHS)>
 *
 * note: not used in practice...
 */
void Grammar::getRHSs(string LHS, vector<stringAndDouble>& RHSs) {
  ruleRangeIterator = l2rTable.equal_range(LHS);

  for (ruleIterator = ruleRangeIterator.first; ruleIterator != ruleRangeIterator.second; ruleIterator++) {
    RHSs.push_back((*ruleIterator).second);
  }
}

/**
 *
 * @param string RHS
 * @return Vector with Pairs denoting <the left hand sides,  their accompanying probability (of LHS -> RHS)>
 *
 */
void Grammar::getLHSs(string RHS, vector<stringAndDouble>& LHSs, bool RHSisTerminal /* = false */, bool RHSisFirstTerminal /* = false */) {
  if (RHSisTerminal && isNumber(RHS)) {// it is a terminal and a number
    RHS = numberRHS;
  }
  ruleRangeIterator = r2lTable.equal_range(RHS);

  for (ruleIterator = ruleRangeIterator.first; ruleIterator != ruleRangeIterator.second; ruleIterator++) {
    LHSs.push_back((*ruleIterator).second);
  }
  if (smoothing) { // if smoothing parameter is set to "true" (see constructor)
    if (RHSisTerminal && LHSs.empty()) { // if it is a terminal but nothing found in r2lTable
      getLHSsUnknownTerm(RHS, LHSs,  RHSisFirstTerminal) ; // get all LHS's from unknownProbTable
    }
  }
}

void Grammar::getLHSsUnknownTerm(string RHS, vector<stringAndDouble>& LHSs,  bool RHSisFirstTerminal) {
  int capitalChoice = getCapitalChoicesNumber(RHS, RHSisFirstTerminal);
  int suffixChoice = getSuffixChoicesNumber(RHS) ;
  int hyphenChoice = getHyphenChoicesNumber(RHS);

  for(unknownProbLHSiterator = unknownProbTable[capitalChoice][suffixChoice][hyphenChoice].begin();
      unknownProbLHSiterator !=  unknownProbTable[capitalChoice][suffixChoice][hyphenChoice].end(); unknownProbLHSiterator++) {
     LHSs.push_back(stringAndDouble(unknownProbLHSiterator->first, unknownProbLHSiterator->second));
  }
}

/**
 * Print all rules in LHS->RHS format
 */
void Grammar::printL2rTable() {
  for (ruleIterator = l2rTable.begin(); ruleIterator != l2rTable.end(); ruleIterator++) {
    printf("%s \t ==>  \t %s  \t (prob/count %f ) \n", (*ruleIterator).first.c_str(), (*ruleIterator).second.first.c_str(), (*ruleIterator).second.second);
  }
}

/**
 * Print all rules in LHS->RHS format
 */
void Grammar::printR2lTable() {
  for (ruleIterator = r2lTable.begin(); ruleIterator != r2lTable.end(); ruleIterator++) {
    printf("%s \t <== \t %s  \t (prob/count %f ) \n", (*ruleIterator).first.c_str(), (*ruleIterator).second.first.c_str(), (*ruleIterator).second.second);
  }
}

/**
 * Transforms counts per rule in l2rTable to probability per rule
 */
void Grammar::l2rTableCountToProbability() {
  for (ruleIterator = l2rTable.begin(); ruleIterator != l2rTable.end(); ruleIterator++) {
    ruleIterator->second.second /= lhsCountTable.find(ruleIterator->first)->second;
     ruleIterator->second.second = log( ruleIterator->second.second);
  }
}

/**
 * Given the LHS->RHS lookup table (l2rTable), fill the table with the RHS->LHS lookup function (r2lTable)
 */
void Grammar::fillR2lTableFromL2rTable() {
  for (ruleIterator = l2rTable.begin(); ruleIterator != l2rTable.end(); ruleIterator++) {
    r2lTable.insert(tableKeyAndValue((*ruleIterator).second.first, stringAndDouble((*ruleIterator).first, (*ruleIterator).second.second)));
  }
}

void Grammar::unknownProbTableCountToProbability() {
   for (int i = 0; i < capitalChoices; i++) {
    for (int j = 0; j < suffixChoices; j++) {
      for (int k = 0; k < hyphenChoices; k++) {
        int totalCount =0;
        for (unknownProbLHSiterator = unknownProbTable[i][j][k].begin(); unknownProbLHSiterator != unknownProbTable[i][j][k].end(); unknownProbLHSiterator++) {          
          totalCount += unknownProbLHSiterator->second;
        }
        for (unknownProbLHSiterator = unknownProbTable[i][j][k].begin(); unknownProbLHSiterator != unknownProbTable[i][j][k].end(); unknownProbLHSiterator++) {
          unknownProbLHSiterator->second /= totalCount;
           unknownProbLHSiterator->second = log( unknownProbLHSiterator->second);
        }
      }
    }
  }
}

void Grammar::printUnknownProbTable() {
  for (int i = 0; i < capitalChoices; i++) {
    for (int j = 0; j < suffixChoices; j++) {
      for (int k = 0; k < hyphenChoices; k++) {

        int nrNonTerms = unknownProbTable[i][j][k].size();
        cout << nrNonTerms << " nonTerms:  ";
        for (unknownProbLHSiterator = unknownProbTable[i][j][k].begin(); unknownProbLHSiterator != unknownProbTable[i][j][k].end(); unknownProbLHSiterator++) {
          cout << unknownProbLHSiterator->first << "(" <<  unknownProbLHSiterator->second << ") ";
        }     
        cout << endl;
      }
      cout << " " << endl;
    }
    cout << "  " << endl;
  }
  cout << endl;
}

int Grammar::getCapitalChoicesNumber(string term, bool firstTerm) {
  if (!isWord(term)) {
    return 2; // caps don't count then
  }
  else if (boost::to_upper_copy(term) == term) { // all caps
    return 1;
  }
  else if ((term[0] == toupper(term[0])) && (!firstTerm)) { // first letter is capital and term was not first word in sentence
    return 0;
  }
  else {
    return 2; // no caps
  }
}

int Grammar::getSuffixChoicesNumber(string term) {
  const string s1 = "ing";
  const string s2 = "ly";
  const string s3a = "es";  const char s3b = 's';
  const string s4 = "ed";
  int termLength = term.length();

  if (term.length() > 3) {
    string termEnd = term.substr(termLength - 3, 3); // size of 3
    if (termEnd == s1) {
      return 0;
    }
    else {
      termEnd = termEnd.substr(1, 2); // size of 2
      if (termEnd == s2) {
        return 1;
      }
      else if ((termEnd[1] == s3b) || (termEnd == s3a)) {
        return 2;
      }
      else if (termEnd == s4) {
        return 3;
      }
    }
  }
  return 4;
}

int Grammar::getHyphenChoicesNumber(string term) {
  const string hyphen = "-";
  size_t found = term.find_first_of(hyphen);

  if (found != string::npos) { // found hyphen
    return 0;
  }
  else { // no hyphen
    return 1;
  }
}

 void Grammar::insertNonTermUnknownProbTable(string nonTerm) {
   for (int i = 0; i < capitalChoices; i++) {
    for (int j = 0; j < suffixChoices; j++) {
      for (int k = 0; k < hyphenChoices; k++) {

        unknownProbLHSiterator = unknownProbTable[i][j][k].find(nonTerm);
        if (unknownProbLHSiterator != unknownProbTable[i][j][k].end()) { // LHS has been added already
          return;
        }
        else {
          unknownProbTable[i][j][k].insert(stringAndDouble(nonTerm, 1)); // add one smoothing
        }
      }
    }
  }
 }
 void Grammar::insertUnknownProbTable(string nonTerm, string term, bool firstTerm) {
   // add nonTerm producing a term always to any category (for smoothing in unknownProbTable)
   insertNonTermUnknownProbTable(nonTerm);
   
   int capitalChoice = getCapitalChoicesNumber(term, firstTerm);
   int suffixChoice = getSuffixChoicesNumber(term) ;
   int hyphenChoice = getHyphenChoicesNumber(term);

   unknownProbLHSiterator = unknownProbTable[capitalChoice][suffixChoice][hyphenChoice].find(nonTerm);
   unknownProbLHSiterator->second++; // increase count

 }

/**
 * Insert a key (string) and value (stringAndDouble) in l2rTable.
 * If the element already exists the double in the stringAndDouble value is increased by one.
 *
 * @param key
 * @param valueString
 */
void Grammar::insertL2rTable(string key, string valueString) {
  ruleRangeIterator = l2rTable.equal_range(key);

  if (ruleRangeIterator.first == ruleRangeIterator.second) { // LHS didnt occur yet
    lhsCountTable.insert(stringAndInt(key, 1)); // keep track of number of rules per LHS
    l2rTable.insert(tableKeyAndValue(key, stringAndDouble(valueString, 1))); // insert rule in l2rTable
  }
  else { // LHS did occur
    lhsCountTableIterator = lhsCountTable.find(key);
    lhsCountTableIterator->second++;

    bool RHSfound = false;
    for (ruleIterator = ruleRangeIterator.first; ruleIterator != ruleRangeIterator.second; ruleIterator++) {
      if (ruleIterator->second.first == valueString) {
        ruleIterator->second.second++;
        RHSfound = true;
        break;
      }
    }
    if (!RHSfound)
      l2rTable.insert(tableKeyAndValue(key, stringAndDouble(valueString, 1)));
  }
}

/**
 * Check if the character is a character we need to process anyway
 * and not e.g. the TAB or DEL character
 *
 * @param nextChar
 * @return
 */
bool Grammar::validCharacter(char nextChar) {
  return !(static_cast<int> (nextChar) < 33 || static_cast<int> (nextChar) > 126);
}

bool Grammar::isNumber(string term) {
  if (!isdigit(term[0])) {
    return false;
  }
  for (int i = 1; i < term.size(); i++) {
    if (!(isdigit(term[i])) && !(term[i] == '.') && !(term[i] == ',')) {
      return false;
    }
  }
  return true;
}

bool Grammar::isWord(string term) {
  int firstLetterInt = static_cast<int>(term[0]);
  if ((firstLetterInt >= 65 && firstLetterInt <= 90)  || (firstLetterInt >= 97 && firstLetterInt <= 122)) {
    return true;
  }
  return false;
}

/**
 * Parse a line, fill l2rTable accordingly
 * Recursive function
 *
 * @param line
 * @param linePos
 * @param stringLevelStack
 * @param level
 */
void Grammar::processLineRecursively(const char * line, int linePos, stack <stringAndInt> stringLevelStack, int level, bool firstTerm) {
  char nextChar = line[linePos];
  while (nextChar == ' ' && linePos < strlen(line)) {
    linePos++;
    nextChar = line[linePos];
  }
  if ((linePos >= strlen(line) - 1) || !validCharacter(nextChar)) // check for e.g. tab-characters in input file
    return;
  else {
    if (nextChar == ')') { // ========> non-terminal rule (RHS end) found
      stringAndInt RHS1, RHS2, LHS;
      RHS2 = stringLevelStack.top();
      stringLevelStack.pop();

      if (stringLevelStack.top().second == RHS2.second) { // if same level
        RHS1 = stringLevelStack.top();
        stringLevelStack.pop();
        RHS1.first += " " + nonTerminalSymbol;
      }
      else {
        RHS1.first = "";
      }
      LHS = stringLevelStack.top(); 
      insertL2rTable(nonTerminalSymbol + LHS.first, nonTerminalSymbol + RHS1.first + RHS2.first);
      return processLineRecursively(line, linePos + 1, stringLevelStack, level - 1, firstTerm);
    }
    if (nextChar != ')' && nextChar != '(') { //  ========> non-terminal LHS found
      string nonTerm = "";
      while (line[linePos] != ' ') {
        nonTerm += line[linePos];
        linePos++;
      }
      stringLevelStack.push(stringAndInt(nonTerm, level));    
      linePos++; // go beyond space
      nextChar = line[linePos];
    }
    if (nextChar == '(') { // ========> begin new rule found (new LHS)
      return processLineRecursively(line, linePos + 1, stringLevelStack, level + 1, firstTerm);
    }
    else { //  ========> terminal symbol + ')' found
      string term = "";
      while (line[linePos] != ')') {
        term += line[linePos];
        linePos++;
      }
      string nonTerm = stringLevelStack.top().first;
      if (isNumber(term)) { // if its number, number-entry
        term = Grammar::numberRHS;
      }
      insertL2rTable(nonTerminalSymbol + nonTerm, term);    
      insertUnknownProbTable(nonTerminalSymbol + nonTerm, term, firstTerm);
      firstTerm = false;
      return processLineRecursively(line, linePos + 1, stringLevelStack, level - 1, firstTerm);
    }
  }
}

/**
 * Helper for parseLineRecursively
 * Parse a line, fill l2rTable accordingly
 *
 * @param line
 */
void Grammar::processLine(string line) {
  stack<stringAndInt> stringLevelStack;
  int level = 0;
  int linePos = 0;
  bool firstTerm = true;
  processLineRecursively(line.c_str(), linePos, stringLevelStack, level, firstTerm);
}

/**
 * Read treebank file
 * parse it, fill l2rTable accordingly
 */
void Grammar::readGrammar(bool print) {  
  ifstream myfile(treeBankFileName.c_str());
  string line;
  if (myfile) {
    int numberLines = 0;
    while (getline(myfile, line)) {
      if (!line.empty()) {
        line.append(" ");
        processLine(line);
        numberLines++;
        if (print && numberLines % 100 == 0)
          cout << "processed " << numberLines << " lines " << endl;
      }
      line = "";
    }
    myfile.close();
  }
  else {
    cerr << " !!! " << "Unable to open treebank file " << treeBankFileName << endl;
    cerr << " Exiting."<< endl;
    exit(1);
  }
}

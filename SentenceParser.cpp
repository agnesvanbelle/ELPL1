
#include "SentenceParser.h"

SentenceParser::SentenceParser(Grammar * aGrammar) : emptyRHS(stringAndLocation({"", {-1, -1}})) {
  myCFG = aGrammar;
  CYKTable = NULL;
}

SentenceParser::SentenceParser(const SentenceParser &orig) : emptyRHS(stringAndLocation({"", {-1, -1}})) {
}

SentenceParser::~SentenceParser() {
  for (int i = 0; i < nrTerms; i++) {
    for (int j = 0; j < nrTerms; j++) {
      tableEntryMap().swap(CYKTable[i][j]); // deletes content from memory
    }
    delete[] CYKTable[i];
  }
  delete[] CYKTable;
}

void SentenceParser::reset() {
  myTree.clear();
  allTOPs.clear();

  if (CYKTable != NULL) {
    for (int i = 0; i < nrTerms; i++) {
      for (int j = 0; j < nrTerms; j++) {
        tableEntryMap().swap(CYKTable[i][j]);
      }
      delete[] CYKTable[i];
    }
    delete[] CYKTable;
  }
  CYKTable = NULL;

  nrTerms = 0;
  lineTerms.clear();
}

bool SentenceParser::parseLine(const string givenLine) {
  reset();
  line = givenLine;
  lineTerms = split(line);
  nrTerms = lineTerms.size();

  if (nrTerms > maxTerms){
    allTOPs.push_back(pair<string, RHSEntry>(Grammar::nonTerminalSymbol+"TOP", RHSEntry({emptyRHS, emptyRHS, 1, false})));
    return false;
  }

  CYKTable = new tableEntryMap*[nrTerms];
  for (int i = 0; i < nrTerms; i++) {
    CYKTable[i] = new tableEntryMap[nrTerms];
  }
  CYKLine();
  return true;
}

void SentenceParser::CYKLine() {
  CYKLineBaseCase();
  if (nrTerms > 1) {
    CYKLineRecursiveCase();
  }
}

void SentenceParser::CYKLineBaseCase() {
  for (int i = 0; i < nrTerms; i++) { // for each terminal
    bool firstTerminal = (i==0);
    vector<Grammar::stringAndDouble> LHSs;

    myCFG->getLHSs(lineTerms[i], LHSs, true, firstTerminal); // true means it's a terminal. get all rules A_j --> terminal_i
    bool sAdded = false;

    int LHSsSize = LHSs.size();
    for (int j = 0; j < LHSsSize; j++) { // for each A_j(LHS), add it to table
      sAdded = true;
      location back1 = (location{i, -1}); // the terminal is the "back"
      CYKTable[i][i][LHSs[j].first] = RHSEntry({stringAndLocation(lineTerms[i], back1), emptyRHS, LHSs[j].second, true}); // true = backIsTerminal
    }
    // handle unaries
    if (sAdded) {
      int k = 0;
      while (k < LHSsSize) { // for each added entry to this cell

        vector<Grammar::stringAndDouble> LHSsRec;
        myCFG->getLHSs(LHSs[k].first, LHSsRec); // get all rules A --> B

        int LHSsRecSize = LHSsRec.size();
        for (int l = 0; l < LHSsRecSize; l++) { // for all A's

          double prob = LHSsRec[l].second + CYKTable[i][i][LHSs[k].first].prob;
     
          location back1 = (location{i, i}); 
          RHSEntry rightEntry =  RHSEntry({stringAndLocation({LHSs[k].first, back1}), emptyRHS, prob, false});
          cellIterator findA_i = CYKTable[i][i].find(LHSsRec[l].first);
          if (findA_i != CYKTable[i][i].end()) {
            if (prob >  findA_i->second.prob) {
              findA_i->second = rightEntry;
              LHSs.push_back(Grammar::stringAndDouble(LHSsRec[l].first, prob));
              LHSsSize++;
            }
          }
          else {
            CYKTable[i][i][LHSsRec[l].first] = rightEntry;
            LHSs.push_back(Grammar::stringAndDouble(LHSsRec[l].first, prob));
            LHSsSize++;
          }
        }
        k++;
      }
    }
  }
}

void SentenceParser::CYKLineRecursiveCase() {
  for (int span = 2; span <= nrTerms; span++) {
    for (int begin = 1; begin <= (nrTerms - span) + 1; begin++) {
      int end = (begin + span) - 1;
      for (int split = begin; split <= (end - 1); split++) {

        // we do these indices minus 1 because or table starts at index 0
        tableEntryMap Bs = CYKTable[begin - 1][split - 1];
        tableEntryMap Cs = CYKTable[split][end - 1];

        // this is needed for recursive case
        bool sAdded = false;

        cellIterator iteratorB;
        cellIterator iteratorC;
        for (iteratorB = Bs.begin(); iteratorB != Bs.end(); iteratorB++) {
          for (iteratorC = Cs.begin(); iteratorC != Cs.end(); iteratorC++) {
            vector<Grammar::stringAndDouble> addedToCell;

            vector<Grammar::stringAndDouble> As;
            string RHS1 = iteratorB->first;
            string RHS2 = iteratorC->first; 
            myCFG->getLHSs(RHS1 + " " + RHS2, As); // get all rules A --> B C
            int AsSize = As.size();

            for (int a_i = 0; a_i < AsSize; a_i++) {
              sAdded = true;
              double prob = As[a_i].second + iteratorB->second.prob + iteratorC->second.prob;

              location locC_i = {split, end - 1};
              location locB_i = {begin - 1, split - 1};
              RHSEntry rightEntry = {stringAndLocation({iteratorB->first, locB_i}), stringAndLocation( {iteratorC->first, locC_i}), prob, false};

              cellIterator findA_i = CYKTable[begin - 1][end - 1].find(As[a_i].first);
              if (findA_i != CYKTable[begin - 1][end - 1].end()) { // element exists already
                if (findA_i->second.prob < prob ) { // if probability is higher
                   findA_i->second = rightEntry; // swap right hand side entry
                   addedToCell.push_back(Grammar::stringAndDouble({As[a_i].first, prob}));
                }
              }
              else {           
                CYKTable[begin - 1][end - 1][As[a_i].first] = rightEntry;
                addedToCell.push_back(Grammar::stringAndDouble({As[a_i].first, prob}));
              }
            }
            // handle unaries
            if ((begin - 1) == 0 && ((end - 1) == (nrTerms - 1)) && sAdded) {
              int k = 0;
              int addedToCellSize = addedToCell.size();
              while (k < addedToCellSize) { // for each added entry to this cell
                vector<Grammar::stringAndDouble> LHSsRec;
                myCFG->getLHSs(addedToCell[k].first, LHSsRec); // get all rules A --> B

                int LHSsRecSize = LHSsRec.size();
                for (int l = 0; l < LHSsRecSize; l++) { // for all A's				         
                  double prob = LHSsRec[l].second + CYKTable[begin - 1][end - 1][addedToCell[k].first].prob;
                  
                  location back1 = (location{begin - 1, end - 1});
                  RHSEntry rightEntry = RHSEntry({stringAndLocation({addedToCell[k].first, back1}), emptyRHS, prob, false});

                  cellIterator findAdded_i = CYKTable[begin - 1][end - 1].find(LHSsRec[l].first);
                  if (findAdded_i != CYKTable[begin - 1][end - 1].end()) { // element exists already
                    if (findAdded_i->second.prob < prob) { // if probability is higher
                      findAdded_i->second = rightEntry; // swap right hand side entry
                      addedToCell.push_back(Grammar::stringAndDouble({LHSsRec[l].first, prob}));
                      addedToCellSize++;
                    }
                  }
                  else {
                    CYKTable[begin - 1][end - 1][LHSsRec[l].first] = rightEntry;
                    addedToCell.push_back(Grammar::stringAndDouble({LHSsRec[l].first, prob}));
                    addedToCellSize++;
                  }
                  if (LHSsRec[l].first == Grammar::nonTerminalSymbol + "TOP") { // keep track of ALL TOPs (part of assignment 2)
                    allTOPs.push_back(pair<string, RHSEntry>(LHSsRec[l].first,rightEntry ));
                  }
                }
                k++;
              }
            }
          }
        }
      }
    }
  }
}

void SentenceParser::printCYKTable() {
  cellIterator iterator;

  cout << "Table: " << endl;
  for (int i = 0; i < nrTerms; i++) {
    for (int j = 0; j < nrTerms; j++) {
      cout << "At cell " << i << ", " << j << ": ";
      for (iterator = CYKTable[i][j].begin(); iterator != CYKTable[i][j].end(); iterator++) {
        if (iterator != CYKTable[i][j].begin()) {
          cout << ", ";
        }
        cout << iterator->first << "(" << iterator->second.prob << ")";
         // print backs
        cout << "(";
        cout << "(" << iterator->second.RHS1.first << " " << iterator->second.RHS1.second.i << " " << iterator->second.RHS1.second.j << ")";
        if (iterator->second.RHS2.first != "") {
          cout << "(" << iterator->second.RHS2.first << " " << iterator->second.RHS2.second.i << " " << iterator->second.RHS2.second.j << ")";
        }
        cout << ")";
      }
      cout << endl;
    }
    cout << endl;
  }

}

void SentenceParser::printTOPs() {
  cout << "TOPs: " << endl;
  int allTOPsSize = allTOPs.size();
  for (int i=0; i< allTOPsSize; i++) {
    cout << allTOPs[i].first << "(" << allTOPs[i].second.prob << ")";
    // print backs
    cout << "(";
    cout << "(" << allTOPs[i].second.RHS1.first << " " << allTOPs[i].second.RHS1.second.i << " " << allTOPs[i].second.RHS1.second.j << ")";
    if ( allTOPs[i].second.RHS2.first != "") {
      cout << "(" << allTOPs[i].second.RHS2.first << " " << allTOPs[i].second.RHS2.second.i << " " << allTOPs[i].second.RHS2.second.j << ")";
    }
    cout << ")" << endl;
  }
}

void SentenceParser::writeTOPs(string fileName) {
  ofstream outputFile;
  outputFile.open(fileName.c_str());

  outputFile << "TOPs for \"" << line << "\"" << endl;
  int allTOPsSize = allTOPs.size();
  for (int i=0; i< allTOPsSize; i++) {
    outputFile << allTOPs[i].first << "(" << allTOPs[i].second.prob << ")";
    // write backs
    outputFile << "(";
    outputFile << "(" << allTOPs[i].second.RHS1.first << " " << allTOPs[i].second.RHS1.second.i << " " << allTOPs[i].second.RHS1.second.j << ")";
    if ( allTOPs[i].second.RHS2.first != "") {
      outputFile << "(" << allTOPs[i].second.RHS2.first << " " << allTOPs[i].second.RHS2.second.i << " " << allTOPs[i].second.RHS2.second.j << ")";
    }
    outputFile << ")" << endl ;
  }
  outputFile << endl;
  outputFile.close();
}

void SentenceParser::splitHelper(const string line, vector<string> &terms) {
  stringstream termStream(line);
  string term;
  while (getline(termStream, term, termDelimiter)) {
    if (!term.empty()) // we don't want the last space
      terms.push_back(term);
  }
}

vector<string> SentenceParser::split(const string line) {
  vector<string> terms;
  splitHelper(line, terms);
  return terms;
}

void SentenceParser::makeFailureTree(tree<string>& myTree, tree<string>::iterator node, location locLHS, string lhsString) {
  for (int i=0; i < nrTerms; i++) {
    tree<string>::iterator node2 = myTree.append_child(node, Grammar::nonTerminalSymbol + "POS");
    myTree.append_child(node2, lineTerms[i]);
  }
}

bool SentenceParser::makeDerivationTreeFromCYKTable(tree<string>& myTree, tree<string>::iterator node, location locLHS, string lhsString){
   RHSEntry thisRHSentry = CYKTable[locLHS.i][locLHS.j][lhsString];
  tree<string>::iterator node2 = myTree.append_child(node, thisRHSentry.RHS1.first);

  if (! thisRHSentry.backIsTerminal){ // stop condition
      makeDerivationTreeFromCYKTable(myTree, node2, thisRHSentry.RHS1.second, thisRHSentry.RHS1.first); // first rhs
  }
  if (!(thisRHSentry.RHS2.first == "")) { // second rhs
    node2 = myTree.append_child(node, thisRHSentry.RHS2.first);
    makeDerivationTreeFromCYKTable(myTree, node2, thisRHSentry.RHS2.second, thisRHSentry.RHS2.first);
  }
}


void SentenceParser::makeDerivationTree() {
  tree<string>::iterator node = myTree.begin();
  location locLHS = {0, nrTerms-1}; // start at upper right corner
  string lhsString = Grammar::nonTerminalSymbol + "TOP" ; // with TOP symbol
  node = myTree.insert(node, lhsString);

  cellIterator findTOP = CYKTable[locLHS.i][locLHS.j].find(lhsString);
  if (findTOP != CYKTable[locLHS.i][locLHS.j].end()) { // if TOP element exists
    makeDerivationTreeFromCYKTable(myTree, node, locLHS, lhsString);
  }
  else {
    makeFailureTree( myTree, node, locLHS, lhsString); // make fake POS-tag tree
  } 
}


void SentenceParser::getDerivationTree(tree<string>& theTree) {
  if (myTree.empty()) {
    makeDerivationTree();
  }
  theTree = myTree;
}
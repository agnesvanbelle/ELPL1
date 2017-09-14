/*
 * File:   TreeManager.cpp
 * Author: agnes
 *
 * Created on November 25, 2012, 8:35 PM
 */

#include "TreeManager.h"

using namespace std;

TreeManager::TreeManager() {  
}

TreeManager::TreeManager(const TreeManager& orig) {
}

TreeManager::~TreeManager() {
}

void TreeManager::printTree(tree<string> myTree) {
  cout << "Tree: " << endl;
  tree<string>::iterator it = myTree.begin();
  tree<string>::iterator end = myTree.end();
  while (it != end) {
    for (int i = 0; i < myTree.depth(it); i++) {
      cout << "    ";
    }
    cout << (*it) << endl;
    it++;
  }
}

string TreeManager::getTreeString(tree<string> myTree) {
  string treeString = "";
  tree<string>::iterator it = myTree.begin();
  tree<string>::iterator end = myTree.end();
  int currentDepth = 0;
  int previousDepth = 0;
  while (it != end) {
    string thing; // thing = terminal or nonterminal
    if (myTree.number_of_children(it) != 0) { // if nonterminal
      treeString += "(";
    }
    if (myTree.number_of_children(it) != 0) { // if nonterminal
      thing = (*it).substr(Grammar::nonTerminalSymbol.length(), (*it).length()); // remove "nt_" prefix
    }
    else {
      thing = (*it);
    }
    treeString += thing;
    if (myTree.number_of_children(it) != 0) { // if nonterminal
      treeString += " ";
    }
    previousDepth = myTree.depth(it);
    it++;
    currentDepth = myTree.depth(it);
    // put enclosing ")"-brackets
    for (int i = 0; i < previousDepth - currentDepth ; i++) {
      if (currentDepth == 0 && i == (previousDepth - currentDepth - 1)) {
        treeString += " ";
      }
      treeString += ")";
    }
    if (previousDepth - currentDepth > 0)
      treeString += " ";
  }
  return treeString;
}

void TreeManager::debinarize(tree<string>& theTree) {
  tree<string>::iterator it = theTree.begin();
  while (it != theTree.end()) {
    if ((*it)[(*it).size() - 1] == '@') {//if '@' is at last in the name
     
      tree<string>::iterator thisAt = it;
      it++;
      tree<string>::iterator leftChild = it; // find kids of @-node
      tree<string>::iterator rightChild = theTree.next_sibling(it);
     
      theTree.move_after(thisAt, rightChild); // make kids siblings of @-node
      theTree.move_before(thisAt, leftChild); 
      tree<string>::iterator newAt = theTree.next_sibling(it);     
      theTree.erase(newAt); // erase @-node
    }
    else {
      it++; // look at next node
    }
  }
}

void TreeManager::removeSpecialUnaryRules(tree<string>& theTree) {
  int sizeSpecialUnarySymbol = Grammar::specialUnarySymbol.size();
 
  tree<string>::iterator it = theTree.begin();  // assume nt_A%%%%%B
  while (it != theTree.end()) {
    int pos = (*it).find(Grammar::specialUnarySymbol); //   position of the first occurrence in the string of the searched content
    if (!(pos == string::npos)) { // found

       // retrieve A and B from nt_A%%%%%B
      string str1 = (*it).substr(0, pos);
      string str2 = Grammar::nonTerminalSymbol + (*it).substr(pos + sizeSpecialUnarySymbol, string::npos);    
      // Make tree with root nt_B. nt_B's children will be nt_A%%%%%B's children.
      tree<string> tree2(it);
      tree<string>::iterator top2 = tree2.begin();
      (*top2) = str2;      
      // Make tree with root nt_A. nt_A's sole child will be nt_B.
      tree<string> tree1;
      tree<string>::iterator top1 = tree1.begin();
      tree<string>::iterator one1 = tree1.insert(top1, str1);
      theTree.append_child(one1, top2);
      theTree.move_ontop(it, one1);

      it = one1; // go to left-most side of %%%%% rule (works for additive %%%%%-rules, e.g. nt_A%%%%%B%%%%%C)   
    }
    else {
      it++;
    }
  }
}

string TreeManager::getPennWSJstring(tree<string> theTree) {
  debinarize(theTree);
  removeSpecialUnaryRules(theTree);
  return getTreeString(theTree);
}





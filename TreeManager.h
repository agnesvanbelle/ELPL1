/* 
 * File:   TreeManager.h
 * Author: agnes
 *
 * Created on November 25, 2012, 8:35 PM
 */

#ifndef TREEMANAGER_H
#define	TREEMANAGER_H

#include <string>
#include <cstring>
#include <iostream>

#include "tree.hh" 
#include "Grammar.h"

using namespace std;

/**
 * This class is for static use only.
 *
 */
class TreeManager {

public:
  /* attributes and other stuff */  

  /* methods */
  static void printTree(tree<string> myTree);
  static string getTreeString(tree<string> myTree);

  static void debinarize(tree<string>& theTree);
  static void removeSpecialUnaryRules(tree<string>& theTree);
  
  static string getPennWSJstring(tree<string> theTree) ;
  
private:

  /* constructors */
  // they are private, we don't one anyone making instances of this class (it's supposed to be a "static" class)
  TreeManager();
  TreeManager(const TreeManager& orig);
  virtual ~TreeManager();
  
  /* attributes and other stuff */
  
  /* methods */

};

#endif	/* TREEMANAGER_H */


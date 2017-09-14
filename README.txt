= General
	The program is written in C++.
	This code has been written for the course Elements of Language Processing and Learning; Master AI; UvA university; 2012.
	Authors: Agnes van Belle ; Norbert Heijne.

= Compiling 

	Remark #1 : the option-std=c++0x or -std=gnu++0x must be turned on in the compiler. 
				This by default the case in the GNU gcc compiler or in its Windows ports, MinGW and TDM-GCC. 
				(This is because of the way we initialize structs.)

	Remark #2 : we make use of the Boost library collection. 
				One Boost library we use, Boost Serialization, is a non-header-only binary, which means it needs 
				to be compiled separately and then linked during compilation.

== Compiling on Linux

		1. 	First acquire the Boost libraries. These are available via the package manager in most distributions 
			(e.g. Fedora, Ubuntu). Make sure to also install the precompiled serialization library (called e.g.libboost-serialization-dev).

		2. 	Then, locate the (precompiled) boost serialization library by typing (without the ">"):
			> locate libboost_serialization
			and copy the location of e.g. libboost_serialization-mt.so

		3. 	Then, issue the compile command by typing the following command in the source directory, the last term should be the location 
			of e.g. libboost_serialization-mt.so that you copied in the previous step:
			> c++ -o project1 Main.cpp Grammar.cpp SentenceParser.cpp TreeManager.cpp Parser.cpp /usr/lib64/libboost_serialization-mt.so


== Compiling on Windows

		This is less simple, and we advise choosing compiling on Linux. We will provide some links, since a full explanation is too long.

		A tutorial focused on building Boosts non-header-only libraries (see step 5) and Eclipse integration for MinGW and Boost can be found here:
		http://theseekersquill.wordpress.com/2010/08/24/howto-boost-mingw/
		 
		A tutorial focused on Netbeans integration using Cygwin and Boost can be found here:
		http://fischerlaender.de/development/using-boost-c-libraries-with-gcc-g-under-windows

		
= Running
	
	Synopsis:
	
	project1 [TREEBANK_FILENAME & TEST_SENTENCES_FILENAME & TEST_SENTENCES_TREES_FILENAME & [ s | n ]]
	
== The program can be run in three different ways, with regard to command line arguments:
	
		1.	No command line arguments. Then it will kindly prompt you for each parameter.
		2. 	One command line argument, the text "default". Then it will use the following default settings:
				- treebank filename: "treebank.dat"
				- test sentences filename : "testsentences.dat"
				- test sentences trees filename : "testsentencestrees.dat"
				- smoothing : yes
		3.	By providing all these arguments in the following order :
				- the treebank file filename
				- the test sentences file filename
				- the test sentences trees file filename
				- the character 's' or 'n' corresponding to smoothing or no smoothing, respectively.

						
= Code structure

== Class structure

=== Main.cpp
			This provides the initialisation, of Parser.cpp mainly. 
			It contains methods that can execute sub-assignment 2 or sub-assignment 3 (sub-assignment 3 is executed in this submission).
			It also processes the command line arguments.
=== Grammar.cpp
			This represents the CFG. 
			It reads the CFG from the treebank and writes it to a XML file, or it reads the CFG from a XML file.
			It holds 2 tables (multimaps) that map the RHS's to the LHS's and vice versa.
			It holds a table (multidimensional) for the smoothing.
			It can write the left-to-right-rule table and the smoothing table to files, and load them from these files.
			Note that as such, two files are used for this serialization process.		
=== SentenceParser.cpp
			This implements the Viterbi-on-CYK algorithm.
			It can produce the derivations for a given sentence.
			Then it can print the CYK table. 
			It can write all the rules TOP-->XP for a sentence to a file (sub-assignment 2), and also
			produce a tree for the Viterbi parse in the CYK Table (sub-assignment 3).
=== TreeManager.cpp
			This is a static class.
			It is used to debinarize the trees, and reparse the %%%%%-unary-rules from the trees.
			It can return the Penn WSJ string for a tree.
=== Parser.cpp
			This class is the main controller.
			It initializes the Grammar, and then reads the test sentences file and parses it by invoking SentenceParser.
			It produces a new test-trees file, that only contains the test-trees of the corresponding test-sentences actually
			parsed (sentences with more than 16 terms are not parsed). 

== General code remarks

	First of all, the multimap "l2rTable" (a LHS-to-RHS table) in Grammar.cpp is largely useless because we only look up the LHS's of rules.
	It said both directions (LHS to RHS, and RHS to LHS) are necessary in part 1 of the assignment, therefore we declared it.
	Currently we first fill l2rTable and calculate the probabilities using l2rTable. According to the final l2rTable, we fill r2lTable.
	However, it would be cleaner to delete l2rTable and rearrange the code accordingly. (It is on the stack, though, so it should not slow the application down.)
	
	A C++ (STL) multimap is used in Grammar.cpp for containing the rule table.
	It maps keys to values, in which keys having multiple values is possible. Therefore we used it for the CFG representation.
	Lookup is by key. It is efficient for lookup by range, which is what we do when getting all keys being equal to e.g. "S".
	
	A C++ (Boost) unordered_map is used in CYKParser.cpp. It is the structure of the cells of the CYK table.
	No duplicate keys are allowed, i.e. a key cannot have multiple values.
	It is efficient for lookup by one key.
	We use it for the table in the CYK algorithm so that we can efficiently get and edit, and easily insert, the LHS of a rule in the cell. 
	
	For generating trees from the CYK table we use the class tree.hh. Credits go to Kasper Peeters. Information is at http://tree.phi-sci.com/documentation.html.
	
		



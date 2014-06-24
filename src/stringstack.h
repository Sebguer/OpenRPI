#pragma once

#include <strings.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <istream>
#include <stdexcept>

class Stringstack {
private:
	std::string argument;
	std::vector<std::string> argumentMemory;
	bool removeSpaces;
	bool finished;

public:
	// Constructors
	Stringstack();

	Stringstack(std::string input);
	Stringstack(const char *input);
	Stringstack(std::istream &input);

	Stringstack(std::string input, bool removeSpaces);
	Stringstack(const char *input, bool removeSpaces);
	Stringstack(std::istream &input, bool removeSpaces);

	/* Assignment operator
	Takes a string and starts the Stringstack anew with it
	removeSpaces remains the same and if true, the new argument is trim'd */
	Stringstack& operator= (const std::string &newString);
	Stringstack& operator= (const char *newString);

	// Comparison operator
	bool operator== (const std::string compare) const;

	/* Argument Memory Access operator
	index < 0 returns the reference for the argument
	-1 < index < memorySize returns the reference to any string in memory
	memorySize < index throws a runtime error, as this is out of bounds access */
	std::string& operator[] (const int index);

	// Static comparison methods for strings
	bool static compare(const std::string compare, const std::string compare2) {
		if ((strcasecmp(compare.c_str(), compare2.c_str())) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	bool static compare(const char *compare, const char *compare2) {
		if ((strcasecmp(compare, compare2)) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	// Removes spaces from the beginning of a string
	std::string static trim(std::string &input) {
		size_t size = input.size();
		size_t spaceCount = 0;
		bool allSpaces = true;

		if (size > 0) {
			std::string::iterator it;
			for(it = input.begin(); it != input.end(); it++, spaceCount++) {
				if (*it != ' ') {
					input = input.substr(spaceCount);
					allSpaces = false;
					break;
				}
			}
			if (allSpaces) {
				input = "";
			}
		}
		return input;
	}

	// Splits the first or next word off argument and stores it in argumentMemory.
	std::string pop();

	/* The same as pop() but is also sensitive to delimits, to handle words or commands with spaces.
	Pass it a string for both beginning and starting delimits, for example "[(" and "])".

	-eraseDelimits: Decides if the delimits are erased from the popped string or not.

	-delimitsMatch: Decides whether the position of the delimit in the firstDelimit string is matched
	by the position of the second delimit in the secondDelimit string. For example, if "[(" and "])" are 
	passed with delimitsMatch set to true, if a [ is located, only its equivalent character in the 
	second string will be used to locate the end of the word - "]".*/
	std::string pop(std::string firstDelimit, std::string secondDelimit);
	std::string pop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits, bool delimitsMatch);

	// Locates a specific string in the string via delimits and pops it.
	std::string findPop(std::string firstDelimit, std::string secondDelimit);
	std::string findPop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits);

	// Form of popDelimited suitable for popping strings containing " and '. Erases and matches delimits.
	std::string popSpeech();

	/* Form of pop to pop any data up until a specified delimit.

	-eraseDelimit: Decides if the delimit is erased from the popped string or not.*/
	std::string popUntil(std::string delimit);
	std::string popUntil(std::string delimit, bool eraseDelimit);

	// Returns whether a specific string is contained in the argument string or not.
	bool find(std::string specifier);

	// Returns the number of instances of the specified string in argument.
	int argCount(std::string specifier);

	// Returns a vector of the strings that are contained within the argument string.
	std::vector<std::string> argBatchFind(const std::vector<std::string> &strings);

	/* Returns a vector of std::pair containing the string and the count number of that string within
	the argument string.*/
	std::vector<std::pair<std::string, int> > argBatchCount(const std::vector<std::string> &strings);


	/* Locates a specific string in the string and replaces it with the replacement string.

	-replaceAll: Decides whether all instances of the specific word are replaced.

	-replaceReverseOrder: Decides whether the string is traversed in order or in reverse.*/
	void argReplace(std::string specifier, std::string replacement);
	void argReplace(std::string specifier, std::string replacement, bool replaceAll, bool replaceReverseOrder);

	// argReplace which never has replaceAll set true.
	void argReplaceOnce(std::string specifier, std::string replacement);
	void argReplaceOnce(std::string specifier, std::string replacement, bool replaceReverseOrder);

	/* Recall past arguments
	Origin at zero */
	std::string recall(size_t indexArg);
	std::string last();

	// Converts the last popped string to an integer using atoi. Returns -1.0 if it fails.
	int toInt();

	// Converts the passed string to an integer using atoi. Returns -1.0 if it fails.
	int toInt(std::string integerString);

	// Converts the indexed string to an integer using atoi. Returns -1.0 if it fails.
	int toInt(size_t indexArg);

	// Converts the last popped string to a double, uses atof. Returns -1.0 if it fails.
	double toDouble();

	// Converts the last popped string to a double, uses atof. Returns -1.0 if it fails.
	double toDouble(std::string doubleString);

	// Converts the indexed string to a double, uses atof. Returns -1.0 if it fails.
	double toDouble(size_t indexArg);

	/* Return portions of the stored argument, either from an argument, between arguments or all of them
	Origin at zero */
	std::string recallPartialString(size_t begin);
	std::string recallPartialString(size_t begin, size_t arguments);
	std::string recallString();

	// Pops the whole input, either with pop or popSpeech
	void popAll();
	void popAllSpeech();

	// Returns if the Stringstack is fully popped
	bool isFinished();

	// Returns the current argument string
	std::string getArg();

	// Returns the current argument string as a constant C string
	const char* getCArg();

	// Returns the input string as a constant C string
	const char* getCArg(const std::string input);
};

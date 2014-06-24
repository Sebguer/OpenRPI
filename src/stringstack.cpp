#include "stringstack.h"

/*********************************************************
*          Stringstack Function Definitions              *
*                   Case                                 *
*********************************************************/
Stringstack::Stringstack() {
	removeSpaces = false;
	finished = false;
}

Stringstack::Stringstack(std::string input) {
	argument = input;
	removeSpaces = true;
	finished = false;

	Stringstack::trim(argument);
}

Stringstack::Stringstack(const char *input) {
	std::string temp = input;

	argument = temp;
	removeSpaces = true;
	finished = false;

	Stringstack::trim(argument);
}

Stringstack::Stringstack(std::istream &input) {
	std::string temp;

	getline(input, temp);
	argument = temp;
	removeSpaces = true;
	finished = false;

	Stringstack::trim(argument);
}


Stringstack::Stringstack(std::string input, bool removeSpaces) {
	argument = input;
	this->removeSpaces = removeSpaces;
	finished = false;

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

Stringstack::Stringstack(const char *input, bool removeSpaces) {
	argument = input;
	this->removeSpaces = removeSpaces;
	finished = false;

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

Stringstack::Stringstack(std::istream &input, bool removeSpaces) {
	std::string temp;

	getline(input, temp);
	argument = temp;
	this->removeSpaces = removeSpaces;
	finished = false;

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

Stringstack& Stringstack::operator= (const std::string &newString) {
	argument = newString;

	argumentMemory.clear();

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
	finished = false;

	return *this;
}

Stringstack& Stringstack::operator= (const char *newString) {
	argument = newString;

	argumentMemory.clear();

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
	finished = false;

	return *this;
}

bool Stringstack::operator== (const std::string compare) const {
	if (!(this->argumentMemory.empty())) {
		if ((strcasecmp(this->argumentMemory.back().c_str(), compare.c_str())) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

std::string& Stringstack::operator[] (const int index) {
	if (index < 0) {
		return argument;
	}
	else if (index < (int)argumentMemory.size()) {
		return argumentMemory[index];
	}
	else {
		throw std::runtime_error("Stringstack [] out of bounds");
	}
}

std::string Stringstack::pop() {
	if (removeSpaces) {
		Stringstack::trim(argument);
	}

	if (finished && isFinished()) { // Keeping the short circuit just in case [] operator is used to set argument to ""
		return "";
	}

	size_t space = argument.find_first_of(' ');

	if (space != std::string::npos) {
		if (argument.size() > space + 1) {
			argumentMemory.push_back(argument.substr(0,space));
			argument = argument.substr(space + 1);
			return argumentMemory.back();
		}
		else {
			argumentMemory.push_back(argument.substr(0,space));
			argument = "";
			finished = true;
			return argumentMemory.back();
		}
	}
	else {
		if (argument != "" || argumentMemory.empty()) {
			argumentMemory.push_back(argument);
		}
		argument = "";
		finished = true;
		return argumentMemory.back();
	}
}

std::string Stringstack::pop(std::string firstDelimit, std::string secondDelimit) {
	return pop(firstDelimit, secondDelimit, true, false);
}

std::string Stringstack::pop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits, bool delimitsMatch) {
	pop();
	std::string resultString = argumentMemory.back();
	size_t size = resultString.size();
	size_t tempPos = resultString.find_first_of(firstDelimit);

	if (tempPos != std::string::npos) {
		if (delimitsMatch) {
			size_t delimitPos = secondDelimit.find_first_of(resultString[tempPos]);
			if (delimitPos == std::string::npos) {
				delimitPos = firstDelimit.find_first_of(resultString[tempPos]);
				if (delimitPos < secondDelimit.size()) {
					secondDelimit = secondDelimit[delimitPos];
				}
				else {
					throw std::runtime_error("Incorrectly matched delimits on call to popDelimited");
					return argumentMemory.back();
				}
			}
			else {
				secondDelimit = resultString[tempPos];
			}
		}
		if (eraseDelimits) {
			resultString.erase(tempPos,1);
		}
		tempPos = resultString.find_last_of(secondDelimit);
		if (tempPos != std::string::npos) {
			if (eraseDelimits) {
				resultString.erase(tempPos,1);
			}
		}
		else {
			tempPos = argument.find_first_of(secondDelimit);
			if (!eraseDelimits) {
				tempPos++;
			}
			if (tempPos != std::string::npos) {
				resultString.append(" " + argument.substr(0, tempPos));
				size = argument.size();

				if (tempPos >= size) {
					argument = "";
				}
				else {
					argument = argument.substr(tempPos + 1);
				}
			}
			else {
				return argumentMemory.back();
			}
		}
	}
	argumentMemory.back() = resultString;
	return resultString;
}

std::string Stringstack::findPop(std::string firstDelimit, std::string secondDelimit) {
	return findPop(firstDelimit, secondDelimit, true);
}

std::string Stringstack::findPop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits) {
	size_t firstPos = argument.find(firstDelimit);
	size_t secondPos = argument.find(secondDelimit, firstPos);

	if ((firstPos != std::string::npos) && (secondPos != std::string::npos)) {
		argumentMemory.push_back(argument.substr((firstPos + 1), (secondPos - firstPos - 1)));
		if (eraseDelimits) {
			argument.erase(firstPos, argumentMemory.back().size() + 2);
		}
		else {
			argument.erase(firstPos + 1, argumentMemory.back().size() + 1);
		}
		return argumentMemory.back();
	}
	return "";
}

std::string Stringstack::popSpeech() {
	return pop("\"'", "\"'", true, true);
}

std::string Stringstack::popUntil(std::string delimit) {
	return popUntil(delimit, true);
}

std::string Stringstack::popUntil(std::string delimit, bool eraseDelimit) {
	if (removeSpaces) {
		Stringstack::trim(argument);
	}
	size_t tempPos = argument.find(delimit);
	size_t delimitWidth = delimit.size();

	if (delimitWidth == 0) {
		return "";
	}

	if (tempPos != std::string::npos && tempPos > 0) {
		if (eraseDelimit) {
			argumentMemory.push_back(argument.substr(0, tempPos));

			if (tempPos < argument.size() - delimitWidth) {
				argument = argument.substr(tempPos + delimitWidth);
			}
			else {
				argument = "";
				finished = true;
			}
		}
		else {
			if (tempPos < argument.size()) {
				argumentMemory.push_back(argument.substr(0, tempPos + delimitWidth));
				argument = argument.substr(tempPos + delimitWidth);
			}
			else {
				return pop();
			}
		}
	}
	else if (tempPos == 0) {
		if (eraseDelimit) {
			if (argument.size() > delimitWidth) {
				argument = argument.substr(delimitWidth);
				return "";
			}
			else {
				argument = "";
				finished = true;
				return argument;
			}
		}
		else {
			argumentMemory.push_back(argument.substr(0, delimitWidth));
			argument = argument.substr(delimitWidth);
		}
	}
	else {
		return "";
	}
	return argumentMemory.back();
}

bool Stringstack::find(std::string specifier) {
	return (argument.find(specifier) != std::string::npos);
}

int Stringstack::argCount(std::string specifier) {
	int output = 0;
	size_t position = argument.find(specifier);

	while (position != std::string::npos) {
		output++;
		position = argument.find(specifier, position);
	}
	return output;
}

std::vector<std::string> Stringstack::argBatchFind(const std::vector<std::string> &strings) {
	std::vector<std::string> output;

	if (strings.size() > 0) {
		size_t stringsSize = strings.size();

		for (size_t i = 0; i < stringsSize; i++) {
			if (find(strings[i])) {
				output.push_back(strings[i]);
			}
		}
	}
	return output;
}

std::vector<std::pair<std::string, int> > Stringstack::argBatchCount(const std::vector<std::string> &strings) {
	std::vector<std::pair<std::string, int> > output;

	if (strings.size() > 0) {
		size_t stringsSize = strings.size();
		int tempCount = 0;

		for (size_t i = 0; i < stringsSize; i++) {
			tempCount = argCount(strings[i]);
			output.push_back(std::pair<std::string, int>(strings[i], tempCount));
		}
	}
	return output;
}

void Stringstack::argReplace(std::string specifier, std::string replacement) {
	argReplace(specifier, replacement, true, false);
}

void Stringstack::argReplace(std::string specifier, std::string replacement, bool replaceAll, bool replaceReverseOrder) {
	size_t specifierPos = 0;
	size_t specifierSize = specifier.size();
	size_t replacementSize = replacement.size();
	bool finished = false;

	if (replaceReverseOrder) {
		specifierPos = (argument.size() - 1);
	}

	do {
		if (replaceReverseOrder) {
			specifierPos = argument.rfind(specifier, specifierPos);
		}
		else {
			specifierPos = argument.find(specifier, specifierPos);
		}

		if (specifierPos == std::string::npos) {
			break;
		}

		argument.replace(specifierPos, specifierSize, replacement);

		if (replaceReverseOrder) {
			specifierPos--;
		}
		else {
			specifierPos += replacementSize;	
		}
	}while (replaceAll);
}

void Stringstack::argReplaceOnce(std::string specifier, std::string replacement) {
	argReplace(specifier, replacement, false, false);
}

void Stringstack::argReplaceOnce(std::string specifier, std::string replacement, bool replaceReverseOrder) {
	argReplace(specifier, replacement, false, replaceReverseOrder);
}

std::string Stringstack::recall(size_t indexString) {
	if (indexString < argumentMemory.size()) {
		return argumentMemory[indexString];
	}
	else {
		return "";
	}
}

std::string Stringstack::last() {
	if (argumentMemory.size() > 0) {
		return argumentMemory.back();
	}
	else {
		return "";
	}
}

int Stringstack::toInt() {
	if (argumentMemory.size() > 0 && argumentMemory.back().size() > 0) {
		return atoi(argumentMemory.back().c_str());
	}
	else {
		return -1;
	}
}

int Stringstack::toInt(std::string integerString) {
	return atoi(integerString.c_str());
}

int Stringstack::toInt(size_t indexString) {
	return atoi((recall(indexString)).c_str());
}

double Stringstack::toDouble() {
	if (argumentMemory.size() > 0 && argumentMemory.back().size() > 0) {
		return atof(argumentMemory.back().c_str());
	}
	else {
		return -1.0;
	}
}

double Stringstack::toDouble(std::string doubleString) {
	return atof(doubleString.c_str());
}

double Stringstack::toDouble(size_t indexString) {
	return atof((recall(indexString)).c_str());
}

std::string Stringstack::recallPartialString(size_t begin) {
	size_t size = argumentMemory.size();
	std::stringstream toReturn;

	if (begin >= size) {
		return "ERROR: No Argument There At That Position";
	}
	else {
		for (size_t i = begin; i <= size; i++) {
			toReturn << recall(i);
			if (i < (size - 1)) {
				toReturn << " ";
			}
		}
	}
	return toReturn.str();
}

std::string Stringstack::recallPartialString(size_t begin, size_t arguments) {
	size_t size = argumentMemory.size();
	std::stringstream toReturn;

	if (begin >= size) {
		return recallPartialString(0);
	}
	else {
		for (size_t i = begin; i < size && arguments; i++) {
			toReturn << recall(i);
			if (i < (size - 1) && i < arguments) {
				toReturn << " ";
			}
			arguments--;
		}
	}
	return toReturn.str();
}

std::string Stringstack::recallString() {
	return recallPartialString(0);
}

void Stringstack::popAll() {
	while (!finished) {
		pop();
	}
}

void Stringstack::popAllSpeech() {
	while (!finished) {
		popSpeech();
	}
}

bool Stringstack::isFinished() {
	finished = (argument == "");
	return finished;
}

std::string Stringstack::getArg() {
	return argument;
}

const char* Stringstack::getCArg() {
	return (argument.c_str());
}

const char* Stringstack::getCArg(const std::string input) {
	return (input.c_str());
}

/**
 * @file	MetaObj.h
 * @brief	Definition file for MetaObj class
 * @author	Wei Tong
 * @details Specifies all members of MetaObj class
 * @version	1.00
 * 			Wei Tong (7 February 2018)
 *			Initial development, functions will possibly
 *			be used in the future
 */

#include <string>

class MetaObj{
private:
	char metaCode;
	std::string metaDescription;
	int metaCycles;
public:
	MetaObj();																// Default constructor
	~MetaObj();																// Default deconstructor
	MetaObj(char inputCode, std::string inputDescription, int inputCycles);	// Parameterized constructor
	bool setCode(char);														// Sets the code for the meta-data
	char getCode();															// Retrieves the code for the meta-data
	bool setDescription(std::string);										// Sets the description for the meta-data
	std::string getDescription();											// Retrieves the description for the meta-data
	bool setCycles(int);													// Sets the number of cycles
	int getCycles();														// Retrieves the number of cycles
};
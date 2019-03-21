/**
 * @file	MetaObj.cpp
 * @brief	Implementation of MetaObj class
 * @author	Wei Tong
 * @details All member functions of MetaObj are implemented
 * @version	1.00
 * 			Wei Tong (7 February 2018)
 *			Initial development, functions will possibly
 *			be used in the future
 * @note	Requires MetaObj.h
 */

#include "MetaObj.h"

// Default constructor, sets initial values
MetaObj::MetaObj(){
	metaCode = ' ';
	metaDescription = "";
	metaCycles = -1;
}

// Deconstructor, nothing here yet
MetaObj::~MetaObj(){

}

// Parameterized constructor, sets specified values
// Should NEVER be used, unless input is guaranteed correct
MetaObj::MetaObj(char inptCode, std::string inptDescription, int inptCycles){
	metaCode = inptCode;
	metaDescription = inptDescription;
	metaCycles = inptCycles;
}

// Don't really need, but just in case
bool MetaObj::setCode(char inptCode){
	if(inptCode == 'S' || inptCode == 'A' || inptCode == 'P' || inptCode == 'I' || inptCode == 'O' || inptCode == 'M'){
		metaCode = inptCode;
		return true;
	}
	else{
		return false;
	}
}

// Function to get meta-data code
char MetaObj::getCode(){
	return metaCode;
}

// Don't really need, but just in case
bool MetaObj::setDescription(std::string inptDescription){
	if(!inptDescription.compare("begin") || !inptDescription.compare("finish") || !inptDescription.compare("hard drive") ||
		!inptDescription.compare("keyboard") || !inptDescription.compare("scanner") || !inptDescription.compare("monitor") ||
		!inptDescription.compare("run") || !inptDescription.compare("allocate") || !inptDescription.compare("projector") || 
		!inptDescription.compare("block")){

		metaDescription = inptDescription;
		return true;
	}
	else{
		return false;
	}
}

// Function to get meta-data description
std::string MetaObj::getDescription(){
	return metaDescription;
}

// Don't really need, but just in case
bool MetaObj::setCycles(int inptCycles){
	if(inptCycles >= 0){
		metaCycles = inptCycles;
		return true;
	}
	else{
		return false;
	}
}

// Function to get meta-data cycle amount
int MetaObj::getCycles(){
	return metaCycles;
}
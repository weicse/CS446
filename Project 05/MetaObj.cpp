/**
 * @file	MetaObj.cpp
 * @brief	Implementation of MetaObj class
 * @author	Wei Tong
 * @details All member functions of MetaObj are implemented
 * @version	2.0
 * 			Wei Tong (28 February 2018)
 *			Fixed setting description to work with new implementation
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
	if(!inptDescription.compare("begin") || !inptDescription.compare("finish") || !inptDescription.compare("harddrive") ||
		!inptDescription.compare("keyboard") || !inptDescription.compare("scanner") || !inptDescription.compare("monitor") ||
		!inptDescription.compare("run") || !inptDescription.compare("allocate") || !inptDescription.compare("projector") || 
		!inptDescription.compare("block")){

		// v2.0 changed description from "hard drive" to "harddrive" to work with new implementation
		if(!inptDescription.compare("harddrive")){
			metaDescription = "hard drive";
		}
		else{
			metaDescription = inptDescription;
		}
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
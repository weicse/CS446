/**
 * @file	ConfData.cpp
 * @brief	Implementation of ConfData class
 * @author	Wei Tong
 * @details All members of ConfData are implemented
 * @version	4.0
 * 			Wei Tong (18 April 2018)
 *			Added new value and functions to understand
 *			processor quantum number and scheduling
 *			algorithm in new configuration files
 * @note	Requires ConfData.h
 */

#include "ConfData.h"

// Default constructor, sets initial values
ConfData::ConfData(){
	version = -1;
	filePath = "";
	monitorTime = -1;
	processorTime = -1;
	scannerTime = -1;
	hardDriveTime = -1;
	keyboardTime = -1;
	memoryTime = -1;
	projectorTime = -1;
	logLevel = -1;
	logPath = "";

	// v2.0
	maxMem = -1;

	// v3.0
	numProj = -1;
	numHDD = -1;
	memBlockSize = -1;

	// v4.0
	pqn = -1;
	sch_type = "";
}

// Default deconstructor, nothing to deallocate
ConfData::~ConfData(){

}

void ConfData::setVer(float inptVersion){
	version = inptVersion;
}

float ConfData::getVer(){
	return version;
}

void ConfData::setFilePath(std::string inptFP){
	filePath = inptFP;
}

std::string ConfData::getFilePath(){
	return filePath;
}

void ConfData::setLogPath(std::string inptLP){
	logPath = inptLP;
}

std::string ConfData::getLogPath(){
	return logPath;
}

void ConfData::setLogLvl(int inptLvl){
	logLevel = inptLvl;
}

int ConfData::getLogLvl(){
	return logLevel;
}

bool ConfData::setCycleTime(std::string inptName, int inptTime){
	if(!inptName.compare("Monitor"))
		monitorTime = inptTime;
	else if(!inptName.compare("Processor"))
		processorTime = inptTime;
	else if(!inptName.compare("Scanner"))
		scannerTime = inptTime;
	else if(!inptName.compare("Hard Drive"))
		hardDriveTime = inptTime;
	else if(!inptName.compare("Keyboard"))
		keyboardTime = inptTime;
	else if(!inptName.compare("Memory"))
		memoryTime = inptTime;
	else if(!inptName.compare("Projector"))
		projectorTime = inptTime;
	else
		return false;
	return true;
}

int ConfData::getCycleTime(std::string inptName){
	if(!inptName.compare("Monitor"))
		return monitorTime;
	else if(!inptName.compare("Processor"))
		return processorTime;
	else if(!inptName.compare("Scanner"))
		return scannerTime;
	else if(!inptName.compare("Hard Drive"))
		return hardDriveTime;
	else if(!inptName.compare("Keyboard"))
		return keyboardTime;
	else if(!inptName.compare("Memory"))
		return memoryTime;
	else if(!inptName.compare("Projector"))
		return projectorTime;
	else
		return -1;
}

int ConfData::readLine(std::string inptLine){

	// used to store a number value
	float inptNum;

	// used to store a string value
	std::string convertText;

	// Check for no semicolon, return 1 if not found
	std::size_t checkColon = inptLine.find_first_of(":");
	if(checkColon == std::string::npos){
		if(!inptLine.compare("Start Simulator Configuration File") || !inptLine.compare("End Simulator Configuration File")){
			return 0;		// Ignore beginning and end of config file
		}
		else
			return 1;		// No semicolon found
	}

	// Make string of what is being input
	std::string cycleType = inptLine.substr(0, inptLine.find(": "));

	// Extract the info after :
	// This is when a number cycle time is not inputted
	// Such as log path
	if(!cycleType.compare("File Path")){
		convertText = inptLine.substr(inptLine.find_first_of(":"));
		convertText.erase(0,2);	// Remove the ": "
		filePath = convertText;
		// std::cout << "Read in file path is: " << filePath << std::endl;
		return 0;
	}
	else if(!cycleType.compare("Log File Path")){
		convertText = inptLine.substr(inptLine.find_first_of(":"));
		convertText.erase(0,2);	// Remove the ": "
		logPath = convertText;
		// std::cout << "Read in log file path is: " << logPath << std::endl;
		return 0;
	}
	else if(inptLine.find_first_of("1234567890") == std::string::npos){
		convertText = inptLine.substr(inptLine.find_first_of(":"));
		convertText.erase(0,2);	// Remove the ": "
	}

	// This is when a cycle time is inputted
	else{
		std::string convertNum = inptLine.substr(inptLine.find_first_of("1234567890"));
		std::istringstream buffer(convertNum);
		buffer >> inptNum;
	}

	// Set number to correct type
	if(!cycleType.compare("Version/Phase")){
		version = inptNum;
		// std::cout << "version: " << version << std::endl;
	}
	else if(!cycleType.compare("Monitor display time {msec}")){
		monitorTime = inptNum;
		// std::cout << "monitor: " << monitorTime << std::endl;
	}
	else if(!cycleType.compare("Processor cycle time {msec}")){
		processorTime = inptNum;
		// std::cout << "processor: " << processorTime << std::endl;
	}
	else if(!cycleType.compare("Scanner cycle time {msec}")){
		scannerTime = inptNum;
		// std::cout << "scanner: " << scannerTime << std::endl;
	}
	else if(!cycleType.compare("Hard drive cycle time {msec}")){
		hardDriveTime = inptNum;
		// std::cout << "hd: " << hardDriveTime << std::endl;
	}
	else if(!cycleType.compare("Keyboard cycle time {msec}")){
		keyboardTime = inptNum;
		// std::cout << "keyboard: " << keyboardTime << std::endl;
	}
	else if(!cycleType.compare("Memory cycle time {msec}")){
		memoryTime = inptNum;
		// std::cout << "memory: " << memoryTime << std::endl;
	}
	else if(!cycleType.compare("Projector cycle time {msec}")){
		projectorTime = inptNum;
		// std::cout << "projector: " << projectorTime << std::endl;
	}
	else if(!cycleType.compare("Log")){
		if(!convertText.compare("Log to Monitor"))
			logLevel = Monitor;
		else if(!convertText.compare("Log to File"))
			logLevel = File;
		else if(!convertText.compare("Log to Both"))
			logLevel = Both;
		// std::cout << "Log level: " << logLevel << std::endl;
	}

	// v2.0
	else if(!cycleType.compare("System memory {kbytes}")){
		maxMem = inptNum;
		// std::cout << "Maximum memory: " << maxMem << std::endl;
	}
	else if(!cycleType.compare("System memory {Mbytes}")){
		maxMem = inptNum * 1024;
		// std::cout << "Maximum memory: " << maxMem << std::endl;
	}
	else if(!cycleType.compare("System memory {Gbytes}")){
		maxMem = inptNum * 1024 * 1024;
		// std::cout << "Maximum memory: " << maxMem << std::endl;
	}

	// v3.0
	else if(!cycleType.compare("Projector quantity")){
		numProj = inptNum;
	}
	else if(!cycleType.compare("Hard drive quantity")){
		numHDD = inptNum;
	}
	else if(!cycleType.compare("Memory block size {kbytes}")){
		memBlockSize = inptNum;
	}
	else if(!cycleType.compare("Memory block size {Mbytes}")){
		memBlockSize = inptNum * 1024;
	}
	else if(!cycleType.compare("Memory block size {Gbytes}")){
		memBlockSize = inptNum * 1024 * 1024;
	}

	// v4.0
	else if(!cycleType.compare("Processor Quantum Number")){
		pqn = inptNum;
	}

	else if(!cycleType.compare("CPU Scheduling Code")){
		if(!convertText.compare("FIFO"))
			sch_type = convertText;
		else if(!convertText.compare("PS"))
			sch_type = convertText;
		else if(!convertText.compare("SJF"))
			sch_type = convertText;
	}

	else
		return 2;	// Incorrect input
	return 0;
}

bool ConfData::readStatus(){

	bool programStatus = true;

	if(version == -1)
		std::cout << "Error: version not specified" << std::endl;
	if(!filePath.compare("")){
		std::cout << "Error: file path not specified" << std::endl;
		programStatus = false;
	}
	if(monitorTime == -1){
		std::cout << "Error: monitor time not specified" << std::endl;
		programStatus = false;
	}
	if(processorTime == -1){
		std::cout << "Error: processor time not specified" << std::endl;
		programStatus = false;
	}
	if(scannerTime == -1){
		std::cout << "Error: scanner time not specified" << std::endl;
		programStatus = false;
	}
	if(hardDriveTime == -1){
		std::cout << "Error: hard drive not specified" << std::endl;
		programStatus = false;
	}
	if(keyboardTime == -1){
		std::cout << "Error: keyboard time not specified" << std::endl;
		programStatus = false;
	}
	if(memoryTime == -1){
		std::cout << "Error: memory time not specified" << std::endl;
		programStatus = false;
	}
	if(projectorTime == -1){
		std::cout << "Error: projector time not specified" << std::endl;
		programStatus = false;
	}
	if(logLevel == -1){
		std::cout << "Error: log level not specified" << std::endl;
		programStatus = false;
	}
	if(!logPath.compare("")){
		std::cout << "Error: log file path not specified" << std::endl;
		programStatus = false;
	}

	// v2.0
	if(maxMem == -1){
		std::cout << "Error: system memory not specified" << std::endl;
		programStatus = false;
	}

	// v3.0
	if(numProj == -1){
		std::cout << "Error: projector quantity not specified" << std::endl;
		programStatus = false;
	}
	if(numHDD == -1){
		std::cout << "Error: hard drive quantity not specified" << std::endl;
		programStatus = false;
	}
	if(memBlockSize == -1){
		std::cout << "Error: memory block size not specified" << std::endl;
		programStatus = false;
	}

	// v4.0
	if(pqn == -1){
		std::cout << "Error: processor quantum number not specified" << std::endl;
		programStatus = false;
	}
	if(!sch_type.compare("")){
		std::cout << "Error: scheduling algorithm not specified" << std::endl;
		programStatus = false;
	}
// ----------------------------------------------------------------------------
	if(monitorTime == 0){
		std::cout << "Error: monitor time is zero" << std::endl;
		programStatus = false;
	}
	if(processorTime == 0){
		std::cout << "Error: processor time is zero" << std::endl;
		programStatus = false;
	}
	if(scannerTime == 0){
		std::cout << "Error: scanner time is zero" << std::endl;
		programStatus = false;
	}
	if(hardDriveTime == 0){
		std::cout << "Error: hard drive is zero" << std::endl;
		programStatus = false;
	}
	if(keyboardTime == 0){
		std::cout << "Error: keyboard time is zero" << std::endl;
		programStatus = false;
	}
	if(memoryTime == 0){
		std::cout << "Error: memory time is zero" << std::endl;
		programStatus = false;
	}
	if(projectorTime == 0){
		std::cout << "Error: projector time is zero" << std::endl;
		programStatus = false;
	}

	// v2.0
	if(maxMem == 0){
		std::cout << "Error: system memory is zero" << std::endl;
		programStatus = false;
	}

	// Need the 3 new ones
	// v3.0
	if(numProj == 0){
		std::cout << "Error: number of projectors is zero" << std::endl;
		programStatus = false;
	}
	if(numHDD == 0){
		std::cout << "Error: number of hard drives is zero" << std::endl;
		programStatus = false;
	}
	if(memBlockSize == 0){
		std::cout << "Error: memory block size is zero" << std::endl;
		programStatus = false;
	}

	// v4.0
	if(pqn == 0){
		std::cout << "Error: processor quantum number is zero" << std::endl;
		programStatus = false;
	}

	return programStatus;
}

// v2.0

void ConfData::setMem(int inptMem){
	maxMem = inptMem;
}

int ConfData::getMem(){
	return maxMem;
}

// v3.0

void ConfData::setNumProj(int inptProj){
	numProj = inptProj;
}

int ConfData::getNumProj(){
	return numProj;
}

void ConfData::setNumHDD(int inptHDD){
	numHDD = inptHDD;
}

int ConfData::getNumHDD(){
	return numHDD;
}

void ConfData::setMemBlock(int memSize){
	memBlockSize = memSize;
}

int ConfData::getMemBlock(){
	return memBlockSize;
}

// v4.0

void ConfData::set_pqn(int inpt_pqn){
	pqn = inpt_pqn;
}

int ConfData::get_pqn(){
	return pqn;
}

void ConfData::set_sch(std::string inpt_sch){
	sch_type = inpt_sch;
}

std::string ConfData::get_sch(){
	return sch_type;
}
/**
 * @file	ConfData.h
 * @brief	Definition file for ConfData class
 * @author	Wei Tong
 * @details Specifies all members of ConfData class
 * @version	4.0
 * 			Wei Tong (18 April 2018)
 *			Added new value and functions to understand
 *			processor quantum number and scheduling
 *			algorithm in new configuration files
 */

#include <string>
#include <iostream>
#include <sstream>

#define Monitor 1
#define File 2
#define Both 3

class ConfData{
private:
	float version;
	std::string filePath;
	int monitorTime;
	int processorTime;
	int scannerTime;
	int hardDriveTime;
	int keyboardTime;
	int memoryTime;
	int projectorTime;
	int logLevel;
	std::string logPath;

	// v2.0
	int maxMem;

	// v3.0
	int numProj;
	int numHDD;
	int memBlockSize;

	// v4.0
	int pqn;	// Processor Quantum Number
	std::string sch_type;	// Scheduling type (FIFO, PS, SJF)

public:
	ConfData();								// Default constructor
	~ConfData();							// Default deconstructor
	void setVer(float);						// Sets the version number
	float getVer();							// Retrieves the version number
	void setFilePath(std::string);			// Sets the path of the meta data file
	std::string getFilePath();				// Retrieves the path of the meta data file
	void setLogPath(std::string);			// Sets the path of the log file
	std::string getLogPath();				// Retrieves the path of the log file
	void setLogLvl(int);					// Sets the log level (ie. Monitor, File, Both)
	int getLogLvl();						// Retrieves the log level
	bool setCycleTime(std::string, int);	// Takes name of cycle and the time as input
	int getCycleTime(std::string);			// Returns cycle time of specified object
	int readLine(std::string);				// Read function to take in data
	bool readStatus();						// Mark if read in was successful

	// v2.0
	void setMem(int);
	int getMem();

	// v3.0
	void setNumProj(int);
	int getNumProj();
	void setNumHDD(int);
	int getNumHDD();
	void setMemBlock(int);
	int getMemBlock();

	// v4.0
	void set_pqn(int);
	int get_pqn();
	void set_sch(std::string);
	std::string get_sch();
};
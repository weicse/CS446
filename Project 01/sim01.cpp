/**
 * @file	sim01.cpp
 * @brief	Takes in config file from command line argument
 *			and creates output based on data
 * @author	Wei Tong
 * @details Data output will depend on configuration found
 *			in config file, and meta data file specified
 * @version	1.00
 * 			Wei Tong (7 February 2018)
 *			Initial development, functions will possibly
 *			be used in the future
 * @note	Requires ConfData.h and MetaObj.h
 */

#include "ConfData.h"
#include "MetaObj.h"
#include <queue>
#include <fstream>

int mdfParse(std::string, std::queue<MetaObj> &);
void confOut(ConfData &, std::ostream&);
void confOut(ConfData &, std::ostream&, std::ostream&);
void metaOut(ConfData &, std::queue <MetaObj>, std::ostream&);
void metaOut(ConfData &, std::queue <MetaObj>, std::ostream&, std::ostream&);

int main(int argc, char *argv[]){

	// Check for config file as command line argument
	if(argc < 2){
		std::cout << "Error: config file not specified" << std::endl;
		return 0;
	}

	ConfData cfgd;
	std::queue <MetaObj> mdq;
	std::ifstream fin;
	std::ofstream fout;
	std::string cfgFile = argv[1], temp, mdDesc, errorMsg;

	// Check if config file has right extension
	if(cfgFile.substr(cfgFile.length() - 5) != ".conf"){
		std::cout << "Error: config file should have .conf extension" << std::endl;
		return 0;
	}

	fin.open(cfgFile);
	if(!fin.is_open()){
		std::cout << "Error: config file not found" << std::endl;
		fin.close();
		return 0;
	}

	if(!getline(fin, temp)){
		std::cout << "Error: empty config file" << std::endl;
		return 0;
	}

	if(temp.compare("Start Simulator Configuration File")){
		std::cout << "Error: bad start of config file" << std::endl;
		return 0;
	}

	// Read in configuration file
	int lineCounter = 0, readStatus = 0;
	while(!readStatus){
		readStatus = cfgd.readLine(temp);
		if(!getline(fin, temp))
			readStatus = 3;	// Done reading
		lineCounter++;
	}
	if(readStatus == 1){
		std::cout << "Error: no semicolon found on line " << lineCounter << std::endl;
		return 0;
	}
	if(readStatus == 2){
		std::cout << "Error: incorrect input on line " << lineCounter << std::endl;
		return 0;
	}
	fin.close();

	if(!cfgd.readStatus())
		return 0;

	temp = cfgd.getFilePath();
	if(temp.substr(temp.length() - 4) != ".mdf"){
		std::cout << "Error: meta data file should have .mdf extension" << std::endl;
		return 0;
	}

	fin.open(cfgd.getFilePath());
	if(!fin.is_open()){
		std::cout << "Error: meta data file not found" << std::endl;
		return 0;
	}

	if(!getline(fin, temp)){
		std::cout << "Error: empty meta data file" << std::endl;
		return 0;
	}

	if(temp.compare("Start Program Meta-Data Code:")){
		std::cout << "Error: bad start of meta data file" << std::endl;
		return 0;
	}

	// Read in meta data file to queue
	lineCounter = 1, readStatus = 0;
	while(!readStatus){
		if(!getline(fin, temp) || !temp.compare("End Program Meta-Data Code."))
			readStatus = 4;	// Done reading
		else
			readStatus = mdfParse(temp, mdq);
		lineCounter++;
	}
	fin.close();

	if(readStatus == 1){
		std::cout << "Code error in line " << lineCounter << " of the meta data file" << std::endl;
		return 0;
	}
	if(readStatus == 1){
		std::cout << "Description error in line " << lineCounter << " of the meta data file" << std::endl;
		return 0;
	}
	if(readStatus == 1){
		std::cout << "Cycle error in line " << lineCounter << " of the meta data file" << std::endl;
		return 0;
	}

	// Calculations time
	if(cfgd.getLogLvl() == 1){

		// Log to monitor
		confOut(cfgd, std::cout);
		metaOut(cfgd, mdq, std::cout);
	}
	else if(cfgd.getLogLvl() == 2){

		// Log to file
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		confOut(cfgd, fout);
		metaOut(cfgd, mdq, fout);
	}
	else{

		// Log to both
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		confOut(cfgd, std::cout, fout);
		metaOut(cfgd, mdq, std::cout, fout);
	}

	return 0;
}

// This function will parse the line of input and put the
// data into the queue of MetaObj.
int mdfParse(std::string inputStr, std::queue<MetaObj> &inQ){

	char code;
	std::string description, tempStr;
	int cycles;
	size_t tempIndex;

	while(inputStr.compare("")){

		// Parsing the first set of instructions
		code = inputStr[0];
		description = inputStr.substr(inputStr.find_first_of("{") + 1, inputStr.find_first_of("}") - 2);
		tempStr = inputStr.substr(inputStr.find_first_of("}") + 1, inputStr.find_first_of(";.") - inputStr.find_first_of("}"));
		std::stringstream str2Num(tempStr);
		str2Num >> cycles;
		tempIndex = inputStr.find_first_of(";.");
		if(tempIndex + 2 < inputStr.length()){
			inputStr = inputStr.substr(inputStr.find_first_of(";") + 2);
		}
		else
			inputStr = "";

		// Putting the parsed data into the queue
		// Parameterized constructer NOT used because
		// input is not guaranteed to be correct
		MetaObj newMD;
		if(newMD.setCode(code)){
			if(newMD.setDescription(description)){
				if(!newMD.setCycles(cycles)){
					return 3;
				}
			}
			else{
				return 2;
			}
		}
		else{
			return 1;
		}

		// Push into the queue
		inQ.push(newMD);
	}
	return 0;
}


// This function is used to output configuration to either monitor OR file
void confOut(ConfData &confOutput, std::ostream& out){

	out << "Configuration File Data" << std::endl << "Monitor = " << confOutput.getCycleTime("Monitor")
	<< " ms/cycle" << std::endl << "Processor = " << confOutput.getCycleTime("Processor")
	<< " ms/cycle" << std::endl << "Scanner = " << confOutput.getCycleTime("Scanner")
	<< " ms/cycle" << std::endl << "Hard Drive = " << confOutput.getCycleTime("Hard Drive")
	<< " ms/cycle" << std::endl << "Keyboard = " << confOutput.getCycleTime("Keyboard")
	<< " ms/cycle" << std::endl << "Memory = " << confOutput.getCycleTime("Memory")
	<< " ms/cycle" << std::endl << "Projector = " << confOutput.getCycleTime("Projector")
	<< " ms/cycle" << std::endl;
	if(confOutput.getLogLvl() == 1)
		out << "Logged to: monitor" << std::endl;
	else
		out << "Logged to: " << confOutput.getLogPath() << std::endl;

	out << std::endl << "Meta-Data Metrics" << std::endl;
}

// This function is used to output configuration to BOTH monitor and file
void confOut(ConfData &confOutput, std::ostream& out1, std::ostream& out2){

	out1 << "Configuration File Data" << std::endl << "Monitor = " << confOutput.getCycleTime("Monitor")
	<< " ms/cycle" << std::endl << "Processor = " << confOutput.getCycleTime("Processor")
	<< " ms/cycle" << std::endl << "Scanner = " << confOutput.getCycleTime("Scanner")
	<< " ms/cycle" << std::endl << "Hard Drive = " << confOutput.getCycleTime("Hard Drive")
	<< " ms/cycle" << std::endl << "Keyboard = " << confOutput.getCycleTime("Keyboard")
	<< " ms/cycle" << std::endl << "Memory = " << confOutput.getCycleTime("Memory")
	<< " ms/cycle" << std::endl << "Projector = " << confOutput.getCycleTime("Projector")
	<< " ms/cycle" << std::endl;
	out1 << "Logged to: monitor and " << confOutput.getLogPath() << std::endl;
	out1 << std::endl << "Meta-Data Metrics" <<std::endl;

	out2 << "Configuration File Data" << std::endl << "Monitor = " << confOutput.getCycleTime("Monitor")
	<< " ms/cycle" << std::endl << "Processor = " << confOutput.getCycleTime("Processor")
	<< " ms/cycle" << std::endl << "Scanner = " << confOutput.getCycleTime("Scanner")
	<< " ms/cycle" << std::endl << "Hard Drive = " << confOutput.getCycleTime("Hard Drive")
	<< " ms/cycle" << std::endl << "Keyboard = " << confOutput.getCycleTime("Keyboard")
	<< " ms/cycle" << std::endl << "Memory = " << confOutput.getCycleTime("Memory")
	<< " ms/cycle" << std::endl << "Projector = " << confOutput.getCycleTime("Projector")
	<< " ms/cycle" << std::endl;
	out2 << "Logged to: monitor and " << confOutput.getLogPath() << std::endl;
	out2 << std::endl << "Meta-Data Metrics" << std::endl;
}


void metaOut(ConfData &confNums, std::queue <MetaObj> dataQ, std::ostream& out){

	int monT = confNums.getCycleTime("Monitor");
	int procT = confNums.getCycleTime("Processor");
	int scanT = confNums.getCycleTime("Scanner");
	int hdT = confNums.getCycleTime("Hard Drive");
	int keyT = confNums.getCycleTime("Keyboard");
	int memT = confNums.getCycleTime("Memory");
	int projT = confNums.getCycleTime("Projector");
	MetaObj temp;
	while(!dataQ.empty()){
		temp = dataQ.front();

		// Ignore begin and finish commands
		if(temp.getDescription() != "begin" && temp.getDescription() != "finish"){
			out << temp.getCode() << "{" << temp.getDescription() << "}" << temp.getCycles() << " - ";
			if(temp.getDescription() == "monitor"){
				out << monT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "run"){
				out << procT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "scanner"){
				out << scanT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "hard drive"){
				out << hdT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "keyboard"){
				out << keyT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "projector"){
				out << projT * temp.getCycles() << " ms" << std::endl;
			}
			else{
				out << memT * temp.getCycles() << " ms" << std::endl;
				// This takes care of allocate and block, which are both considered Memory
				// Unfortunately takes care of anything else. Might consider fixing in future if needed
			}
		}
		dataQ.pop();
	}
}

void metaOut(ConfData &confNums, std::queue <MetaObj> dataQ, std::ostream& out1, std::ostream& out2){

	int monT = confNums.getCycleTime("Monitor");
	int procT = confNums.getCycleTime("Processor");
	int scanT = confNums.getCycleTime("Scanner");
	int hdT = confNums.getCycleTime("Hard Drive");
	int keyT = confNums.getCycleTime("Keyboard");
	int memT = confNums.getCycleTime("Memory");
	int projT = confNums.getCycleTime("Projector");

	MetaObj temp;
	while(!dataQ.empty()){
		temp = dataQ.front();

		// Ignore begin and finish commands
		if(temp.getDescription() != "begin" && temp.getDescription() != "finish"){
			out1 << temp.getCode() << "{" << temp.getDescription() << "}" << temp.getCycles() << " - ";
			out2 << temp.getCode() << "{" << temp.getDescription() << "}" << temp.getCycles() << " - ";
			if(temp.getDescription() == "monitor"){
				out1 << monT * temp.getCycles() << " ms" << std::endl;
				out2 << monT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "run"){
				out1 << procT * temp.getCycles() << " ms" << std::endl;
				out2 << procT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "scanner"){
				out1 << scanT * temp.getCycles() << " ms" << std::endl;
				out2 << scanT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "hard drive"){
				out1 << hdT * temp.getCycles() << " ms" << std::endl;
				out2 << hdT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "keyboard"){
				out1 << keyT * temp.getCycles() << " ms" << std::endl;
				out2 << keyT * temp.getCycles() << " ms" << std::endl;
			}
			else if(temp.getDescription() == "projector"){
				out1 << projT * temp.getCycles() << " ms" << std::endl;
				out2 << projT * temp.getCycles() << " ms" << std::endl;
			}
			else{
				out1 << memT * temp.getCycles() << " ms" << std::endl;
				out2 << memT * temp.getCycles() << " ms" << std::endl;
				// This takes care of allocate and block, which are both considered Memory
				// Unfortunately takes care of anything else. Might consider fixing in future if needed
			}
		}
		dataQ.pop();
	}
}
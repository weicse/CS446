/**
 * @file	sim03.cpp
 * @brief	Takes in config file from command line argument
 *			and creates output based on data
 * @author	Wei Tong
 * @details Data output will depend on configuration found
 *			in config file, and meta data file specified
 * @version	3.0
 * 			Wei Tong (28 March 2018)
 *			Created hardware timer simulator, outputs times
 *			for when operations start and end
 * @note	Requires ConfData.h and MetaObj.h
 */

#include "ConfData.h"
#include "MetaObj.h"
#include "PCB.h"
#include <queue>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <semaphore.h>

#define START 1
#define READY 2
#define RUNNING 3
#define WAITING 4
#define EXIT 5

int mdfParse(std::string, std::queue<MetaObj> &);
void confOut(ConfData, std::ostream&, std::ostream&);
void metaOut(ConfData, std::queue <MetaObj>, std::ostream&, std::ostream&);

// v2.0
unsigned int allocateMem(int, int &, int);
void waitTime(int);
void* timerThreadFunc(void*);
void* io_sim(void*);
void procSim(ConfData &, std::queue <MetaObj> &, PCB &, std::ostream&, std::ostream&);

// Helper to create null output stream
class nullBuffer : public std::streambuf{
public:
	int overflow(int c){
		return c;
	}
};

struct timerPackage{

	int countTime = 0;	// Time to count down
	bool contRun = true;	// Helper to stop thread
	std::chrono::system_clock::time_point timeEnd;
};

struct ioPackage{

	int io_time;
	char io_operation;

	// v3.0
	sem_t io_sem;
};

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
	if(readStatus == 2){
		std::cout << "Description error in line " << lineCounter << " of the meta data file" << std::endl;
		return 0;
	}
	if(readStatus == 3){
		std::cout << "Cycle error in line " << lineCounter << " of the meta data file" << std::endl;
		return 0;
	}

	/* Deprecated as of project 2
	if(cfgd.getLogLvl() == 1){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to monitor
		confOut(cfgd, std::cout, null_stream);
		metaOut(cfgd, mdq, std::cout, null_stream);
	}
	else if(cfgd.getLogLvl() == 2){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to file
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		confOut(cfgd, fout, null_stream);
		metaOut(cfgd, mdq, fout, null_stream);
	}
	else{

		// Log to both
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		confOut(cfgd, std::cout, fout);
		metaOut(cfgd, mdq, std::cout, fout);
	}
	*/

	// Simulate process
	PCB procState;

	if(cfgd.getLogLvl() == 1){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to monitor
		procSim(cfgd, mdq, procState, std::cout, null_stream);
	}
	else if(cfgd.getLogLvl() == 2){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to file
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		procSim(cfgd, mdq, procState, fout, null_stream);
	}
	else{

		// Log to both
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		procSim(cfgd, mdq, procState, std::cout, fout);
	}

	return 0;
}

// This function will parse the line of input and put the
// data into the queue of MetaObj.
int mdfParse(std::string inputStr, std::queue<MetaObj> &inQ){

	char code;
	std::string description, tempStr;
	int cycles;

	// Erase any beginning white spaces
	inputStr.erase(remove_if(inputStr.begin(), inputStr.end(), ::isblank), inputStr.end());

	while(inputStr.compare("")){

		// Parsing the first set of instructions
		code = inputStr[0];
		description = inputStr.substr(inputStr.find_first_of("{") + 1, inputStr.find_first_of("}") - 2);
		tempStr = inputStr.substr(inputStr.find_first_of("}") + 1, inputStr.find_first_of(";.") - inputStr.find_first_of("}"));
		std::stringstream str2Num(tempStr);
		str2Num >> cycles;
		if(!str2Num){
			return 3;
		}
		inputStr = inputStr.substr(inputStr.find_first_of(";.") + 1);

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

void confOut(ConfData confOutput, std::ostream& out1, std::ostream& out2){

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

void metaOut(ConfData confNums, std::queue <MetaObj> dataQ, std::ostream& out1, std::ostream& out2){

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

unsigned int allocateMem(int maxMem, int &last_addr, int block_size){

	if(last_addr == -1){
		last_addr = 0;
		return 0;
	}
	else{
		last_addr += block_size;
		if(last_addr > maxMem){
			last_addr = 0;
			return 0;
		}
		else{
			return last_addr;
		}
	}
}

void waitTime(int msec){

	auto start = std::chrono::system_clock::now();
	bool sleep = true;
	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
	while(sleep){
		now = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		if(elapsed.count() >= msec){
			sleep = false;
		}
	}
}

void* timerThreadFunc(void* castedPackage){

	timerPackage *p2 = (struct timerPackage*)castedPackage;
	while(p2->contRun){

		// Counting down every millisecond
		if(p2->countTime > 0){
			waitTime(1);
			p2->countTime--;
		}

		// Finished counting down, marking time
		else if(p2->countTime == 0){
			p2->timeEnd = std::chrono::system_clock::now();
			p2->countTime--;
		}
		p2 = (struct timerPackage*)castedPackage;
	}

	// Return statement required to remove compiler warning
	// However, it is not technically required
	return castedPackage;
}

void* io_sim(void* casted_io){

	ioPackage op_info = *(struct ioPackage*)casted_io;

	// Semaphore lock
	sem_wait(&op_info.io_sem);
	waitTime(op_info.io_time);

	return casted_io;
}

void procSim(ConfData &timeConf, std::queue <MetaObj> &procInfo, PCB &controlBlock, std::ostream& out1, std::ostream& out2){

	int monT = timeConf.getCycleTime("Monitor");
	int procT = timeConf.getCycleTime("Processor");
	int scanT = timeConf.getCycleTime("Scanner");
	int hdT = timeConf.getCycleTime("Hard Drive");
	int keyT = timeConf.getCycleTime("Keyboard");
	int memT = timeConf.getCycleTime("Memory");
	int projT = timeConf.getCycleTime("Projector");

	timerPackage p1;
	pthread_t original_thread, io_tracker;
	int procNum = 0;	// Keep track of the process ID/number

	// New thread dedicated towards timing
	pthread_create(&original_thread, NULL, timerThreadFunc, (void *) &p1);

	auto refPoint = std::chrono::system_clock::now();
	auto rightNow = std::chrono::system_clock::now();
	MetaObj temp;

	int max_mem = timeConf.getMem();
	int last_mem_addr = -1;
	int mem_block = timeConf.getMemBlock();
	int last_hdd = -1;
	int num_hdd = timeConf.getNumHDD();
	int last_proj = -1;
	int num_proj = timeConf.getNumProj();

	while(!procInfo.empty()){
		temp = procInfo.front();
		rightNow = std::chrono::system_clock::now();
		out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
		out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;

		if(temp.getCode() == 'S'){
			if(temp.getDescription() == "begin"){
				out1 << " - Simulator program starting" << std::endl;
				out2 << " - Simulator program starting" << std::endl;
			}
			else{
				out1 << " - Simulator program ending" << std::endl;
				out2 << " - Simulator program ending" << std::endl;
			}
		}
		else if(temp.getCode() == 'A'){
			if(temp.getDescription() == "begin"){
				controlBlock.setState(START);
				procNum++;
				out1 << " - OS: preparing process " << procNum << std::endl;
				out2 << " - OS: preparing process " << procNum << std::endl;
				controlBlock.setState(READY);

				/*
				PCB will have to be overhauled in order to support multi-processes.
				Current implementation (v2.0) tracks process ID/number, but does not support
				multiple processes.
				*/

				rightNow = std::chrono::system_clock::now();
				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out1 << " - OS: starting process " << procNum << std::endl;
				out2 << " - OS: starting process " << procNum << std::endl;
				controlBlock.setState(RUNNING);
			}
			else{
				out1 << " - OS: removing process " << procNum << std::endl;
				out2 << " - OS: removing process " << procNum << std::endl;
				controlBlock.setState(EXIT);
			}
		}
		else if(temp.getCode() == 'P'){
			out1 << " - Process " << procNum << ": start processing action" << std::endl;
			out2 << " - Process " << procNum << ": start processing action" << std::endl;
			p1.countTime = procT * temp.getCycles();
			while(p1.countTime >= 0){
				waitTime(100);	// Check only every 100ms
			}
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
			out1 << " - Process " << procNum << ": end processing action" << std::endl;
			out2 << " - Process " << procNum << ": end processing action" << std::endl;
		}
		else if(temp.getCode() == 'M'){
			if(temp.getDescription() == "allocate"){
				out1 << " - Process " << procNum << ": allocating memory" << std::endl;
				out2 << " - Process " << procNum << ": allocating memory" << std::endl;
				p1.countTime = memT * temp.getCycles();
				while(p1.countTime >= 0){
					waitTime(100);
				}

				int memAddr = allocateMem(max_mem, last_mem_addr, mem_block);

				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
				out1 << " - Process " << procNum << ": memory allocated at " <<  "0x" << std::hex << std::setw(8) << std::setfill('0') << memAddr << std::endl;
				out2 << " - Process " << procNum << ": memory allocated at " <<  "0x" << std::hex << std::setw(8) << std::setfill('0') << memAddr << std::endl;
			}
			else{
				out1 << " - Process " << procNum << ": start memory blocking" << std::endl;
				out2 << " - Process " << procNum << ": start memory blocking" << std::endl;
				p1.countTime = memT * temp.getCycles();
				while(p1.countTime >= 0){
					waitTime(100);
				}

				rightNow = std::chrono::system_clock::now();
				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out1 << " - Process " << procNum << ": end memory blocking" << std::endl;
				out2 << " - Process " << procNum << ": end memory blocking" << std::endl;
			}
		}
		else if(temp.getCode() == 'I'){
			out1 << " - Process " << procNum << ": start " << temp.getDescription() << " input";
			out2 << " - Process " << procNum << ": start " << temp.getDescription() << " input";
			ioPackage op_data;
			if(temp.getDescription() == "hard drive"){
				if(last_hdd == -1 || last_hdd + 1 >= num_hdd){
					out1 << " on HDD 0" << std::endl;
					out2 << " on HDD 0" << std::endl;
					last_hdd = 0;
				}
				else{
					last_hdd++;
					out1 << " on HDD " << last_hdd << std::endl;
					out2 << " on HDD " << last_hdd << std::endl;
				}
				op_data.io_time = hdT * temp.getCycles();
			}
			else if(temp.getDescription() == "keyboard"){
				out1 << std::endl;
				out2 << std::endl;
				op_data.io_time = keyT * temp.getCycles();
			}
			else{
				out1 << std::endl;
				out2 << std::endl;
				op_data.io_time = scanT * temp.getCycles();
			}
			op_data.io_operation = 'I';
			sem_init(&op_data.io_sem, 0, 1);
			
			controlBlock.setState(WAITING);
			pthread_create(&io_tracker, NULL, io_sim, (void *) &op_data);
			pthread_join(io_tracker, NULL);
			sem_post(&op_data.io_sem);	// Semaphore release
			controlBlock.setState(RUNNING);
			
			rightNow = std::chrono::system_clock::now();
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out1 << " - Process " << procNum << ": end " << temp.getDescription() << " input" << std::endl;
			out2 << " - Process " << procNum << ": end " << temp.getDescription() << " input" << std::endl;
		}
		else if(temp.getCode() == 'O'){
			out1 << " - Process " << procNum << ": start " << temp.getDescription() << " output";
			out2 << " - Process " << procNum << ": start " << temp.getDescription() << " output";
			ioPackage op_data;
			if(temp.getDescription() == "hard drive"){
				if(last_hdd == -1 || last_hdd + 1 >= num_hdd){
					out1 << " on HDD 0" << std::endl;
					out2 << " on HDD 0" << std::endl;
					last_hdd = 0;
				}
				else{
					last_hdd++;
					out1 << " on HDD " << last_hdd << std::endl;
					out2 << " on HDD " << last_hdd << std::endl;
				}
				op_data.io_time = hdT * temp.getCycles();
			}
			else if(temp.getDescription() == "monitor"){
				out1 << std::endl;
				out2 << std::endl;
				op_data.io_time = monT * temp.getCycles();
			}
			else{
				if(last_proj == -1 || last_proj + 1 >= num_proj){
					out1 << " on PROJ 0" << std::endl;
					out2 << " on PROJ 0" << std::endl;
					last_proj = 0;
				}
				else{
					last_proj++;
					out1 << " on PROJ " << last_proj << std::endl;
					out2 << " on PROJ " << last_proj << std::endl;
				}
				op_data.io_time = projT * temp.getCycles();
			}
			op_data.io_operation = 'O';
			
			controlBlock.setState(WAITING);
			pthread_create(&io_tracker, NULL, io_sim, (void *) &op_data);
			pthread_join(io_tracker, NULL);
			sem_post(&op_data.io_sem);	// Semaphore release
			controlBlock.setState(RUNNING);
			
			rightNow = std::chrono::system_clock::now();
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out1 << " - Process " << procNum << ": end " << temp.getDescription() << " output" << std::endl;
			out2 << " - Process " << procNum << ": end " << temp.getDescription() << " output" << std::endl;
		}

		procInfo.pop();
	}

	p1.contRun = false;	// Alert timer thread to stop, since process is ending
	pthread_join(original_thread, NULL);
}
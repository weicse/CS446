/**
 * @file	sim05.cpp
 * @brief	Takes in config file from command line argument
 *			and creates output based on data
 * @author	Wei Tong
 * @details Data output will depend on configuration found
 *			in config file, and meta data file specified
 * @version	5.0
 * 			Wei Tong (9 May 2018)
 *			This version supports scheduling algorithms
 *			for RR and 
 * @note	Requires ConfData.h, MetaObj.h, PCB.h
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
#include <vector>

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
void procSim(ConfData &, std::queue <MetaObj> &, PCB &, std::ostream&, std::ostream&, int*);

// v4.0
void schAlg(std::queue <MetaObj> &, std::string, int *&);

// v5.0
void* proc_arrival(void*);

struct new_proc_data{

	std::string file_name;
	std::queue <MetaObj>* existing_proc_list;
};


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
	pthread_mutex_t io_lock;
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

	/* Deprecated as of v2.0
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

	// Schedule algorithm
	int* procList;
	schAlg(mdq, cfgd.get_sch(), procList);

	// Simulate process
	PCB procState;

	// v5.0, simulate process arrival
	new_proc_data newProcData;
	newProcData.file_name = cfgFile;
	newProcData.existing_proc_list = &mdq;

	pthread_t add_proc;
	pthread_create(&add_proc, NULL, proc_arrival, (void *) &newProcData);

	if(cfgd.getLogLvl() == 1){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to monitor
		procSim(cfgd, mdq, procState, std::cout, null_stream, procList);
	}
	else if(cfgd.getLogLvl() == 2){

		nullBuffer nb;
		std::ostream null_stream(&nb);

		// Log to file
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		procSim(cfgd, mdq, procState, fout, null_stream, procList);
	}
	else{

		// Log to both
		std::ofstream fout;
		fout.open(cfgd.getLogPath(), std::fstream::out);
		procSim(cfgd, mdq, procState, std::cout, fout, procList);
	}

	pthread_join(add_proc, NULL);

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

	ioPackage* op_info = (struct ioPackage*)casted_io;

	// Semaphore lock
	sem_wait(&op_info->io_sem);

	// Mutex Lock
	pthread_mutex_lock(&op_info->io_lock);

	waitTime(op_info->io_time);

	// Mutex Unlock
	pthread_mutex_unlock(&op_info->io_lock);

	return casted_io;
}

void procSim(ConfData &timeConf, std::queue <MetaObj> &procInfo, PCB &controlBlock, std::ostream& out1, std::ostream& out2, int* org_procList){

	int monT = timeConf.getCycleTime("Monitor");
	int procT = timeConf.getCycleTime("Processor");
	int scanT = timeConf.getCycleTime("Scanner");
	int hdT = timeConf.getCycleTime("Hard Drive");
	int keyT = timeConf.getCycleTime("Keyboard");
	int memT = timeConf.getCycleTime("Memory");
	int projT = timeConf.getCycleTime("Projector");

	timerPackage p1;
	pthread_t original_thread, io_tracker;
	// int procNum = 0;	// Keep track of the process ID/number
	int procCounter = -1; // Keep track of process ID/number

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
				procCounter++;
				out1 << " - OS: preparing process " << org_procList[procCounter] << std::endl;
				out2 << " - OS: preparing process " << org_procList[procCounter] << std::endl;
				controlBlock.setState(READY);

				/*
				PCB will have to be overhauled in order to support multi-processes.
				Current implementation (v2.0) tracks process ID/number, but does not support
				multiple processes.
				*/

				rightNow = std::chrono::system_clock::now();
				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out1 << " - OS: starting process " << org_procList[procCounter] << std::endl;
				out2 << " - OS: starting process " << org_procList[procCounter] << std::endl;
				controlBlock.setState(RUNNING);
			}
			else{
				out1 << " - OS: removing process " << org_procList[procCounter] << std::endl;
				out2 << " - OS: removing process " << org_procList[procCounter] << std::endl;
				controlBlock.setState(EXIT);
			}
		}
		else if(temp.getCode() == 'P'){
			out1 << " - Process " << org_procList[procCounter] << ": start processing action" << std::endl;
			out2 << " - Process " << org_procList[procCounter] << ": start processing action" << std::endl;
			p1.countTime = procT * temp.getCycles();
			while(p1.countTime >= 0){
				waitTime(100);	// Check only every 100ms
			}
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
			out1 << " - Process " << org_procList[procCounter] << ": end processing action" << std::endl;
			out2 << " - Process " << org_procList[procCounter] << ": end processing action" << std::endl;
		}
		else if(temp.getCode() == 'M'){
			if(temp.getDescription() == "allocate"){
				out1 << " - Process " << org_procList[procCounter] << ": allocating memory" << std::endl;
				out2 << " - Process " << org_procList[procCounter] << ": allocating memory" << std::endl;
				p1.countTime = memT * temp.getCycles();
				while(p1.countTime >= 0){
					waitTime(100);
				}

				int memAddr = allocateMem(max_mem, last_mem_addr, mem_block);

				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(p1.timeEnd - refPoint).count() / (double)1000000;
				out1 << " - Process " << org_procList[procCounter] << ": memory allocated at " <<  "0x" << std::hex << std::setw(8) << std::setfill('0') << memAddr << std::endl;
				out2 << " - Process " << org_procList[procCounter] << ": memory allocated at " <<  "0x" << std::hex << std::setw(8) << std::setfill('0') << memAddr << std::endl;
			}
			else{
				out1 << " - Process " << org_procList[procCounter] << ": start memory blocking" << std::endl;
				out2 << " - Process " << org_procList[procCounter] << ": start memory blocking" << std::endl;
				p1.countTime = memT * temp.getCycles();
				while(p1.countTime >= 0){
					waitTime(100);
				}

				rightNow = std::chrono::system_clock::now();
				out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
				out1 << " - Process " << org_procList[procCounter] << ": end memory blocking" << std::endl;
				out2 << " - Process " << org_procList[procCounter] << ": end memory blocking" << std::endl;
			}
		}
		else if(temp.getCode() == 'I'){
			out1 << " - Process " << org_procList[procCounter] << ": start " << temp.getDescription() << " input";
			out2 << " - Process " << org_procList[procCounter] << ": start " << temp.getDescription() << " input";
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
			
			pthread_mutex_init(&op_data.io_lock, NULL);

			controlBlock.setState(WAITING);
			pthread_create(&io_tracker, NULL, io_sim, (void *) &op_data);
			pthread_join(io_tracker, NULL);
			pthread_mutex_destroy(&op_data.io_lock);	// Mutex destruction
			sem_post(&op_data.io_sem);	// Semaphore release
			controlBlock.setState(RUNNING);
			
			rightNow = std::chrono::system_clock::now();
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out1 << " - Process " << org_procList[procCounter] << ": end " << temp.getDescription() << " input" << std::endl;
			out2 << " - Process " << org_procList[procCounter] << ": end " << temp.getDescription() << " input" << std::endl;
		}
		else if(temp.getCode() == 'O'){
			out1 << " - Process " << org_procList[procCounter] << ": start " << temp.getDescription() << " output";
			out2 << " - Process " << org_procList[procCounter] << ": start " << temp.getDescription() << " output";
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
			sem_init(&op_data.io_sem, 0, 1);
			
			pthread_mutex_init(&op_data.io_lock, NULL);
			
			controlBlock.setState(WAITING);
			pthread_create(&io_tracker, NULL, io_sim, (void *) &op_data);
			pthread_join(io_tracker, NULL);
			pthread_mutex_destroy(&op_data.io_lock);	// Mutex destruction
			sem_post(&op_data.io_sem);	// Semaphore release
			pthread_mutex_destroy(&op_data.io_lock);
			controlBlock.setState(RUNNING);
			
			rightNow = std::chrono::system_clock::now();
			out1 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out2 << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(rightNow - refPoint).count() / (double)1000000;
			out1 << " - Process " << org_procList[procCounter] << ": end " << temp.getDescription() << " output" << std::endl;
			out2 << " - Process " << org_procList[procCounter] << ": end " << temp.getDescription() << " output" << std::endl;
		}

		procInfo.pop();
	}

	p1.contRun = false;	// Alert timer thread to stop, since process is ending
	pthread_join(original_thread, NULL);
}

void schAlg(std::queue <MetaObj> &procList, std::string schType, int *&procOrganized){

	std::queue <MetaObj> q_temp_1, q_temp_2;
	int procNum = 0;
	while(!procList.empty()){
		if(procList.front().getCode() != 'A'){
			q_temp_1.push(procList.front());
			procList.pop();
		}
		else{
			if(procList.front().getDescription() == "begin"){
				procNum++;
			}
			q_temp_1.push(procList.front());
			procList.pop();
		}
	}

	procOrganized = new int[procNum];	// Create array listing all the processes
	for(int i = 0; i < procNum; i++){	// Number the array accordingly
		procOrganized[i] = i + 1;
	}

	int schData[procNum]{};	// Used to track number of I/O
	int sdCounter = -1;	// Need to start at -1, since first encountered P will raise to 0

	if(schType == "FIFO"){
		while(!q_temp_1.empty()){
			procList.push(q_temp_1.front());
			q_temp_1.pop();
		}
		return;
	}

	else if(schType == "PS"){
		while(!q_temp_1.empty()){
			if(q_temp_1.front().getCode() == 'S' || q_temp_1.front().getCode() == 'P' || q_temp_1.front().getCode() == 'M'){
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else if(q_temp_1.front().getCode() == 'I' || q_temp_1.front().getCode() == 'O'){
				schData[sdCounter]++;
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else{
				if(q_temp_1.front().getDescription() == "begin"){
					sdCounter++;
				}
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
		}
		// Organize the processes NUMBERS (sort largest to smallest)
		for(int i = 0; i < procNum; i++){
			for(int j = i + 1; j < procNum; j++){
				if(schData[i] < schData[j]){
					for(int k = j; k > i; k--){
						std::swap(schData[k], schData[k - 1]);
						std::swap(procOrganized[k], procOrganized[k - 1]);
					}
				}
			}
		}
	}
	else if(schType == "SJF"){
		while(!q_temp_1.empty()){
			if(q_temp_1.front().getCode() == 'S'){
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else if(q_temp_1.front().getCode() == 'A'){
				if(q_temp_1.front().getDescription() == "begin"){
					sdCounter++;
				}
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else{
				schData[sdCounter]++;
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
		}
		// Organize the processes NUMBERS (sort smallest to largest)
		for(int i = 0; i < procNum; i++){
			for(int j = i + 1; j < procNum; j++){
				if(schData[j] < schData[i]){
					for(int k = j; k > i; k--){
						std::swap(schData[k], schData[k - 1]);
						std::swap(procOrganized[k], procOrganized[k - 1]);
					}
				}
			}
		}
	}

	// v5.0
	else if(schType == "RR"){
		while(!q_temp_1.empty()){
			if(q_temp_1.front().getCode() == 'S'){
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else if(q_temp_1.front().getCode() == 'A'){
				if(q_temp_1.front().getDescription() == "begin"){
					sdCounter++;
				}
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else{
				schData[sdCounter]++;
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
		}
		// Organize the processes NUMBERS (sort smallest to largest)
		for(int i = 0; i < procNum; i++){
			for(int j = i + 1; j < procNum; j++){
				if(schData[j] < schData[i]){
					for(int k = j; k > i; k--){
						std::swap(schData[k], schData[k - 1]);
						std::swap(procOrganized[k], procOrganized[k - 1]);
					}
				}
			}
		}
	}

	else if(schType == "STR"){
		while(!q_temp_1.empty()){
			if(q_temp_1.front().getCode() == 'S'){
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else if(q_temp_1.front().getCode() == 'A'){
				if(q_temp_1.front().getDescription() == "begin"){
					sdCounter++;
				}
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
			else{
				schData[sdCounter]++;
				q_temp_2.push(q_temp_1.front());
				q_temp_1.pop();
			}
		}
		// Organize the processes NUMBERS (sort smallest to largest)
		for(int i = 0; i < procNum; i++){
			for(int j = i + 1; j < procNum; j++){
				if(schData[j] < schData[i]){
					for(int k = j; k > i; k--){
						std::swap(schData[k], schData[k - 1]);
						std::swap(procOrganized[k], procOrganized[k - 1]);
					}
				}
			}
		}
	}

	// Actual process re-organization
	std::vector<std::queue<MetaObj>> procDivide;
	std::queue<MetaObj> mo_temp;
	while(!q_temp_2.empty()){
		if(q_temp_2.front().getCode() == 'S'){
			if(q_temp_2.front().getDescription() == "begin"){
				procList.push(q_temp_2.front());
				q_temp_2.pop();
			}
			else{
				procDivide.push_back(mo_temp);
				while(!mo_temp.empty()){
					mo_temp.pop();
				}
				mo_temp.push(q_temp_2.front());
				q_temp_2.pop();
				procDivide.push_back(mo_temp);
			}
		}
		else if(q_temp_2.front().getCode() == 'A'){
			if(q_temp_2.front().getDescription() == "finish"){
				mo_temp.push(q_temp_2.front());
				q_temp_2.pop();
				procDivide.push_back(mo_temp);
				while(!mo_temp.empty()){
					mo_temp.pop();
				}
			}
			else{
				mo_temp.push(q_temp_2.front());
				q_temp_2.pop();
			}
		}
		else{
			mo_temp.push(q_temp_2.front());
			q_temp_2.pop();
		}
	}
		for(int i = 0; i < procNum; i++){
		while(!procDivide[procOrganized[i] - 1].empty()){
			procList.push(procDivide[procOrganized[i] - 1].front());
			procDivide[procOrganized[i] - 1].pop();
		}
	}
	procList.push(procDivide.back().front());
}

// v5.0
void* proc_arrival(void* casted_data){

	new_proc_data* temp = (new_proc_data*)casted_data;
	std::ifstream fin;
	fin.open(temp->file_name);
	std::string read_in_data;

	for(int i = 0; i < 10; i++){
		int readStatus = 0;
		while(!readStatus){
			if(!getline(fin, read_in_data) || !read_in_data.compare("End Program Meta-Data Code."))
				readStatus = 4;	// Done reading
			else
				readStatus = mdfParse(read_in_data, *(temp->existing_proc_list));
		}
		waitTime(100);
	}
	fin.close();
	std::cout << "DEBUG: New process should be added now" << std::endl;
	return casted_data;
}
/**
 * @file	PCB.cpp
 * @brief	Implementation file for PCB class
 * @author	Wei Tong
 * @details Implements all members of PCB class
 * @version	1.00
 * 			Wei Tong (28 February 2018)
 *			Initial development, functions will possibly
 *			be used in the future
 */

#include "PCB.h"

PCB::PCB(){
    processState = 0;
}

PCB::~PCB(){

}

int PCB::getState(){
    return processState;
}

void PCB::setState(int inpt){
    if(inpt > 0 && inpt < 6)
        processState = inpt;
}
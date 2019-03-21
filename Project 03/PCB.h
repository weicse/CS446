/**
 * @file	PCB.h
 * @brief	Definition file for PCB class
 * @author	Wei Tong
 * @details Specifies all members of PCB class
 * @version	1.00
 * 			Wei Tong (28 February 2018)
 *			Initial development, functions will possibly
 *			be used in the future
 */

class PCB{
public:
    PCB();

    ~PCB();

    int getState();

    void setState(int);

private:
    int processState;
};
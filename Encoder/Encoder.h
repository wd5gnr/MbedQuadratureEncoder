/*

Public Domain 2022 Al Williams


 */


#ifndef __Encoder_h__
#define __Encoder_h__

#include "mbed.h"

class Encoder{
public:
// constructor with default no pulldown
    Encoder(PinName pinA, PinName pinB, PinMode mode=PullNone);
// Set the lock timeout - this is the time to decide the encoder is quiesecent
// and that the next reading may be in any direction
// without this there are some issues with missing a few clicks when you change directions    
    void setLockTimeout(std::chrono::milliseconds to) { locktime0=to; }
// set to true if you want the class to accumlate counts, otherwise every read goes to zero    
    void setAccumulate(bool flag) { accumulate=flag; }
    int read();

protected:
    InterruptIn APin;  // pin A
    DigitalIn  BPin;   // pin B
    bool        accumulate=false;  // if true, don't clear on read
    int         lastB;  // remember last B for debounce
    int         accum;  // accumulated count since last read
    int         lock;   // lock to avoid noise
    Kernel::Clock::time_point locktime;  // this is a hack to handle going from CW to CCW or vice versa
    std::chrono::milliseconds locktime0; // essentially we ignore the lock after a set  amount of time
    // ISRs
    void isrRisingA();
    void isrFallingA();
};

#endif
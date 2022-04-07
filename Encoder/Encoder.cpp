/*

Public Domain 2022 Al Williams


 */

#include "Encoder.h"


// Our constructor
Encoder::Encoder(PinName pinA, PinName pinB, PinMode mode) :
    APin(pinA, mode),
    BPin(pinB, mode)
{
    accum = 0;   // no counts
    lock=0;      // unlocked
    // default options
    locktime0=100ms;
    locktime=Kernel::Clock::now()-locktime0;
    lastB=2;  // not 0 or 1
// set up interrupts
    APin.rise(callback(this, &Encoder::isrRisingA));
    APin.fall(callback(this, &Encoder::isrFallingA));
}



// Read a value. May or may not clear the count depending on accumulate
int Encoder::read()
{
    int delta;
    APin.disable_irq();      // don't fight with the access to accum
    delta = accum;
    if (!accumulate) accum = 0;
    APin.enable_irq();    
    return delta;
}

// The encoder probably has a pull up so high is the rest state
// So when going back to rest, we look if we should reset the lock
// If b!=lastB then we unlock and set a new lastB
// If you have bounce noise, keep in mind that B should be stable while A is bouncing
// So once you remember the new lastB, this function does nothing
// since you will be unlocked and will set lastB=b which is a NOP
// However, we set lock=0 and lastB=b because it is cheaper than 
// deciding which one to do, if any in the case where you should unlock
// or should remember a new lastB
void Encoder::isrRisingA()
{
    int b=BPin;   // read B
    if (lock && lastB==b) return;   // not time to unlock
    // if lock=0 and _lastB==b these two lines do nothing
    // but if lock is 1 and/or _lastB!=b then one of them does something
    lock=0;
    lastB=b;
    locktime=Kernel::Clock::now()+locktime0;  // even if not locked, timeout the lastB
}

// The falling edge is where we do the count
// Note that if you pause a bit, the lock will expire because otherwise
// we have to monitor B also to know if a change in direction occured
// It is tempting to try to mutually lock/unlock the ISRs, but in real life
// the edges are followed by a bunch of bounce edges while B is stable
// B will change while A is stable
// So unless you want to also watch B against A, you have to make some
// compromise and this works well enough in practice
void Encoder::isrFallingA()
{
   int b;
   // clear lock if timedout and in either case forget lastB if we haven't seen an edge in a long time
    if (locktime<Kernel::Clock::now())
    {
        lock=0;
        lastB=2;  // impossible value so we must read this event
    }
    if (lock) return;  // we are locked so done
    b=BPin;            // read B
    if (b==lastB) return;  // no change in B
    lock=1;             // don't read the upcoming bounces
    locktime=Kernel::Clock::now()+locktime0;  // set up timeout for lock
    lastB=b;            // remember where B is now
    accum+=(b?-1:1);    // finally, do the count!
}





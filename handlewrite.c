#include "ssdlab.h"

// Handles all write requests.
void handleWriteRequests(int addr, int size){
    // Translate logical address into logical page
    int beginLPN = addr / PAGESIZE;
    int endLPN = (addr + size - 1) / PAGESIZE;
    // For each logical page, you need to do something...
    // Was the mapping table entry for the logical page valid?
    // If so, what should you do?

    // When writing, if the frontier block is full, 
    // what should you do?

    // Remember to set up the mapping between the logical and physical page number.

    // Lastly, you should always check if the SSD should perform GC.
    // If GC is needed, then perform GC.
    // Remember you can skip writing your own victim selection and GC algorithm
    // by utilizing the findVictim() and gc() provided by us.
    // See the handout for more information.
}
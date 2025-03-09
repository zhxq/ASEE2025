#include "ssdlab.h"
void student_gc(int victimBlock);
void gc(int victimBlock){
    if (victimBlock >= 0 && victimBlock < TOTALPHYSBLOCKS){
        realGCPages += PAGESPERBLOCK - countBlockInvalidPages(victimBlock);
    }
    student_gc(victimBlock);
}
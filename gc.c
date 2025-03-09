#include "ssdlab.h"

// Performs garbage collection on the given victim block.
void gc(int victimBlock){
    // In GC, you should copy all valid page in the victim to the current frontier block
    // You should iterate through all pages in the victim block.

    // Each block has PAGESPERBLOCK pages. 
    // You can iterate through all pages in a block using a for loop like
    // for (pageNum = 0; pageNum < PAGESPERBLOCK; pageNum++)

    // To get the PPN given a block number and the page number within that block,
    // you can use ppnFromBlockPageNum(blockNum, pageNum);

    // For each page in the victim: think about what to do if the page is valid.
    // Think about relocation and mapping issues.

    // Finally you should mark the block as free for future use.
}

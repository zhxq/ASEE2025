
#include "ssdlab.h"

char *trace_file;
bool toIntegrityCheck = 0;
bool toPrintWAF = 0;
bool exceedMaxOps = 0;

bool errorOccured = 0;

int frontierBlock = 0;
int frontierPage = 0;

int hostWritePages = 0;
int realGCPages = 0;

int totalLogiPages = 0;
int totalPhysPages = 0;

struct map *maptbl;
struct rmap *rmap;

bool showError = false;

int numFreeBlocks;
struct block_info block_info[TOTALPHYSBLOCKS];
struct page_info page_info[TOTALPHYSBLOCKS][PAGESPERBLOCK];


// Translates a ppa struct into page index for rmap.
int ppa2pgidx(struct ppa *ppa){
    return ppa->g.blk * PAGESPERBLOCK + ppa->g.pg;
}

// Checks if PPN is valid by checking if the block/page is within their respective ranges.
bool checkPPNValidity(int ppn){
    struct ppa ppa;
    ppa.ppa = ppn;
    if (ppa.g.blk < TOTALPHYSBLOCKS && ppa.g.pg < PAGESPERBLOCK){
        return true;
    }
    return false;
}

// Uses next free block as the write frontier.
bool useNextFreeBlock(){
    int block = 0;
    int i = 0;
    if (frontierPage < PAGESPERBLOCK){
        error_log("Your frontier block is not currently full. You need to fully write your frontier block before requesting for a new block. You currently used %d pages in the frontier block.\n", frontierPage);
    }
    for (i = 0; i < TOTALPHYSBLOCKS; i++){
        // Need to make sure we choose a free block
        if (block_info[i].status == BLOCK_FREE){
            // Change the current frontier block status to full
            block_info[frontierBlock].status = BLOCK_FULL;
            // Change the frontier block and reset the frontier page counter
            frontierBlock = i;
            frontierPage = 0;
            block_info[i].status = BLOCK_FRONTIER;
            // Keep trace of the number of free blocks
            numFreeBlocks -= 1;
            return true;
        }
    }
    error_log("Failed to find a free block. Did you perform GC correctly? Did you mark physical pages invalid when needed so the GC can run correctly? Did you set up the logical-to-physical mapping when needed?\n");
    return false;
}

// Checks if the current frontier block is full.
bool isFrontierBlockFull(){
    // printf("frontierBlock: %d, frontierPage: %d\n", frontierBlock, frontierPage);
    if (frontierPage >= PAGESPERBLOCK){
        return true;
    }
    return false;
}

// Get the next available physical page number from the frontier block.
// Returns -1 if the frontier block is full.
int getAvailPPN(){
    struct ppa ppa;
    if (isFrontierBlockFull()){
        error_log("The current frontier block is full. A new free block is needed.\n");
        return -1;
    }
    
    ppa.g.blk = frontierBlock;
    ppa.g.pg = frontierPage;
    checkPPNValidity(ppa.ppa);
    if (page_info[frontierBlock][frontierPage].status != PG_FREE){
        error_log("You are trying to use the PPN 0x%x as the next physical page. However, it is currently used by LPN 0x%x.\n", ppa.ppa, getP2LMapping(ppa.ppa));
    }
    frontierPage++;
    return ppa.ppa;
}

// Gets the raw mapping table entry given the LPN.
struct map* getL2PMappingEntry(int lpn){
    return &maptbl[lpn];
}

// Gets the raw reverse mapping table entry given the PPN.
struct rmap* getP2LMappingEntry(int ppn){
    struct ppa ppa;
    ppa.ppa = ppn;
    return &rmap[ppa2pgidx(&ppa)];
}

// Marks a block as free.
// Also marks all pages in the block as free.
void markBlockFree(int block){
    int i;
    if (block >= TOTALPHYSBLOCKS){
        error_log("Block %d exceeds total number of physical blocks %d.\n", block, TOTALPHYSBLOCKS);
    }
    // Mark the block as free first
    block_info[block].status = BLOCK_FREE;
    block_info[block].numInvalidPages = 0;
    // Mark the pages as free after marking the block free for consistency in reall SSDs
    for (i = 0; i < PAGESPERBLOCK; i++){
        page_info[block][i].status = PG_FREE;
    }
    numFreeBlocks += 1;
}

// Gets the PPN given the LPN from the mapping table.
int getL2PMapping(int lpn){
    if (lpn >= totalLogiPages){
        error_log("LPN 0x%x exceeds total number of logical pages %d.\n", lpn, totalLogiPages);
    }
    struct map* maptblEntry = getL2PMappingEntry(lpn);
    return maptblEntry->ppn;
}

// Gets the LPN given the PPN from the reverse mapping table.
int getP2LMapping(int ppn){
    struct ppa ppa;
    ppa.ppa = ppn;
    if (!checkPPNValidity(ppn)){
        error_log("PPN 0x%x is invalid. It corresponses to block %d, page %d.\n", ppn, ppa.g.blk, ppa.g.pg);
    }
    struct rmap* rmapEntry = getP2LMappingEntry(ppn);
    return rmapEntry->lpn;
}

// Checks if the mapping table entry of the given LPN is valid.
bool isValidLPNMapping(int lpn){
    if (lpn >= totalLogiPages){
        error_log("LPN 0x%x exceeds total number of logical pages 0x%x.\n", lpn, totalLogiPages);
    }
    struct map* maptblEntry = getL2PMappingEntry(lpn);
    return maptblEntry->isValid;
}

// Checks if the reverse mapping table entry of the given PPN is valid.
bool isValidPPNMapping(int ppn){
    struct ppa ppa;
    ppa.ppa = ppn;
    if (!checkPPNValidity(ppn)){
        error_log("PPN 0x%x is invalid. It corresponses to block %d, page %d.\n", ppn, ppa.g.blk, ppa.g.pg);
    }
    struct rmap* rmapEntry = getP2LMappingEntry(ppn);
    return rmapEntry->isValid;
}


// Gets physical page number given the block number and the page number in a block
int ppnFromBlockPageNum(int block, int page){
    struct ppa ppa;
    ppa.g.blk = block;
    ppa.g.pg = page;
    return ppa.ppa;
}

// Gets the number of free blocks.
int getNumFreeBlocks(){
    return numFreeBlocks;
}

// Returns if the SSD should trigger GC.
// Returns true if the number of free blocks fall under the preset range.
bool shouldGC(){
    int numFreeBlocks = getNumFreeBlocks();
    if (numFreeBlocks < TOTALPHYSBLOCKS * (1 - GCWATERMARK)){
        return true;
    }
    return false;
}

// Sets the logical to physical mapping in mapping table given the LPN and the PPN.
void setL2PMapping(int lpn, int ppn){
    struct ppa ppa;
    if (lpn >= totalLogiPages){
        error_log("LPN 0x%x exceeds total number of logical pages 0x%x.\n", lpn, totalLogiPages);
    }
    if (!checkPPNValidity(ppn)){
        ppa.ppa = ppn;
        error_log("PPN 0x%x is invalid. It corresponses to block %d, page %d.\n", ppn, ppa.g.blk, ppa.g.pg);
    }
    struct map* maptblEntry = getL2PMappingEntry(lpn);
    if (ppn >= 0){
        maptblEntry->isValid = true;
        maptblEntry->ppn = ppn;
    }else{
        maptblEntry->isValid = false;
        maptblEntry->ppn = -1;
    }
}

// Sets the physical to logical mapping in reverse mapping table given the LPN and the PPN.
void setP2LMapping(int lpn, int ppn){
    struct ppa ppa;
    ppa.ppa = ppn;
    if (lpn >= totalLogiPages){
        error_log("LPN 0x%x exceeds total number of logical pages 0x%x.\n", lpn, totalLogiPages);
    }
    if (!checkPPNValidity(ppn)){
        error_log("PPN 0x%x is invalid. It corresponses to block %d, page %d.\n", ppn, ppa.g.blk, ppa.g.pg);
    }
    struct rmap* rmapEntry = getP2LMappingEntry(ppn);
    if (lpn >= 0){
        rmapEntry->isValid = true;
        rmapEntry->lpn = lpn;
        if (page_info[ppa.g.blk][ppa.g.pg].status != PG_FREE){
            error_log("You are trying to mark PPN 0x%x (block %d, page %d) as valid. However, the page was not marked as free currently. You can only mark a page as valid when it was previous free.\n" \
            "Did you set up the physical-to-logical mapping to -1 (which marks the page as invalid) when needed?\n", ppa.ppa, ppa.g.blk, ppa.g.pg);
        }
        page_info[ppa.g.blk][ppa.g.pg].status = PG_VALID;
    }else{
        rmapEntry->isValid = false;
        rmapEntry->lpn = -1;
        if (page_info[ppa.g.blk][ppa.g.pg].status != PG_VALID){
            error_log("You are trying to mark PPN 0x%x (block %d, page %d) as invalid. However, the page was not marked as valid currently. You can only mark a page as invalid when it was previously valid.\n" \
            "Did you set up the physical-to-logical mapping (which marks the page as valid) when needed?\n", ppa.ppa, ppa.g.blk, ppa.g.pg);
        }
        page_info[ppa.g.blk][ppa.g.pg].status = PG_INVALID;
        block_info[ppa.g.blk].numInvalidPages += 1;
    }
}

// Checks the (reverse) mapping table integrity
void integrityCheck(bool showIntegrityErrors){
    int i, j;
    int ppn, lpn;
    struct ppa ppa;
    int validPPNCount = 0, validLPNCount = 0, validPageCount = 0;
    int correctLPNCount = 0, correctPPNCount = 0;

    // Check if L->P matches P->L mappings
    for (i = 0; i < totalLogiPages; i++){
        if (isValidLPNMapping(i)){
            validLPNCount++;
            ppn = getL2PMapping(i);
            ppa.ppa = ppn;
            if (ppa2pgidx(&ppa) < totalPhysPages){
                if (!isValidPPNMapping(ppn)){
                    integrity_log("Integrity check failed. Your LPN 0x%x is mapped to an invalid PPN 0x%x.\n", i, ppn);
                }else{
                    lpn = getP2LMapping(ppn);
                    if (i != lpn){
                        integrity_log("Integrity check failed. Your LPN 0x%x is mapped to PPN 0x%x, but PPN 0x%x is not mapped to LPN 0x%x (mapped to 0x%x instead). \nHave you invalidated physical-to-logical and logical-to-physical mappings when needed?\n", i, ppn, ppn, i, lpn);
                    }else{
                        correctLPNCount++;
                    }
                }
            }else{
                integrity_log("Integrity check failed. Your LPN 0x%x is mapped to an out-of-bound PPN 0x%x.\n", i, ppn);
            }
        }
    }

    // Check if P->L matches L->P mappings
    for (i = 0; i < TOTALPHYSBLOCKS; i++){
        for (j = 0; j < PAGESPERBLOCK; j++){
            if (page_info[i][j].status == PG_VALID){
                validPageCount++;
            }
            ppa.g.blk = i;
            ppa.g.pg = j;
            if (isValidPPNMapping(ppa.ppa)){
                validPPNCount++;
                lpn = getP2LMapping(ppa.ppa);
                if (!isValidLPNMapping(lpn)){
                    integrity_log("Integrity check failed. Your PPN 0x%x is mapped to an invalid LPN 0x%x.\n", ppa.ppa, lpn);
                }else{
                    ppn = getL2PMapping(lpn);
                    if (ppa.ppa != ppn){
                        integrity_log("Integrity check failed. Your PPN 0x%x is mapped to LPN 0x%x, but the LPN 0x%x is not mapped to PPN 0x%x (mapped to 0x%x instead). \nHave you invalidated physical-to-logical and logical-to-physical mappings when needed?\n", ppa.ppa, lpn, lpn, ppa.ppa, ppn);
                    }else{
                        correctPPNCount++;
                    }
                }
            }
        }
    }
    if (validLPNCount != validPPNCount){
        integrity_log("Integrity check failed. You should have the same number of valid LPN and PPN entries; \nhowever, you have %d valid LPN entries and %d valid PPN entries. Have you invalidated physical-to-logical and logical-to-physical mappings when needed?\n", validLPNCount, validPPNCount);
    }

    if (validLPNCount != correctLPNCount){
        integrity_log("Integrity check failed. You should have the same number of valid LPN entries and correct LPN entries; \nhowever, you have %d valid LPN entries and %d correct LPN entries. Have you invalidated/updated logical-to-physical mappings when needed?\n", validLPNCount, correctLPNCount);
    }

    if (validPPNCount != correctPPNCount){
        integrity_log("Integrity check failed. You should have the same number of valid PPN entries and correct PPN entries; \nhowever, you have %d valid PPN entries and %d correct PPN entries. Have you invalidated/updated physical-to-logical mappings when needed?\n", validPPNCount, correctPPNCount);
    }

    if (correctLPNCount != correctPPNCount){
        integrity_log("Integrity check failed. You should have the same number of correct LPN entries and correct PPN entries; \nhowever, you have %d correct LPN entries and %d correct PPN entries. Have you invalidated/updated physical-to-logical and physical-to-logical mappings when needed?\n", correctLPNCount, correctPPNCount);
    }
    
    if (!errorOccured){
        printf("Congratulations! All integrity checks passed.\n");
    }else{
        printf("There is at least an error in the integrity check.\n");
    }

    printf("Statistics:\n");
    printf("Correct mapping table entries: %d\n", correctLPNCount);
    printf("Correct reverse mapping table entries: %d\n", correctPPNCount);
}

// Initialize all data structures
void init(){
    int i = 0, j = 0;
    // Init mapping table
    struct block_num *bnum;
    totalPhysPages = TOTALPHYSBLOCKS * PAGESPERBLOCK;
    printf("totalPhysPages: %d\n", totalPhysPages);

    totalLogiPages = TOTALLOGIBLOCKS * PAGESPERBLOCK;

    // Initialize mapping table
    maptbl = malloc(sizeof(struct map) * totalLogiPages);
    for (i = 0; i < totalLogiPages; i++){
        maptbl[i].isValid = false;
        maptbl[i].ppn = -1;
    }

    // Initialize reverse mapping table
    rmap = malloc(sizeof(struct rmap) * totalPhysPages);
    for (i = 0; i < totalPhysPages; i++){
        // Init reverse mapping table
        rmap[i].isValid = false;
        rmap[i].lpn = -1;
    }
    
    // Initialize all block/page information
    for (i = 0; i < TOTALPHYSBLOCKS; i++){
        markBlockFree(i);
    }

    // A small hack to circumvent the sanity check in useNextFreeBlock()
    frontierPage = PAGESPERBLOCK;
    // Request a new block as the frontier block
    useNextFreeBlock();
}

// Returns the number of invalid pages in a block given the block number.
int countBlockInvalidPages(int block){
    return block_info[block].numInvalidPages;
}

// Increments time.
void timeInc(){
    curTime++;
}

// Returns current time.
uint32_t getTime(){
    return curTime;
}

// Gets the status of a block given the block number.
int getBlockStatus(int block){
    return block_info[block].status;
}
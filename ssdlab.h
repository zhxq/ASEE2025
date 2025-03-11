#ifndef _SSDLAB_HEADER
#define _SSDLAB_HEADER

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#define error_log(fmt, ...) \
    do { errorOccured = true; if (showError){printf("Assertion error at function %s:\n", __func__); printf(fmt, ## __VA_ARGS__); integrityCheck(false); exit(2);}} while (0)

#define integrity_log(fmt, ...) \
    do { errorOccured = true; if (showIntegrityErrors){printf("Integrity error at function %s:\n", __func__); printf(fmt, ## __VA_ARGS__); integrityCheck(false); exit(2);}} while (0)

#define PAGESIZE 4096

enum {
    PG_FREE = 0,
    PG_INVALID = 1,
    PG_VALID = 2
};

enum {
    BLOCK_FREE = 0,
    BLOCK_FULL = 1,
    BLOCK_FRONTIER = 2
};

#define GCWATERMARK 0.75

#define TOTALPHYSBLOCKS 640
#define TOTALLOGIBLOCKS 512
#define PAGESPERBLOCK 512
// Total logical capacity: 1GiB = 1073741824B

typedef uint32_t mem_addr_t;

extern uint32_t curTime;

struct map {
    int ppn;
    bool isValid;
};

struct rmap {
    int lpn;
    bool isValid;
};

struct block_info {
    int status;
    int numInvalidPages;
};

struct page_info {
    int status;
};


extern char *trace_file;
extern bool toIntegrityCheck;
extern bool toPrintWAF;
extern bool exceedMaxOps;

extern bool errorOccured;

extern int frontierBlock;
extern int frontierPage;

extern int hostWritePages;
extern int realGCPages;

extern int totalLogiPages;
extern int totalPhysPages;

extern struct map *maptbl;
extern struct rmap *rmap;

extern bool showError;

extern int numFreeBlocks;
extern struct block_info block_info[TOTALPHYSBLOCKS];
extern struct page_info page_info[TOTALPHYSBLOCKS][PAGESPERBLOCK];

// FROM FEMU START
#define BLK_BITS   (16)
#define PG_BITS    (16)

struct ppa {
    union {
        struct {
            uint32_t blk : BLK_BITS;
            uint32_t pg  : PG_BITS;
        } g;
        uint32_t ppa;
    };
};
int ppa2pgidx(struct ppa *ppa);
// FROM FEMU END

bool checkPPNValidity(int ppn);
int ppnFromBlockPageNum(int block, int page);

void panic();
int getAvailPPN();

bool isValidLPNMapping(int lpn);
int getL2PMapping(int lpn);
void setL2PMapping(int lpn, int ppn);

bool isValidPPNMapping(int ppn);
int getP2LMapping(int ppn);
void setP2LMapping(int lpn, int ppn);

struct map* getL2PMappingEntry(int lpn);
struct rmap* getP2LMappingEntry(int ppn);
bool isFrontierBlockFull();
bool useNextFreeBlock();
void markBlockFree(int block);
int countBlockInvalidPages(int block);
int findVictim();
void gc(int victimBlock);
void handleWriteRequests(int addr, int size);
void init();
bool shouldGC();
void integrityCheck(bool showIntegrityErrors);
uint32_t getTime();
void timeInc();
int getBlockStatus(int block);
#endif
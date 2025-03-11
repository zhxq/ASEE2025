#include "ssdlab.h"
// This file contains trace replay driver, based on cachelab's test-trans.c.
// This file will NOT be submitted when you turn in your work.
// Please make all changes to your ssdlab.c.

uint32_t curTime = 0;

/*
 * replayTrace - replays the given trace file against the SSD 
 */
void replayTrace(char* trace_fn){
    char buf[1000];
    int startPage = 0, endPage = 0;
    mem_addr_t addr = 0;
    int len = 0;
    uint32_t time;
    int i;
    struct pte* pte;
    int counter;
    int victimBlock = -1;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "The trace file you provided, %s, cannot be opened: \n%s.\n", trace_fn, strerror(errno));
        exit(1);
    }

    while(fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1] == 'W') {
            sscanf(buf + 3, "%x,%u", &addr, &len);
            // Increment the time for each line of trace
            timeInc();
            counter++;
            startPage = addr / PAGESIZE;
            endPage = (addr + len - 1) / PAGESIZE;

            // Check start logical page address: is it out of bound?
            if (startPage >= totalLogiPages){
                error_log("Your operation W,%x,%d has a start address 0x%x that exceeds total logical SSD size 0x%x bytes. Exiting.\n", addr, len, addr, totalLogiPages * PAGESIZE);
                exit(1);
            }

            // Check end logical page address: is it out of bound?
            if (endPage >= totalLogiPages){
                error_log("Your operation W,%x,%d has a start address 0x%x, which is in SSD logical size range. However, the length of the request %d is too long and exceeds the total logical SSD size 0x%x bytes. Exiting.\n", addr, len, addr, len, totalLogiPages * PAGESIZE);
                exit(1);
            }
            hostWritePages += (endPage - startPage + 1);
            
            // Issue the request to student's implementation
            handleWriteRequests(addr, len);
        }
        if (getTime() == UINT32_MAX){
            printf("You have reached the maximum number of operations you can have in a trace.\n");
            printf("You can only have %d operations per trace file.\n", UINT32_MAX);
            exit(0);
        }
    }
    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[]){
    printf("Usage: %s [-hiw] -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -i         Check mapping table integrity before exit.\n");
    printf("  -w         Print WAF before exit.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("To check mapping table integrity after running traces/systor17-1.trace before exit:\n");
    printf("  linux>  %s -i -t traces/systor17-1.trace\n\n", argv[0]);
    printf("To show WAF after running traces/systor17-1.trace before exit:\n");
    printf("  linux>  %s -w -t traces/systor17-1.trace\n\n", argv[0]);
    exit(0);
}

int main(int argc, char* argv[]){
    char c;
    showError = true;
    while((c = getopt(argc, argv, "t:iwh")) != -1){
        switch(c){
        case 't':
            trace_file = optarg;
            break;
        case 'i':
            toIntegrityCheck = 1;
            break;
        case 'w':
            toPrintWAF = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }
    if (!toIntegrityCheck && !toPrintWAF){
        printf("You did not request any output although you provided a trace file.\n");
        printf("The following flags can produce helpful outputs for you to debug your code:\n");
        printUsage(argv);
        exit(1);
    }
    if (trace_file == NULL){
        printUsage(argv);
        exit(1);
    }
    
    init();
    replayTrace(trace_file);

    if (toIntegrityCheck){
        integrityCheck(true);
    }
    if (toPrintWAF){
        if (hostWritePages == 0){
            printf("You did not issue any write requests. Try feed the simulator with a valid trace file first.\n");
        }else{
            printf("Host write pages: %d, GC write pages: %d, WAF: %.04f\n", hostWritePages, realGCPages, (double)(hostWritePages + realGCPages) / hostWritePages);
        }
    }
    return 0;
}
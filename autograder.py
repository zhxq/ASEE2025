#!/usr/bin/env python3
import argparse
import subprocess
import os, sys

# https://stackoverflow.com/questions/287871/how-do-i-print-colored-text-to-the-terminal
class bcolors:
    OKLIGHTRED = '\033[91m'
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class ssdlabGrader():
    def wafCheck(self, path, timeout, trace):
        print('Running... this could take a while.')
        p1 = subprocess.Popen([f'./ssdlab -w -t {trace}'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out1, err1 = p1.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p1.kill()
        if p1.returncode != 0 and p1.returncode != 2:
            print(out1.decode('utf-8'))
            print(err1.decode('utf-8'))
            print("Your implementation did not quit as expected. Exiting.")
            quit(1)
        out1 = out1.decode('utf-8').strip().split('\n')
        
        p2 = subprocess.Popen([f'./ssdlab-ref -w -t {trace}'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out2, err2 = p2.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p2.kill()
        if p2.returncode != 0 and p2.returncode != 2:
            print(out2.decode('utf-8'))
            print(err2.decode('utf-8'))
            print("Cannot run the reference file. Exiting.")
            quit(1)
        out2 = out2.decode('utf-8').strip().split('\n')

        studentWAF = -1
        referenceWAF = -1
        for i in out1:
            if ('Host write pages:' in i):
                studentWAF = float(i.split(' ')[-1])
        if studentWAF < 0:
            print('\n'.join(out1))
            print('Your implementation cannot run correctly. Cannot get WAF information.')
            print('If there is an assertion error, you will get 0% for this part.')
            quit(1)

        for i in out2:
            if ('Host write pages:' in i):
                referenceWAF = float(i.split(' ')[-1])
        if referenceWAF < 0:
            print('\n'.join(out2))
            print('The reference implementation cannot run correctly. Cannot get WAF information.')
            quit(1)

        print(f"Your WAF: {studentWAF}")
        print(f"Reference WAF: {referenceWAF}")
        print("")
        print("The reference file has a VERY good WAF.")
        print("If you can achieve this WAF, it is likely you will get a good score for this part.")
        print("Remember, the performance for the reference is just a reference,")
        print("Your score will be determined by the lowest WAF of your classmates.")
    def integrityCheck(self, path, timeout, trace):
        print('Running... this could take a while.')
        p1 = subprocess.Popen([f'./ssdlab -i -t {trace}'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out1, err1 = p1.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p1.kill()
        if p1.returncode != 0 and p1.returncode != 2:
            print(out1.decode('utf-8'))
            print(err1.decode('utf-8'))
            print("Your implementation did not quit as expected. Exiting.")
            quit(1)
        out1 = out1.decode('utf-8').split('\n')
        p2 = subprocess.Popen([f'./ssdlab-ref -i -t {trace}'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out2, err2 = p2.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p2.kill()
        if p2.returncode != 0 and p2.returncode != 2:
            print(out2.decode('utf-8'))
            print(err2.decode('utf-8'))
            print("Cannot run the reference file. Exiting.")
            quit(1)
        out2 = out2.decode('utf-8').split('\n')
        # Check results
        studentValidLPNEntries = -1
        referenceValidLPNEntries = -1
        studentValidPPNEntries = -1
        referenceValidPPNEntries = -1
        assertionError = False
        integrityError = False
        for i in out1:
            if ('Assertion error at function' in i):
                assertionError = True
            if ('Integrity error at function' in i):
                integrityError = True
            if ('Correct mapping table entries' in i):
                studentValidLPNEntries = int(i.split(' ')[-1])
            if ('Correct reverse mapping table entries' in i):
                studentValidPPNEntries = int(i.split(' ')[-1])
        if studentValidLPNEntries < 0:
            print('\n'.join(out1))
            print('Your implementation cannot run correctly. Cannot get the number of valid mapped entries.')
            quit(1)
        if studentValidPPNEntries < 0:
            print('\n'.join(out1))
            print('Your implementation cannot run correctly. Cannot get the number of valid reverse mapped entries.')
            quit(1)
        
        for i in out2:
            if ('Correct mapping table entries' in i):
                referenceValidLPNEntries = int(i.split(' ')[-1])
            if ('Correct reverse mapping table entries' in i):
                referenceValidPPNEntries = int(i.split(' ')[-1])
        if referenceValidLPNEntries < 0:
            print('\n'.join(out2))
            print('The reference implementation cannot run correctly. Cannot get the number of total mapped entries.')
            quit(1)
        if referenceValidPPNEntries < 0:
            print('\n'.join(out1))
            print('The reference implementation cannot run correctly. Cannot get the number of valid reverse mapped entries.')
            quit(1)
        
        print(f"Your implementation has {bcolors.OKCYAN}{studentValidLPNEntries}{bcolors.ENDC} correct mapping table entries and {bcolors.OKCYAN}{studentValidPPNEntries}{bcolors.ENDC} correct reverse mapping table entries.")
        print(f"The reference implementation has {bcolors.OKCYAN}{referenceValidLPNEntries}{bcolors.ENDC} mapping table entries.")


        lpnScore = (1 - (abs(studentValidLPNEntries - referenceValidLPNEntries) / referenceValidLPNEntries)) * 40
        ppnScore = (1 - (abs(studentValidPPNEntries - referenceValidPPNEntries) / referenceValidPPNEntries)) * 40
        if assertionError:
            print(f'{bcolors.OKLIGHTRED}Assertion error detected. 50% penalty.{bcolors.ENDC}')
            lpnScore *= 0.5
            ppnScore *= 0.5
        if integrityError:
            print(f'{bcolors.OKLIGHTRED}Integrity error detected. 25% penalty.{bcolors.ENDC}')
            lpnScore *= 0.75
            ppnScore *= 0.75
        print(f'{bcolors.OKGREEN}Score for the mapping table: {round(lpnScore, 2)}/40{bcolors.ENDC}')
        print(f'{bcolors.OKGREEN}Score for the reverse mapping table: {round(ppnScore, 2)}/40{bcolors.ENDC}')
        print(f'{bcolors.OKGREEN}Total: {round(lpnScore + ppnScore, 2)}/80{bcolors.ENDC}')
        if (studentValidLPNEntries == referenceValidLPNEntries and studentValidPPNEntries == referenceValidPPNEntries and studentValidLPNEntries == studentValidPPNEntries and not assertionError):
            print('Congratulations, your (reverse) mapping table management integrity is correct.')
        else:
            print('The number valid entries in the mapping table and the reverse mapping table do not match.')
            print('Please check your (reverse) mapping table management.')
            print('Did you implemented the handleWriteRequests() correctly?')
            print('If you are sure handleWriteRequests() is implemented correctly,')
            print('you need to make sure gc() and findVictim() are implemented correctly.')
            print('Remember, you can use the -g flag to compile with the reference gc() function provided by the instructors;')
            print('you can also use the -f flag to compile with the reference findVictim() function.')
            print('You can use both flags together to compile with both reference functions provided by the instructors.')
    def make(self, path, timeout=60, usesamplegc = False, usesamplefindvictim = False):
        makeArgs = 'make'
        p1 = subprocess.Popen([f'make clean'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out1, err1 = p1.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p1.kill()
        
        if usesamplegc and usesamplefindvictim:
            print(bcolors.WARNING + 'CAUTION: your code is compiled with sample findVictim() and gc().' + bcolors.ENDC)
            makeArgs += ' withsamplefindvictimandgc'
        elif usesamplegc:
            print(bcolors.WARNING + 'CAUTION: your code is compiled with sample gc().' + bcolors.ENDC)
            makeArgs += ' withsamplegc'
        elif usesamplefindvictim:
            print(bcolors.WARNING + 'CAUTION: your code is compiled with sample findVictim().' + bcolors.ENDC)
            makeArgs += ' withsamplefindvictim'
        p2 = subprocess.Popen([makeArgs], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out2, err2 = p2.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p2.kill()
        if p2.returncode != 0:
            print("Your code does not compile. Exiting.")
            quit(1)
    def grade(self, path, timeout, trace):
        self.make(path, timeout)
        self.integrityCheck(path, timeout, trace)
        self.wafCheck(path, timeout, trace)
    def __init__(self):
        pass


if __name__ == '__main__':
    path = './'
    timeout = 6000
    vg = ssdlabGrader()
    parser = argparse.ArgumentParser(description='An autograder for your convenience.')
    parser.add_argument('-t', '--trace', dest='tracefile', type=str, nargs=1, default=[], action='append', 
                    help=f'Load the given trace file.\n For example, {sys.argv[0]} -t traces/systor17-1.trace will read traces/systor17-1.trace')
    parser.add_argument('-i', '--integrity', dest='integrity', action='store_true', 
                    help='Check the integrity of your (reverse) mapping table.')
    parser.add_argument('-g', '--sample-gc', dest='samplegc', action='store_true', 
                    help='Use the sample GC function provided.')
    parser.add_argument('-f', '--sample-findvictim', dest='samplefindvictim', action='store_true', 
                    help='Use the sample findVictim function provided.')
    parser.add_argument('-w', '--waf', dest='waf', action="store_true",  
                    help='Check the WAF of your work.')
    args = parser.parse_args()

    if len(args.tracefile) == 0:
        parser.print_help()
        print("You have to provide a trace file using -t flag. Exiting.")
        quit(1)
    if not os.path.exists(args.tracefile[0][0]):
        parser.print_help()
        print(f"Trace file {args.tracefile[0][0]} does not exist. Exiting.")
        quit(1)
    vg.make(path, timeout, args.samplegc, args.samplefindvictim)
    if (args.integrity):
        vg.integrityCheck(path, timeout, args.tracefile[0][0])
    if (args.waf):
        vg.wafCheck(path, timeout, args.tracefile[0][0])
    if (not args.integrity) and (not args.waf):
        parser.print_help()
        print("You also need to use other arguments than just -t to see the results of your work.")
        
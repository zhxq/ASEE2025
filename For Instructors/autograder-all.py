#!/usr/bin/env python3
import random
import subprocess
import os
import shutil
from datetime import datetime

# We assume the following list of student home folders are located under /home/ directory
# and all students submit their files (using the submission script) to the 
# turnin folder under their home folder
# e.g., /home/student1/turnin

# Please provide the list of students here
# E.g., the following list will read /home/provide/turnin, /home/the/turnin/, ... for students' implementation
allStudentUsernames = ['provide', 'the', 'path', 'to', 'student', 'folders']
# Change the deadline so it can penalize late homework submissions
homeworkDeadlineDate = "2023-12-10"

# Select traces for grading. Can be left as is.
gradeTraces = ['./traces/systor17-1.trace', './traces/systor17-2.trace', './traces/systor17-3.trace', './traces/systor17-4.trace', './traces/systor17-5.trace', './traces/systor17-6.trace']

# Penalty for late homework submissions (e.g., 0.2 = 20% penalty for each late day):
latePenaltyPerDay = 0.2


# The final score will be saved at ./scores.csv.

os.chdir(os.path.dirname(__file__))
scriptLoc = os.getcwd()
class AutoGrader(object):
    def grading(self, username, path, timeout=5, traces=['traces/long1.trace']):
        self.make(path, timeout)
        invoke = self.invoke(username, path, timeout, traces)
        return invoke
    def make(self, path, timeout=5):
        p1 = subprocess.Popen([f'make clean'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out1, err1 = p1.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p1.kill()
        p2 = subprocess.Popen([f'make'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            out2, err2 = p2.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p2.kill()
    def invoke(self, username, path, timeout=5, traces=['traces/long1.trace']):
        assertionError = False
        integrityError = False
        
        scoreInfo = []
        for t in traces:
            firstScore = 0
            secondScore = 0
            p1 = subprocess.Popen([f'./autograder.py -w -i -t {t}'], cwd=path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            try:
                out1, err1 = p1.communicate(timeout=timeout)
            except subprocess.TimeoutExpired:
                p1.kill()
            if out1 != None:
                out1 = out1.decode('utf-8').split('\n')
                for l in out1:
                    if 'Total: ' in l:
                        print(l)
                        firstScore = float(l.split(' ')[1].split('/')[0])
                    if 'Assertion error detected' in l:
                        assertionError = True
                    if 'Integrity error detected' in l:
                        integrityError = True
                    if 'Your WAF: ' in l:
                        secondScore = float(l.split(' ')[2])
                scoreInfo.append(firstScore)
                scoreInfo.append(secondScore)
                if assertionError:
                    scoreInfo += [0.5, 0]
                elif integrityError:
                    scoreInfo += [0.75, 1]
                else:
                    scoreInfo += [1, 1]
            else:
                scoreInfo += [0, 0, 0, 0]
        return [username] + scoreInfo
            
    def getFolders(self, path = "/home/", files = [], tmpLoc = "/tmp/", ddl="1970-01-01"):
        randPath = ""
        while True:
            randPath = f"{tmpLoc}{os.sep}grading{random.randint(10000000, 99999999)}"
            if os.path.exists(randPath):
                continue
            else:
                os.mkdir(randPath)
                break
        ddl = datetime.strptime(ddl, "%Y-%m-%d")
        
        print(f"Random path: {randPath}")
        allPaths = os.listdir(path)
        allPaths = list(set(allPaths).intersection(allStudentUsernames))
        allPaths.sort()
        tableHeader = ["Name"]
        allResults = []
        for hf in allPaths:
            latestTS = 0
            studentRandPath = randPath + os.sep + hf
            studentSrcPath = path + os.sep + hf + os.sep + "turnin"
            shutil.copytree(f"{scriptLoc}{os.sep}", studentRandPath)
            if os.path.isdir(studentSrcPath):
                # path exists
                # copy files to current directory
                for f in files:
                    if f == '':
                        print('Filename is empty, exiting.')
                        exit()
                    srcFilePath = studentSrcPath + os.sep + f
                    dstFilePath = studentRandPath + os.sep + f
                    if os.path.exists(srcFilePath):
                        tempTS = os.path.getmtime(srcFilePath)
                        if tempTS > latestTS:
                            latestTS = tempTS
                        shutil.copyfile(srcFilePath, dstFilePath)
            studentLatestEdit = datetime.fromtimestamp(latestTS)
            diffDates = studentLatestEdit - ddl
            totalPct = 1
            result = self.grading(hf, studentRandPath, traces=gradeTraces, timeout=60000)
            if (diffDates.days > 0):
                print(f'{hf} is late for {diffDates.days} days')
                totalPct -= diffDates.days * latePenaltyPerDay
                if totalPct < 0:
                    totalPct = 0
                print("Original: ")
                print(result)
            result.append(round(totalPct, 2))
            for t in gradeTraces:
                tableHeader += [f"{t} Integrity Score", f"{t} WAF", f"{t} Error Penalty", f"{t} WAF Calculated"]
            allResults.append(','.join([i for i in tableHeader]) + '\n')
            allResults.append(','.join([str(i) for i in result]) + '\n')
            print(result)
        with open("./scores.csv", "w") as f:
            f.writelines(allResults)
        print(f'Removing {randPath} grading folder')
        shutil.rmtree(randPath, ignore_errors=True)
a = AutoGrader()
a.getFolders(files=['gc.c', 'handlewrite.c', 'findvictim.c'], ddl=homeworkDeadlineDate)
# SSDLab: Filling in the Missing Piece: Integrating Storage into CompOrg Courses

This repo contains source code template for `SSDLab` presented in the paper _Filling in the Missing Piece: Integrating Storage into CompOrg Courses_, published at ASEE 2025. Link to the paper will be updated after the paper is published.

## For Instructors

### Required Environments

System: Linux. Each student should be assigned to a folder under `/home/`.
Toolchain for the assignment: `gcc`, `objcopy`.
Handout: Any LaTeX compiler.

### Instructor-only Files

`For Instructors` folder contains all files required for instructors.

- `ssdlab.tex`: The LaTeX source code for the handout.
- `autograder-all.py`: The source code for collecting and grading all students' assignments. `sudo` is required to copy from all students' home folders.
  - Before using this script, the instructor should first copy the script to a directory with a clean copy of SSDLab code template (e.g., copy this file to the root of this project and run). The script will create a temporary folder under `/tmp/` and copy all files in its current folder to the temp folder to provide the environment for `make`-ing the students' code. Each student will have a different temp folder. The script will then automatically copy all students' implementation to the temp folder and grade.
  - Please remember to change `allStudentUsernames`, `homeworkDeadlineDate`, and `latePenaltyPerDay` at the top of the script to set student username (which will be used to determine the locations of student's submissions), homework deadline, and late penalty, respectively.
  - Please make sure the students submit their assignments to the `turnin` folder under their home directory (This will be automatic if the students use the `submission` script under the source code folder to submit their assignment).
  - The output will be saved at `./scores.csv`.

The columns for `./scores.csv` is as follows:

1. Student's user name.
2. Trace 1's correctness score (Max: 80). Assertion error results in 50% penalty and will not proceed to calculate WAF, and integrity error results in 25% penalty (but will proceed to calculate WAF).
3. WAF for Trace 1.
4. Percentage received for correctness when running Trace 1 (see column 2).
5. Attempted to calculate WAF for Trace 1 (1=Yes, 0=No).
6. Same as Column 2, but for Trace 2.
7. Same as Column 3, but for Trace 2.
8. Same as Column 4, but for Trace 2.
9. Same as Column 5, but for Trace 2.
10. ...

Instructors should remove the `For Instructors` folder before deploying the assignment.

## Files for Students

Students should make changes to the following files:

- `handlewrite.c`: Students should make change to the `handleWriteRequests` function.
- `gc.c`: Students should make change to the `gc` function.
- `findvictim.c`: Students should make change to the `findVictim` function.

Students can use the autograder file (`autograder.py`) to grade their own assignments. Run `autograder.py` without any arguments for help.

All other files do not need to be changed by students or instructors.

- `helper.c`: Contains all helper functions. We strongly suggest students to read this file to learn how the helper functions are implemented. This file also helps students to learn C language syntax.
- `autograder.py`: Autograder for students.
- `waf.c`: Hooks on the students' implementation of the `gc` function to count the number of pages relocated in their garbage collection process.
- `main.c`: Trace driver. Reads in the trace and calls the `handleWriteRequests` for each write request in the trace.
- `Makefile`: Contains the instructions to `make` the assignment.
- `sample_findvictim.o`: The object file containing a sample answer of `findVictim` function for student debug purposes. Students can `make` with this sample `findVictim` function using `make withsamplefindvictim` before implementing their own version or to debug with it.
- `sample_gc.o`: Similar to above, but for the `gc` function. Run `make withsamplegc` to compile with `sample_gc.o`.
  - To compile with both `sample_findvictim.o` and `sample_gc.o`, run `make withsamplefindvictimandgc`.
  - The `autograder.py` script also provides the feature to compile with these object files.
- `ssdlab-ref`: The compiled `SSDLab` binary with sample answers to the three functions students need to solve. This is used by the autograder as reference.

## Acknowledgements

- The trace driver, `main.c`, is designed based on the structure of `csim.c` in CSAPP:3E. The handout is also based on CSAPP:3E assignment handout LaTeX template.
- The physical page address data structure, `ppa`, is based on the implementation of [FEMU](https://github.com/MoatLab/FEMU) by Huaicheng Li.
- The traces under the `./traces` folder are from the Systor 2017 paper _[Understanding Storage Traffic Characteristics on Enterprise Virtual Desktop Infrastructure](https://dl.acm.org/doi/10.1145/3078468.3078479)_. The traces are available at [http://iotta.snia.org/traces/block-io/4928](http://iotta.snia.org/traces/block-io/4928).

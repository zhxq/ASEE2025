all: clean gc.o
	gcc -g -o ssdlab main.c handlewrite.c gc.o findvictim.c helper.c waf.c

gc.o:
	gcc -c ./gc.c 
	objcopy --redefine-sym gc=student_gc ./gc.o

withsamplefindvictim: gc.o
	gcc -g -o ssdlab main.c handlewrite.c gc.o sample_findvictim.o helper.c waf.c

withsamplegc:
	objcopy --redefine-sym gc=student_gc ./sample_gc.o
	gcc -g -o ssdlab main.c handlewrite.c sample_gc.o findvictim.c helper.c waf.c
	
withsamplefindvictimandgc:
	objcopy --redefine-sym gc=student_gc ./sample_gc.o
	gcc -g -o ssdlab main.c handlewrite.c sample_gc.o sample_findvictim.o helper.c waf.c
clean:
	rm -rf ./ssdlab ./gc.o
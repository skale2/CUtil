all:
	gcc -c src/*.c tst/*.c
	mv *.o obj
	gcc -o bin/test obj/*.o

tester: 
	gcc -c src/test_runner.c 
	mv test_runner.o obj/test.o
	gcc -o bin/test obj/test.o

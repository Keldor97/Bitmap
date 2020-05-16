
output: main.o arg_parse.o bitmap.o
	gcc main.o arg_parse.o bitmap.o -o bitmap

main.o: main.c
	gcc -c main.c -mavx -mfma -Wall

arg_parse.o: arg_parse.c
	gcc -c arg_parse.c

bitmap.o: bitmap.c
	gcc -c bitmap.c


clean:
	rm *.o

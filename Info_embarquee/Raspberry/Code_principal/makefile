all: clean CodeCDF Pince

Pince: listechaine.o arduinoserial.o pince.o
	gcc -o Pince pince.o arduinoserial.o listechaine.o -Wall -lm -lwiringPi
#	chmod u+s ./Pince

CodeCDF: libodo.o listechaine.o arduinoserial.o main.o 
	gcc -o CodeCDF main.o libodo.o arduinoserial.o listechaine.o -Wall -lm -lwiringPi
#	chmod u+s ./CodeCDF

libodo.o: libodo.c
	gcc -o libodo.o -c libodo.c 

main.o: main.c libodo.h arduinoserial.h listechaine.h libodo.o arduinoserial.o listechaine.o
	gcc -o main.o -c main.c 

arduinoserial.o: arduinoserial.c listechaine.h listechaine.o
	gcc -o arduinoserial.o -c arduinoserial.c 

listechaine.o: listechaine.c
	gcc -o listechaine.o -c listechaine.c

pince.o: pince.c
	gcc -o pince.o -c pince.c

clean:
	rm -rf *.o

mrproper: clean
	rm -rf CodeCDF
build: myPrint.o
	echo "going to compile"
	g++ -std=c++11 -o fat main.cpp myPrint.o
	echo "finish compile"

myPrint.o: myPrint.asm
	nasm -f elf -o myPrint.o myPrint.asm

run: fat
	./fat
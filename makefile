build: myPrint.o
	@echo "going to compile main.cpp"
	@g++ -std=c++11 -o fat main.cpp myPrint.o
	@echo "finish compile main.cpp"

myPrint.o: myPrint.asm
	@echo "going to compile myPrint.asm"
	@nasm -f elf -o myPrint.o myPrint.asm
	@echo "going to compile myPrint.asm"

run: build
	@./fat

clean:
	@rm -f *.o
	@rm -f fat
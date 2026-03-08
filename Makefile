server: main.o
	clang++ -o bin/server bin/main.o -luring

main.o: src/main.cpp
	clang++ -std=c++20 -c src/main.cpp -o bin/main.o

compile_commands.json:
	compiledb make
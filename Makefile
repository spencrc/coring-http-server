CXX="clang++"

server: main.o
	$(CXX) -o bin/server bin/main.o -luring $(LDFLAGS)

main.o: src/main.cpp
	$(CXX) -std=c++20 $(CFLAGS) -c src/main.cpp -o bin/main.o

debug: src/main.cpp
	$(MAKE) CFLAGS="-g -O0 -Wall -Wextra -Wpedantic" LDFLAGS="-fsanitize=address" server

compile_commands.json:
	compiledb make
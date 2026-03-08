CC="clang"
CXX="clang++"

all: bin/server

debug: 
	$(MAKE) CFLAGS="-g -O0 -Wall -Wextra -Wpedantic" LDFLAGS="-fsanitize=address" server

run: all
	./bin/server

clean:
	rm -rf bin

################################################################
# APP
################################################################

APP_SOURCES = $(wildcard src/*.cpp)
APP_OBJECTS = $(patsubst src/%.cpp, bin/%.o, $(APP_SOURCES))

bin/server: $(APP_OBJECTS) bin/libllhttp.a
	$(CXX) -o bin/server $(APP_OBJECTS) -luring -lllhttp -Lbin $(LDFLAGS)

bin/main.o: src/main.cpp
	@mkdir -p bin
	$(CXX) -std=c++20 $(CFLAGS) -Ivendor/llhttp -c src/main.cpp -o bin/main.o

bin/%.o: src/%.cpp
	@mkdir -p bin
	$(CXX) -std=c++20 $(CFLAGS) -Ivendor/llhttp -c $< -o $@

################################################################
# LLHTTP
################################################################

LLHTTP_SOURCES = vendor/llhttp/api.c vendor/llhttp/http.c vendor/llhttp/llhttp.c
LLHTTP_OBJECTS = $(patsubst vendor/llhttp/%.c, bin/%.o, $(LLHTTP_SOURCES))

bin/libllhttp.a: $(LLHTTP_OBJECTS)
	ar rcs bin/libllhttp.a $(LLHTTP_OBJECTS)

bin/%.o: vendor/llhttp/%.c
	@mkdir -p bin
	$(CC) -c $< -o $@

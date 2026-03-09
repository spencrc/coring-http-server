CC=clang
CXX=clang++

EXE_NAME=server
OUTPUT_DIR=bin/release

all: $(OUTPUT_DIR)/$(EXE_NAME)

debug: 
	$(MAKE) CFLAGS="-g -O0 -Wall -Wextra -Wpedantic" LDFLAGS="-fsanitize=address" OUTPUT_DIR="bin/debug" all

run: all
	./$(OUTPUT_DIR)/server

clean:
	rm -rf bin

################################################################
# APP
################################################################

APP_SOURCES = $(wildcard src/*.cpp)
APP_OBJECTS = $(patsubst src/%.cpp, $(OUTPUT_DIR)/%.o, $(APP_SOURCES))
DEPENDS := $(patsubst src/%.cpp, $(OUTPUT_DIR)/%.d, $(APP_SOURCES))

$(OUTPUT_DIR)/$(EXE_NAME): $(APP_OBJECTS) $(OUTPUT_DIR)/libllhttp.a
	$(CXX) -o $(OUTPUT_DIR)/$(EXE_NAME) $(APP_OBJECTS) -luring -lllhttp -L$(OUTPUT_DIR) $(LDFLAGS)

-include $(DEPENDS)

$(OUTPUT_DIR)/%.o: src/%.cpp
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) -std=c++20 $(CFLAGS) -Ivendor/llhttp -Iinclude -MMD -MP -c $< -o $@

################################################################
# LLHTTP
################################################################

LLHTTP_SOURCES = vendor/llhttp/api.c vendor/llhttp/http.c vendor/llhttp/llhttp.c
LLHTTP_OBJECTS = $(patsubst vendor/llhttp/%.c, $(OUTPUT_DIR)/%.o, $(LLHTTP_SOURCES))

$(OUTPUT_DIR)/libllhttp.a: $(LLHTTP_OBJECTS)
	ar rcs $(OUTPUT_DIR)/libllhttp.a $(LLHTTP_OBJECTS)

$(OUTPUT_DIR)/%.o: vendor/llhttp/%.c
	@mkdir -p $(OUTPUT_DIR)
	$(CC) -c $< -o $@

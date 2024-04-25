
# compiler
CC := gcc

# other variables
OUTPUT := tst.e
CC_FLAGS := -g -O3 -Wall -Wextra -Wpedantic 
LD_FLAGS := -O3 -lcurl

SRC_FILES := $(wildcard src/*.c)
H_FILES := $(wildcard src/*.h)
OBJ_FILES := $(SRC_FILES:.c=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJ_FILES) Makefile
	$(CC) $(OBJ_FILES) $(H_FILES) $(LD_FLAGS) -o ./bin/$@

.c.o:
	$(CC) -c $(CC_FLAGS) $< -o $@

clean:
	rm -f $(OBJ_FILES) ./bin/tst.e

run: $(OUTPUT)
	cd bin && ./tst.e && cd ..

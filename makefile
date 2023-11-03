#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServer


CUSTOM_INCLUDE_PATH_TMP := $(shell find ./sources ! -path '*/.git/*' -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
CUSTOM_INCLUDE_PATH := $(addprefix -I,$(CUSTOM_INCLUDE_PATH_TMP))



# .c files
C_SOURCE := $(shell find ./sources  ! -path '*/.git/*' ! -path '*/tests/*' -name '*.cpp' -or -name '*.c' -or -name '*.s')

# .h files
H_SOURCE := $(shell find ./sources  ! -path '*/.git/*' ! -path '*/tests/*' -name '*.hpp' -or -name '*.h' -or -name '*.inc')


prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear
	@ mkdir ./build | true
	@ cp -r ./sources/assets/* ./build | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst ./sources,./build/objects,$(C_SOURCE)))
# Compiler and linker
#CC=g++
CC=clang++


CC_FLAGS=-c			\
		-pedantic  \
		-pthread   \
		-std=c++20 \
		-lssl      \
		-lcrypto   \
		$(CUSTOM_INCLUDE_PATH)


LK_FLAGS=-pedantic  \
		-pthread   \
		-std=c++20 \
		-lssl      \
		-lcrypto   \
		$(CUSTOM_INCLUDE_PATH)
#
# Compilation and linking
#
 

 
# Command used at clean target
RM = rm -rf
 
debug: CC=g++
debug: CC_FLAGS+=-g
debug: CC_FLAGS+=-ggdb
debug: LK_FLAGS+=-g
debug: LK_FLAGS+=-ggdb
debug: prebuild $(PROJ_NAME)
	
all: CC_FLAGS+=-O3
all: LK_FLAGS+=-O3
all: prebuild $(PROJ_NAME)
 
$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) $^ $(LK_FLAGS) -o build/$@
	@ echo 'Finished building binary: $@'
	@ echo ' '
 
./build/objects/%.o: ./sources/%.cpp ./sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./build/objects/main.o: ./sources/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ $(RM) ./build/objects/*.o $(PROJ_NAME) *~
	@ rm -rf ./build/objects
 
.PHONY: all clean

#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
BIN_NAME=vss


CUSTOM_INCLUDE_PATH_TMP := $(shell find ./sources ! -path '*/.git/*' -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
CUSTOM_INCLUDE_PATH := $(addprefix -I,$(CUSTOM_INCLUDE_PATH_TMP))



#get files using dectect_include_files.sh to prevent multiple definion errors with submodules

# .c files


C_SOURCE := $(shell ./makefile.aux/detect_include_files.sh "withAlternativeExtension cpp" "withCheckOfFilesExistenceAfterExtensionChange")

# .h files
H_SOURCE := $(shell ./makefile.aux/detect_include_files.sh "withAlternativeExtension h" "withCheckOfFilesExistenceAfterExtensionChange")
 	

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear | true
	@ mkdir ./build | true
	@ cp -r ./sources/assets/* ./build >/dev/null 2>&1 | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst ./sources,./build/objects,$(C_SOURCE)))
# Compiler and linker
#CC=g++
CC=clang++

#		-pedantic
CC_FLAGS=-c			\
		-pthread   \
		-std=c++20 \
		-lssl      \
		-lcrypto   \
		$(CUSTOM_INCLUDE_PATH)

#		-pedantic 
LK_FLAGS= -pthread   \
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
debug: prebuild $(BIN_NAME)
	
all: CC_FLAGS+=-O3
all: LK_FLAGS+=-O3
all: prebuild $(BIN_NAME)
 
$(BIN_NAME): $(OBJ)
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

install:
	@ makefile.aux/install.sh __fomrMakeFile__ install "$(BIN_NAME)"

uninstall:
	@ makefile.aux/install.sh __fomrMakeFile__ uninstall "$(BIN_NAME)"

purge:
	@ makefile.aux/install.sh __fomrMakeFile__ uninstall "$(BIN_NAME)"
	
clean:
	@ $(RM) ./build/objects/*.o $(BIN_NAME)
	@ rm -rf ./build/objects
 
.PHONY: all clean

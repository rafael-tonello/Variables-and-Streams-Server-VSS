#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=tests

CUSTOM_INCLUDE_PATH_TMP1 := <mainIncludePaths>

CUSTOM_INCLUDE_PATH_TMP2 := <testsIncludePaths>
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
CUSTOM_INCLUDE_PATH := $(addprefix -I,$(CUSTOM_INCLUDE_PATH_TMP2))


# .cpp files
C_SOURCE_TMP := <mainCppFiles>
C_SOURCE := <testsCppFiles> $(C_SOURCE_TMP)

# .h files
H_SOURCE_TMP := <mainHFiles>
H_SOURCE := <testsHFiles> $(H_SOURCE_TMP)
$(warning $(H_SOURCE))

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
#	@ clear
	@ mkdir ./build | true
	@ cp -r ./sources/assets/* ./build >/dev/null 2>&1 | true
 
# Object files
temp:=$(subst ../sources,./build/objects/main,$(C_SOURCE))
temp:=$(subst ./sources,./build/objects/tests,$(temp))

OBJ=$(subst .cpp,.o,$(temp))
 
# Compiler and linker
#CC=g++
CC=clang++


#		-pedantic
CC_FLAGS=-D__TESTING__\
        <ccFlags> \
		$(CUSTOM_INCLUDE_PATH)



#		-pedantic
LK_FLAGS=-D__TESTING__\
        <lkFlags>   \
		$(CUSTOM_INCLUDE_PATH)
#
# Compilation and linking
#
 

 
# Command used at clean target
RM = rm -rf
 
debug: CC=g++
debug: CC_FLAGS+=-g
debug: LK_FLAGS+=-g
debug: CC_FLAGS+=-ggdb
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
 
./build/objects/main/%.o: ../sources/%.cpp ../sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

./build/objects/tests/%.o: ./sources/%.cpp ./sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./build/objects/tests/main.o: ./sources/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ rm -rf ./build/objects
 
.PHONY: all clean
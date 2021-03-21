#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServerTests
# .c files
C_SOURCE=$(wildcard ./source/*.cpp) \
		 $(wildcard ./source/Shared/Misc/*.cpp) \
		 $(wildcard ./source/Shared/ThreadPool/*.cpp)
 
# .h files
H_SOURCE=$(wildcard ./source/*.h) \
		 $(wildcard ./source/Shared/Misc/*.h) \
		 $(wildcard ./source/Shared/ThreadPool/*.h)

objFolder:
	@ mkdir -p objects
	@ mkdir -p objects/Shared/Misc
	@ mkdir -p objects/Shared/ThreadPool
 
# Object files
OBJ=$(subst .cpp,.o,$(subst source,objects,$(C_SOURCE)))
 
# Compiler and linker
CC=g++
 
# Flags for compiler
CC_FLAGS=-c         \
		 -g         \
		 -O2        \
		 -ggdb      \
         -W         \
         -Wall      \
         -pedantic  \
		 -std=c++11 \
		 -lpthread 

LINK_FLAGS=-W       \
         -Wall      \
         -pedantic  \
		 -std=c++11 \
		 -lpthread 
 
# Command used at clean target
RM = rm -rf 
 
#
# Compilation and linking
#
all: objFolder $(PROJ_NAME)
 
$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) $^ $(LINK_FLAGS) -o ./build/$@
	@ echo 'Finished building binary: $@'
	@ echo ' '
 
./objects/%.o: ./source/%.cpp ./source/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./objects/main.o: ./source/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ $(RM) ./objects/*.o $(PROJ_NAME) *~
	@ rm -rf objects
 
.PHONY: all clean



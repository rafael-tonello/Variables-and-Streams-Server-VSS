#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServer
# .c files
C_SOURCE=$(wildcard ./source/*.cpp) \
		 $(wildcard ./source/fs_integration/*.cpp) \
		 $(wildcard ./source/Shared/TCPServer/*.cpp) \
		 $(wildcard ./source/Shared/ThreadPool/*.cpp) \
		 $(wildcard ./source/Controller/*.cpp) \
		 $(wildcard ./source/Services/VarSystem/*.cpp)
 
# .h files
H_SOURCE=$(wildcard ./source/*.h) \
		 $(wildcard ./source/fs_integration/*.h) \
		 $(wildcard ./source/Shared/TCPServer/*.h) \
		 $(wildcard ./source/Shared/ThreadPool/*.h) \
		 $(wildcard ./source/Controller/*.h) \
		 $(wildcard ./source/Services/VarSystem/*.h)

objFolder:
	@ mkdir -p objects
	@ mkdir -p objects/fs_integration
	@ mkdir -p objects/sci_projects
	@ mkdir -p objects/Shared/TCPServer
	@ mkdir -p objects/Shared/ThreadPool
	@ mkdir -p objects/Controller
	@ mkdir -p objects/Services/VarSystem
 
# Object files
OBJ=$(subst .cpp,.o,$(subst source,objects,$(C_SOURCE)))
 
# Compiler and linker
CC=g++
 
# Flags for compiler
CC_FLAGS=-c         \
         -W         \
         -Wall      \
         -ansi      \
         -pedantic  \
		 -std=c++11
 
# Command used at clean target
RM = rm -rf
 
#
# Compilation and linking
#
all: objFolder $(PROJ_NAME)
 
$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) $^ -o $@
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

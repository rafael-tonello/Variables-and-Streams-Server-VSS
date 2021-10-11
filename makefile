#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServer
# .c files
C_SOURCE=$(wildcard ./source/*.cpp)
C_SOURCE+=$(wildcard ./source/Controller/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ./source/Services/APIs/*.cpp)
C_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/DependencyInjectionManager/*.cpp)
 
# .h files
H_SOURCE=$(wildcard ./source/*.h)
H_SOURCE+=$(wildcard ./source/Controller/*.h)
H_SOURCE+=$(wildcard ./source/Shared/ThreadPool/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ./source/Services/APIs/*.h)
H_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ./source/Shared/DependencyInjectionManager/*.h)


objFolder:
	@ mkdir -p objects
	@ mkdir -p objects/Controller
	@ mkdir -p objects/Shared/ThreadPool
	@ mkdir -p objects/Shared/Misc
	@ mkdir -p objects/Services/APIs
	@ mkdir -p objects/Services/APIs/PHOMAU
	@ mkdir -p objects/Shared/DependencyInjectionManager

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ mkdir ./build | true
	@ cp -r ./source/copyToBuildFolder/* ./build | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst source,objects,$(C_SOURCE)))
 
# Compiler and linker
CC= g++
 
# Flags for compiler
CC_FLAGS=-c			\
		 -W         \
         -Wall      \
         -ansi      \
         -pedantic  \
		 -pthread   \
		 -lssl		\
		 -lcrypto	\
		 -g			\
		 -std=c++11 \
		 -I"./source/"

# Flags for linker
LK_FLAGS=-W         \
         -Wall      \
         -ansi      \
         -pedantic  \
		 -pthread   \
		 -lssl		\
		 -lcrypto	\
		 -g			\
		 -D__TESTING__ \
		 -std=c++11 \
		 -I"./source/"
 
# Command used at clean target
RM = rm -rf
 
#
# Compilation and linking
#
all: objFolder prebuild $(PROJ_NAME)
 
$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) $^ $(LK_FLAGS) -o build/$@
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

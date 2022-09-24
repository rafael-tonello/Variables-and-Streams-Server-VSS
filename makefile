#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServer
CUSTOM_INCLUDE_PATH = -I"./sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Controller"
CUSTOM_INCLUDE_PATH += -I"./sources/Controller/Internal"
CUSTOM_INCLUDE_PATH += -I"./sources/Services"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/Apis"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/Storage"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Misc"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Confs"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Confs/internal"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/DependencyInjectionManager"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Logger/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/TCPServer"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/ThreadPool"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/VarSystem"

# .c files
C_SOURCE=$(wildcard ./sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/Internal/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/Storage/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/Storage/VarSystemLib/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/TCPServer/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/VarSystem/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Misc/*.cpp)
 
# .h files
H_SOURCE=$(wildcard ./sources/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/Internal/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ./sources/Services/Storage/*.h)
H_SOURCE+=$(wildcard ./sources/Services/Storage/VarSystemLib/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/TCPServer/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/ThreadPool/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/VarSystem/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Misc/*.h)

objFolder:
	@ mkdir -p objects/Controller/Internal
	@ mkdir -p objects/Services/APIs/PHOMAU
	@ mkdir -p objects/Services/Storage/VarSystemLib
	@ mkdir -p objects/Shared/Libs/Confs/internal
	@ mkdir -p objects/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/Shared/Libs/DynamicVar
	@ mkdir -p objects/Shared/Libs/Logger/writers
	@ mkdir -p objects/Shared/Misc
	@ mkdir -p objects/Shared/Libs/ThreadPool
	@ mkdir -p objects/Shared/Libs/TCPServer
	@ mkdir -p objects/Shared/Libs/VarSystem

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear
	@ mkdir ./build | true
	@ cp -r ./sources/copyToBuildFolder/* ./build | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst ./sources,./objects,$(C_SOURCE)))
 
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
		 -I"./sources/" \
		 $(CUSTOM_INCLUDE_PATH)

# Flags for linker
LK_FLAGS=-W         \
         -Wall      \
         -ansi      \
         -pedantic  \
		 -pthread   \
		 -lssl		\
		 -lcrypto	\
		 -g			\
		 -std=c++11 \
		 -I"./sources/" \
		 $(CUSTOM_INCLUDE_PATH)
 
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
 
./objects/%.o: ./sources/%.cpp ./sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./objects/main.o: ./sources/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ $(RM) ./objects/*.o $(PROJ_NAME) *~
	@ rm -rf objects
 
.PHONY: all clean

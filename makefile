#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
PROJ_NAME=VarServer
CUSTOM_INCLUDE_PATH = -I"./sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Controller"
CUSTOM_INCLUDE_PATH += -I"./sources/Controller/Internal"
CUSTOM_INCLUDE_PATH += -I"./sources/Services"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/APIs"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/APIs/VSTP"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/APIs/http"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/APIs/http/varsexporters"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/Storage"
CUSTOM_INCLUDE_PATH += -I"./sources/Services/ServerDiscovery"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Misc"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/DynamicVar/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Logger/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Logger/sources/writers"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/json_maker/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Confs"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/Confs/internal"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/DependencyInjection/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/TCPServer/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/ThreadPool"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/VarSystem"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/MessageBus"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/KwWebServer/sources"
CUSTOM_INCLUDE_PATH += -I"./sources/Shared/Libs/KwWebServer/sources/Workers"



# .c files
C_SOURCE=$(wildcard ./sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/Internal/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/VSTP/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/http/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/http/varsexporters/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/Storage/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/Storage/VarSystemLib/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/ServerDiscovery/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/json_maker/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjection/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/TCPServer/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/VarSystem/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/MessageBus/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/KwWebServer/sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/KwWebServer/sources/Workers/*.cpp)

# .h files
H_SOURCE=$(wildcard ./sources/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/Internal/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/VSTP/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/http/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/http/varsexporters/*.h)
H_SOURCE+=$(wildcard ./sources/Services/Storage/*.h)
H_SOURCE+=$(wildcard ./sources/Services/Storage/VarSystemLib/*.h)
H_SOURCE+=$(wildcard ./sources/Services/ServerDiscovery/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/json_maker/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjection/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DynamicVar/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Logger/sources/writers/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/TCPServer/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/ThreadPool/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/VarSystem/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/MessageBus/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/KwWebServer/sources/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/KwWebServer/sources/Workers/*.h)

objFolder:
	@ mkdir -p objects/Controller/Internal
	@ mkdir -p objects/Services/APIs/VSTP
	@ mkdir -p objects/Services/APIs/http/varsexporters
	@ mkdir -p objects/Services/Storage/VarSystemLib
	@ mkdir -p objects/Services/ServerDiscovery
	@ mkdir -p objects/Shared/Misc
	@ mkdir -p objects/Shared/Libs/DynamicVar/sources
	@ mkdir -p objects/Shared/Libs/Logger/sources/writers
	@ mkdir -p objects/Shared/Libs/json_maker/sources
	@ mkdir -p objects/Shared/Libs/Confs/internal
	@ mkdir -p objects/Shared/Libs/DependencyInjection/sources
	@ mkdir -p objects/Shared/Libs/DynamicVar
	@ mkdir -p objects/Shared/Libs/ThreadPool
	@ mkdir -p objects/Shared/Libs/TCPServer/sources
	@ mkdir -p objects/Shared/Libs/VarSystem
	@ mkdir -p objects/Shared/Libs/MessageBus
	@ mkdir -p objects/Shared/Libs/KwWebServer/sources/Workers

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
#		 -W         
#         -Wall      
CC_FLAGS=-c			\
         -ansi      \
         -pedantic  \
		 -pthread   \
		 -g			\
		 -std=c++17 \
		 -I"./sources/" \
		 -lssl      \
		 -lcrypto   \
		 $(CUSTOM_INCLUDE_PATH)

# Flags for linker
#		 -W         
#         -Wall      
LK_FLAGS=-ansi      \
         -pedantic  \
		 -pthread   \
		 -g			\
		 -std=c++17 \
		 -I"./sources/" \
		 -lssl      \
		 -lcrypto   \
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

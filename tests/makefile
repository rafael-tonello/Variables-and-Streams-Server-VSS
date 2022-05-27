 # My third makefile
 
# Name of the project
PROJ_NAME=tests
# .c files

#teste app includes
CUSTOM_INCLUDE_PATH = -I"./"

#main app includes
CUSTOM_INCLUDE_PATH += -I"../sources/Controller"
CUSTOM_INCLUDE_PATH += -I"../sources/Controller/Internal"
CUSTOM_INCLUDE_PATH += -I"../sources/Services"
CUSTOM_INCLUDE_PATH += -I"../sources/Services/Apis"
CUSTOM_INCLUDE_PATH += -I"../sources/Services/Apis/PHOMAU"
CUSTOM_INCLUDE_PATH += -I"../sources/Services/Storage"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Misc"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/Confs"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/Confs/internal"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/DependencyInjectionManager"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/logger"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/logger/writers"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/TCPServer"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/ThreadPool"
CUSTOM_INCLUDE_PATH += -I"../sources/Shared/Libs/VarSystem"



#test app files
C_SOURCE=$(wildcard ./sources/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/*.cpp)
C_SOURCE+=$(wildcard ./sources/Controller/Internal/*.cpp)
C_SOURCE+=$(wildcard ./sources/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.cpp)

#main app files
C_SOURCE+=$(wildcard ../sources/Controller/*.cpp)
C_SOURCE+=$(wildcard ../sources/Controller/Internal/*.cpp)
C_SOURCE+=$(wildcard ../sources/Services/*.cpp)
C_SOURCE+=$(wildcard ../sources/Services/Apis/*.cpp)
C_SOURCE+=$(wildcard ../sources/Services/Apis/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ../sources/Services/Storage/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/logger/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/logger/writers/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/TCPServer/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ../sources/Shared/Libs/VarSystem/*.cpp)
  
# .h files
#test app files
H_SOURCE=$(wildcard ./sources/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/*.h)
H_SOURCE+=$(wildcard ./sources/Controller/Internal/*.h)
H_SOURCE+=$(wildcard ./sources/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ./sources/Shared/Libs/Confs/internal/*.h)

#main app files
H_SOURCE+=$(wildcard ../sources/Controller/*.h)
H_SOURCE+=$(wildcard ../sources/Controller/Internal/*.h)
H_SOURCE+=$(wildcard ../sources/Services/*.h)
H_SOURCE+=$(wildcard ../sources/Services/Apis/*.h)
H_SOURCE+=$(wildcard ../sources/Services/Apis/PHOMAU/*.h)
H_SOURCE+=$(wildcard ../sources/Services/Storage/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/Confs/internal/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/logger/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/logger/writers/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/TCPServer/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/ThreadPool/*.h)
H_SOURCE+=$(wildcard ../sources/Shared/Libs/VarSystem/*.h)

objFolder:
	@ mkdir -p objects/tests/
	@ mkdir -p objects/tests/Controller
	@ mkdir -p objects/tests/Controller/Internal
	@ mkdir -p objects/tests/Services/APIs/PHOMAU
	@ mkdir -p objects/tests/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/tests/Shared/Libs/Confs
	@ mkdir -p objects/tests/Shared/Libs/Confs/internal
#	the main app directories
	@ mkdir -p objects/main/Services/APIs/PHOMAU
	@ mkdir -p objects/main/Controller
	@ mkdir -p objects/main/Controller/Internal
	@ mkdir -p objects/main/Services
	@ mkdir -p objects/main/Services/Storage
	@ mkdir -p objects/main/Shared/Misc
	@ mkdir -p objects/main/Shared/Libs/Confs/internal
	@ mkdir -p objects/main/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/main/Shared/Libs/logger/writers
	@ mkdir -p objects/main/Shared/Libs/TCPServer
	@ mkdir -p objects/main/Shared/Libs/ThreadPool
	@ mkdir -p objects/main/Shared/Libs/VarSystem
	
	

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear
	@ mkdir ./build | true
	@ cp -r ./sources/copyToBuildFolder/* ./build | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst ./sources,./objects/tests,$(subst ../sources,./objects/main,$(C_SOURCE))))
 
# Compiler and linker
CC=g++
 
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
		 -D__TESTING__ \
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
		 -D__TESTING__ \
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
 
./objects/main/%.o: ../sources/%.cpp ../sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

./objects/tests/%.o: ./sources/%.cpp ./sources/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./objects/tests/main.o: ./sources/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ $(RM) ./objects/*.o $(PROJ_NAME) *~
	@ rm -rf objects
 
.PHONY: all clean
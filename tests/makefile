 # My third makefile
 
# Name of the project
PROJ_NAME=tests
# .c files

CUSTOM_INCLUDE_PATH = -I"../source/Controller"
CUSTOM_INCLUDE_PATH += -I"../source/Controller/Internal"
CUSTOM_INCLUDE_PATH += -I"../source/Services"
CUSTOM_INCLUDE_PATH += -I"../source/Services/Apis"
CUSTOM_INCLUDE_PATH += -I"../source/Services/Apis/PHOMAU"
CUSTOM_INCLUDE_PATH += -I"../source/Services/Storage"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Misc"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/Confs"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/DependencyInjectionManager"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/logger"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/TCPServer"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/ThreadPool"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/VarSystem"

#test app files
C_SOURCE=$(wildcard ./source/*.cpp)
C_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ./source/Controller/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Libs/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ./source/Controller/Internal/*.cpp)

#main app files
C_SOURCE+=$(wildcard ../source/Controller/*.cpp)
C_SOURCE+=$(wildcard ../source/Controller/Internal/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/Apis/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/Apis/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/Storage/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/logger/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/TCPServer/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/VarSystem/*.cpp)


 
# .h files
#test app files
H_SOURCE=$(wildcard ./source/*.h)
H_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ./source/Controller/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Libs/Confs/internal/*.h)
H_SOURCE+=$(wildcard ./source/Controller/Internal/*.h)

#main app files
H_SOURCE+=$(wildcard ../source/Controller/*.h)
H_SOURCE+=$(wildcard ../source/Controller/Internal/*.h)
H_SOURCE+=$(wildcard ../source/Services/*.h)
H_SOURCE+=$(wildcard ../source/Services/Apis/*.h)
H_SOURCE+=$(wildcard ../source/Services/Apis/PHOMAU/*.h)
H_SOURCE+=$(wildcard ../source/Services/Storage/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/logger/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/TCPServer/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/ThreadPool/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/VarSystem/*.h)

objFolder:
	@ mkdir -p objects/tests/
	@ mkdir -p objects/tests/Services/APIs/PHOMAU
	@ mkdir -p objects/tests/Controller
	@ mkdir -p objects/tests/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/tests/Shared/Libs/Confs
	@ mkdir -p objects/tests/Shared/Libs/Confs/internal
	@ mkdir -p objects/tests/Controller/Internal
#	the main app directories
	@ mkdir -p objects/main/Services/APIs/PHOMAU
	@ mkdir -p objects/main/Controller/*.h)
	@ mkdir -p objects/main/Controller/Internal/*.h)
	@ mkdir -p objects/main/Services/*.h)
	@ mkdir -p objects/main/Services/Storage/*.h)
	@ mkdir -p objects/main/Shared/Misc/*.h)
	@ mkdir -p objects/main/Shared/Libs/Confs/*.h)
	@ mkdir -p objects/main/Shared/Libs/DependencyInjectionManager/*.h)
	@ mkdir -p objects/main/Shared/Libs/logger/*.h)
	@ mkdir -p objects/main/Shared/Libs/TCPServer/*.h)
	@ mkdir -p objects/main/Shared/Libs/ThreadPool/*.h)
	@ mkdir -p objects/main/Shared/Libs/VarSystem/*.h)
	
	

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear
	@ mkdir ./build | true
	@ cp -r ./source/copyToBuildFolder/* ./build | true
 
# Object files
OBJ=$(subst .cpp,.o,$(subst ./source,./objects/tests,$(subst ../source,./objects/main,$(C_SOURCE))))
 
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
		 -I"./source/" \
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
		 -I"./source/" \
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
 
./objects/main/%.o: ../source/%.cpp ../source/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

./objects/tests/%.o: ./source/%.cpp ./source/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
 
./objects/tests/main.o: ./source/main.cpp $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '
	
clean:
	@ $(RM) ./objects/*.o $(PROJ_NAME) *~
	@ rm -rf objects
 
.PHONY: all clean
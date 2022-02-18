 # My third makefile
 
# Name of the project
PROJ_NAME=tests
# .c files

CUSTOM_INCLUDE_PATH = -I"../source/Shared/Misc"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/VarSystem"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/ThreadPool"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/TcpServer"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/DependencyInjectionManager"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/Confs"
CUSTOM_INCLUDE_PATH += -I"../source/Services/Apis"
CUSTOM_INCLUDE_PATH += -I"../source/Services/Storage"
CUSTOM_INCLUDE_PATH += -I"../source/Shared/Libs/logger"
CUSTOM_INCLUDE_PATH += -I"../source/Controller/Internal"

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
C_SOURCE+=$(wildcard ../source/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/VarSystem/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/logger/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Libs/logger/writers/*.cpp)
C_SOURCE+=$(wildcard ../source/Controller/*.cpp)
C_SOURCE+=$(wildcard ../source/Controller/Internal/*.cpp)


 
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
H_SOURCE+=$(wildcard ../source/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ../source/Services/VarSystem/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/ThreadPool/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/Confs/internal/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/logger/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Libs/logger/writers/*.h)
H_SOURCE+=$(wildcard ../source/Controller/*.h)
H_SOURCE+=$(wildcard ../source/Controller/Internal/*.h)

objFolder:
	@ mkdir -p objects/tests/
	@ mkdir -p objects/tests/Services/APIs/PHOMAU
	@ mkdir -p objects/tests/Controller
	@ mkdir -p objects/tests/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/tests/Shared/Libs/Confs
	@ mkdir -p objects/tests/Shared/Libs/Confs/internal
	@ mkdir -p objects/tests/Controller/Internal
#	the main app directories
	@ mkdir -p objects/main/Controller
	@ mkdir -p objects/main/Services/APIs/PHOMAU
	@ mkdir -p objects/main/Services/VarSystem
	@ mkdir -p objects/main/Shared/Libs/Misc
	@ mkdir -p objects/main/Shared/Libs/ThreadPool
	@ mkdir -p objects/main/Shared/Libs/DependencyInjectionManager
	@ mkdir -p objects/main/Shared/Libs/Confs
	@ mkdir -p objects/main/Shared/Libs/Confs/internal
	@ mkdir -p objects/main/Shared/Misc
	@ mkdir -p objects/main/Shared/Libs/logger
	@ mkdir -p objects/main/Shared/Libs/logger/writers
	@ mkdir -p objects/main/Controller/Internal
	
	

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
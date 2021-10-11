 # My third makefile
 
# Name of the project
PROJ_NAME=tests
# .c files

CUSTOM_INCLUDE_PATH = -I"../source/Shared/Misc"

#test app files
C_SOURCE=$(wildcard ./source/*.cpp)
C_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ./source/Controller/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Confs/*.cpp)
C_SOURCE+=$(wildcard ./source/Shared/Confs/internal/*.cpp)

#main app files
C_SOURCE+=$(wildcard ../source/Controller/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/APIs/PHOMAU/*.cpp)
C_SOURCE+=$(wildcard ../source/Services/VarSystem/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Misc/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/ThreadPool/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/DependencyInjectionManager/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Confs/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Confs/internal/*.cpp)
C_SOURCE+=$(wildcard ../source/Shared/Misc/*.cpp)
 
# .h files
#test app files
H_SOURCE=$(wildcard ./source/*.h)
H_SOURCE+=$(wildcard ./source/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ./source/Controller/*.h)
H_SOURCE+=$(wildcard ./source/Shared/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Confs/*.h)
H_SOURCE+=$(wildcard ./source/Shared/Confs/internal/*.h)

#main app files
H_SOURCE+=$(wildcard ../source/Controller/*.h)
H_SOURCE+=$(wildcard ../source/Services/APIs/PHOMAU/*.h)
H_SOURCE+=$(wildcard ../source/Services/VarSystem/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Misc/*.h)
H_SOURCE+=$(wildcard ../source/Shared/ThreadPool/*.h)
H_SOURCE+=$(wildcard ../source/Shared/DependencyInjectionManager/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Confs/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Confs/internal/*.h)
H_SOURCE+=$(wildcard ../source/Shared/Misc/*.h)

objFolder:
	@ mkdir -p objects/tests/
	@ mkdir -p objects/tests/Services/APIs/PHOMAU
	@ mkdir -p objects/tests/Controller
	@ mkdir -p objects/tests/Shared/DependencyInjectionManager
	@ mkdir -p objects/tests/Shared/Confs
	@ mkdir -p objects/tests/Shared/Confs/internal
#	the main app directories
	@ mkdir -p objects/main/Controller
	@ mkdir -p objects/main/Services/APIs/PHOMAU
	@ mkdir -p objects/main/Services/VarSystem
	@ mkdir -p objects/main/Shared/Misc
	@ mkdir -p objects/main/Shared/ThreadPool
	@ mkdir -p objects/main/Shared/DependencyInjectionManager
	@ mkdir -p objects/main/Shared/Confs
	@ mkdir -p objects/main/Shared/Confs/internal
	@ mkdir -p objects/main/Shared/Misc
	
	

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
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
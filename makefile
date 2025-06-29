#this makefile can be found in https://www.embarcados.com.br/introducao-ao-makefile/

# My third makefile
 
# Name of the project
BIN_NAME=vss


CUSTOM_INCLUDE_PATH_TMP := ./sources ./.shu/packages/TCPServer/sources ./.shu/packages/ThreadPool ./.shu/packages/DependencyInjection/sources ./.shu/packages/aa ./.shu/packages/KwWebServer/sources ./.shu/packages/KwWebServer/sources/helpers ./.shu/packages/KwWebServer/sources/Workers ./.shu/packages/libs.json_maker/sources ./.shu/packages/Logger/sources/writers ./.shu/packages/Logger/sources ./.shu/packages/Logger/sources/common ./.shu/packages/DynamicVar/sources ./sources/Shared ./sources/Shared/Libs/VarSystem ./sources/Shared/Libs/MessageBus ./sources/Shared/Libs/Confs ./sources/Shared/Libs/Confs/internal ./sources/Shared/Misc ./sources/Services/Storage/VarSystemLib ./sources/Services/Storage/RamCacheDB ./sources/Services/Storage ./sources/Services/ServerDiscovery ./sources/Services/APIs ./sources/Services/APIs/http/varsexporters ./sources/Services/APIs/http ./sources/Services/APIs/VSTP ./sources/Controller/Internal ./sources/Controller 
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
CUSTOM_INCLUDE_PATH := $(addprefix -I,$(CUSTOM_INCLUDE_PATH_TMP))



#get files using dectect_include_files.sh to prevent multiple definion errors with submodules

# .c files


C_SOURCE := ./.shu/packages/TCPServer/sources/TCPServer.cpp ./.shu/packages/ThreadPool/ThreadPool.cpp ./.shu/packages/DependencyInjection/sources/dependencyInjectionManager.cpp ./.shu/packages/aa/t2.cpp ./.shu/packages/aa/t1.cpp ./.shu/packages/KwWebServer/sources/KWTinyWebServer.cpp ./.shu/packages/KwWebServer/sources/HttpData.cpp ./.shu/packages/KwWebServer/sources/helpers/httpCodes.cpp ./.shu/packages/KwWebServer/sources/helpers/KWWebServerRouter.cpp ./.shu/packages/KwWebServer/sources/Workers/CookieParser.cpp ./.shu/packages/KwWebServer/sources/Workers/HttpSession.cpp ./.shu/packages/KwWebServer/sources/StringUtilFuncs.cpp ./.shu/packages/KwWebServer/sources/SysLink.cpp ./.shu/packages/libs.json_maker/sources/JSON.cpp ./.shu/packages/Logger/sources/writers/LoggerFileWriter.cpp ./.shu/packages/Logger/sources/writers/loggercommandcallwriter.cpp ./.shu/packages/Logger/sources/writers/LoggerConsoleWriter.cpp ./.shu/packages/Logger/sources/writers/LoggerLambdaWriter.cpp ./.shu/packages/Logger/sources/logger.cpp ./.shu/packages/Logger/sources/common/loggerutils.cpp ./.shu/packages/DynamicVar/sources/DynamicVar.cpp ./sources/Shared/errors.cpp ./sources/Shared/Libs/VarSystem/FileVars.cpp ./sources/Shared/Libs/VarSystem/FVSysLink.cpp ./sources/Shared/Libs/Confs/Confs.cpp ./sources/Shared/Libs/Confs/internal/soenvironmentconfprovider.cpp ./sources/Shared/Libs/Confs/internal/commandlineargumentsconfsprovider.cpp ./sources/Shared/Libs/Confs/internal/SimpleConfFileProvider.cpp ./sources/Shared/Misc/TaggedObject.cpp ./sources/Shared/Misc/timersForDebug.cpp ./sources/Shared/Misc/utils.cpp ./sources/Shared/Misc/Observable.cpp ./sources/Services/Storage/VarSystemLib/VarSystemLibStorage.cpp ./sources/Services/Storage/RamCacheDB/ramcachedb.cpp ./sources/Services/ServerDiscovery/ServerDiscovery.cpp ./sources/Services/APIs/http/varsexporters/ivarsexporter.cpp ./sources/Services/APIs/http/varsexporters/plaintextexporter.cpp ./sources/Services/APIs/http/varsexporters/jsonexporter.cpp ./sources/Services/APIs/http/httpapi.cpp ./sources/Services/APIs/VSTP/VSTP.cpp ./sources/Controller/Internal/Controller_ClientHelper.cpp ./sources/Controller/Internal/Controller_VarHelper.cpp ./sources/Controller/Controller.cpp ./sources/main.cpp 

# .h files
H_SOURCE := ./.shu/packages/TCPServer/sources/TCPServer.h ./.shu/packages/ThreadPool/ThreadPool.h ./.shu/packages/DependencyInjection/sources/dependencyInjectionManager.h ./.shu/packages/aa/t1.h ./.shu/packages/aa/t2.h ./.shu/packages/KwWebServer/sources/HttpData.h ./.shu/packages/KwWebServer/sources/helpers/KWWebServerRouter.h ./.shu/packages/KwWebServer/sources/helpers/httpCodes.h ./.shu/packages/KwWebServer/sources/helpers/WebServerObserverHelper.h ./.shu/packages/KwWebServer/sources/SysLink.h ./.shu/packages/KwWebServer/sources/Workers/HttpSession.h ./.shu/packages/KwWebServer/sources/Workers/CookieParser.h ./.shu/packages/KwWebServer/sources/httpRamCache.h ./.shu/packages/KwWebServer/sources/IWorker.h ./.shu/packages/KwWebServer/sources/KWTinyWebServer.h ./.shu/packages/KwWebServer/sources/StringUtilFuncs.h ./.shu/packages/libs.json_maker/sources/JSON.h ./.shu/packages/Logger/sources/writers/LoggerLambdaWriter.h ./.shu/packages/Logger/sources/writers/LoggerFileWriter.h ./.shu/packages/Logger/sources/writers/loggercommandcallwriter.h ./.shu/packages/Logger/sources/writers/LoggerConsoleWriter.h ./.shu/packages/Logger/sources/logger.h ./.shu/packages/Logger/sources/ilogger.h ./.shu/packages/Logger/sources/common/loggerutils.h ./.shu/packages/DynamicVar/sources/DynamicVar.h ./sources/Shared/errors.h ./sources/Shared/Libs/VarSystem/FileVars.h ./sources/Shared/Libs/VarSystem/FVSysLink.h ./sources/Shared/Libs/MessageBus/messagebus.h ./sources/Shared/Libs/Confs/Confs.h ./sources/Shared/Libs/Confs/internal/commandlineargumentsconfsprovider.h ./sources/Shared/Libs/Confs/internal/IConfProvider.h ./sources/Shared/Libs/Confs/internal/SimpleConfFileProvider.h ./sources/Shared/Libs/Confs/internal/soenvironmentconfprovider.h ./sources/Shared/Misc/Observable.h ./sources/Shared/Misc/argparser.h ./sources/Shared/Misc/timersForDebug.h ./sources/Shared/Misc/TaggedObject.h ./sources/Shared/Misc/utils.h ./sources/Services/Storage/VarSystemLib/VarSystemLibStorage.h ./sources/Services/Storage/RamCacheDB/ramcachedb.h ./sources/Services/Storage/StorageInterface.h ./sources/Services/ServerDiscovery/ServerDiscovery.h ./sources/Services/APIs/ApiInterface.h ./sources/Services/APIs/ApiMediatorInterface.h ./sources/Services/APIs/http/varsexporters/plaintextexporter.h ./sources/Services/APIs/http/varsexporters/ivarsexporter.h ./sources/Services/APIs/http/varsexporters/jsonexporter.h ./sources/Services/APIs/http/httpapi.h ./sources/Services/APIs/VSTP/VSTP.h ./sources/Controller/Internal/Controller_ClientHelper.h ./sources/Controller/Internal/Controller_VarHelper.h ./sources/Controller/Controller.h 
 	

prebuild:
# 	prepares the folder built/gui. This folder contains files copied from GUI/resources. These files contains the HTML5 User interface.
	@ clear | true
	@ mkdir ./build | true
	@ cp -r ./sources/assets/* ./build >/dev/null 2>&1 | true
 
# Object files
# Object files
temp:=$(subst ./.shu,./build/objects/shu,$(C_SOURCE))
temp:=$(subst ./sources,./build/objects/main,$(temp))
OBJ=$(subst .cpp,.o,$(temp))


# Compiler and linker
CC=clang++

CC_FLAGS=$(CUSTOM_INCLUDE_PATH) -c -pthread -std=c++20 -lssl -lcrypto
		

LK_FLAGS=$(CUSTOM_INCLUDE_PATH) -pthread -std=c++20 -lssl -lcrypto
#
# Compilation and linking
#
 

 
# Command used at clean target
RM = rm -rf
 
debug: CC=g++
debug: CC_FLAGS+=-g
debug: CC_FLAGS+=-ggdb
debug: LK_FLAGS+=-g
debug: LK_FLAGS+=-ggdb
debug: prebuild $(BIN_NAME)
	
all: CC_FLAGS+=-O3
all: LK_FLAGS+=-O3
all: prebuild $(BIN_NAME)
 
$(BIN_NAME): $(OBJ)
	echo 'Building binary using GCC linker: $@'
	$(CC) $^ $(LK_FLAGS) -o build/$@
	@echo 'Finished building binary: $@'
	@echo ' '

./build/objects/main/%.o: ./sources/%.cpp ./sources/%.h
	echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@
	@echo ' '

./build/objects/shu/%.o: ./.shu/%.cpp ./.shu/%.h
	echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@
	@echo ' '

./build/objects/main/main.o: ./sources/main.cpp $(H_SOURCE)
	echo 'Building target using GCC compiler: $<'
	mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c $< -o $@
	echo ' '

clean:
	@ $(RM) ./build/objects/*.o $(BIN_NAME)
	@ rm -rf ./build/objects
 
.PHONY: all clean

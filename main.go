//important: this project is a rewrite of the project inside cpp_version folder

package main

import (
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"rtonello/vss/sources/controller"
	"rtonello/vss/sources/misc"
	"rtonello/vss/sources/services/apis/vstp"
	"rtonello/vss/sources/services/storage"
	"strings"
	"sync"
	"syscall"

	"rtonello/vss/sources/misc/logger"
	logwriters "rtonello/vss/sources/misc/logger/writers"

	"rtonello/vss/sources/misc/confs"
	"rtonello/vss/sources/misc/confs/sources"
	httpapi "rtonello/vss/sources/services/apis/httpapi"
)

const INFO_VERSION = "2.0.0+Capella"

func main() {

	//#region configs and logger {
	fmt.Print("Loading configurations...")
	configs := initConfigurations()
	fmt.Println("done.")

	fmt.Print("Initializing logger...")
	logManager := initLogger(configs)
	fmt.Println("done.")
	//#endregion }

	//setupStdoutAndStderrInterception(logManager)

	//#region Initial banner and runtime informations {
	printBanner(logManager, configs)
	//#endregion }

	//#region remain services {
	mainLog := logManager.GetNamedLogger("Main")

	mainLog.KeepNextLineOpened()
	mainLog.Info("Starting storage ...")
	theStorage := initStorage(configs, logManager)
	mainLog.Info("...Storage started.")

	mainLog.Info("Starting controller ...")
	theController := controller.NewController(logManager, configs, theStorage, INFO_VERSION)
	mainLog.Info("...Controller started.")

	mainLog.Info("Starting VSTP API ...")
	tmpDv := configs.Config("vstpApiPort").Value()
	vstpPort := (&tmpDv).GetInt()
	vstpApi, err := vstp.NewVSTP(vstpPort, theController, logManager)
	if err != nil {
		mainLog.Error("Main", "Failed to start VSTP API: "+err.Error())
		// exit early on failure
		return
	}
	_ = vstpApi
	mainLog.Info("...VSTP API started.")

	mainLog.Info("Starting HTTP API ...")
	httpApiPort := configs.Config("httpApiPort").Value()
	httpApi, err := httpapi.New(httpApiPort.GetInt(), theController)
	if err != nil {
		mainLog.Error("Main", "Failed to start HTTP API: "+err.Error())
		// exit early on failure
		return
	} else {
		mainLog.Info("...HTTP API started.")
	}
	_ = httpApi
	//#endregion }

	//#endregion }

	//handle system signals to allow graceful shutdown
	sigs := make(chan os.Signal, 1)
	setupSignalHandler(sigs)

	sig := <-sigs
	mainLog.Info("Received signal: " + sig.String() + ", shutting down...")

	//finalize services in reverse order

	//kepp app running
	mx := sync.Mutex{}
	mx.Lock()
	mx.Lock()

}

func setupSignalHandler(sigs chan os.Signal) {
	// Register for common termination signals. The main goroutine is waiting
	// on the provided channel, so just forward OS signals into it.
	signal.Notify(sigs, os.Interrupt, syscall.SIGTERM)

	// Optionally ignore SIGPIPE to avoid the process being killed when writing
	// to closed sockets on some platforms.
	signal.Ignore(syscall.SIGPIPE)
}

func printBanner(logger logger.ILogger, configs confs.IConfs) {

	text := "" + "The VSS has been started\n"
	text += "" + "+-- Version: " + INFO_VERSION + "\n"
	text += "" + "+-- Portable mode: "
	if runningInPortableMode() {
		text += "Yes" + "\n"
	} else {
		text += "No" + "\n"
	}

	text += "" + "|   +-- conf file: " + findConfigurationFile() + "\n"

	text += "" + "|   +-- log file: " + determineLogFile() + "\n"

	tmp := configs.Config("DbDirectory").Value()
	text += "" + "|   +-- database folder: " + tmp.GetString() + "\n"
	text += "" + "+-- Services" + "\n"

	tmp = configs.Config("vstpApiPort").Value()
	text += "" + "|   +-- VSTP port: TCP/" + tmp.GetString() + "\n"

	tmp = configs.Config("httpApiPort").Value()
	tmp2 := configs.Config("httpApiHttpsPort").Value()
	text += "" + "|   +-- HTTP port: TCP/" + tmp.GetString() + "(http) + TCP/" + tmp2.GetString() + "(https)" + "\n"

	tmp = configs.Config("serverDiscoveryPort").Value()
	text += "" + "|   +-- Server discovery port: UDP/" + tmp.GetString() + "\n"
	text += "" + "+-- All configurations" + "\n"

	for key, item := range configs.AllConfigs() {
		tmp = item.NotMappedValue()
		text += "" + "    +-- " + key + ": " + tmp.GetString() + "\n"
	}

	logger.Info("", text)
}

func initConfigurations() confs.IConfs {
	theConfs := confs.NewConfs(
		//add configuration sources here
		[]confs.IConfsSource{
			//command line arguments
			sources.NewCommandLineSource(os.Args[1:]),

			//environment variables
			sources.NewEnvSource(),

			//configuration file
			sources.NewFileSource(findConfigurationFile()),
		},
	)

	//conf->createPlaceHolders()
	//         .add("%PROJECT_DIR%",  getApplicationDirectory(false))
	//         .add("%APP_DIR%",  getApplicationDirectory(false))
	//         .add("%FILE_SYSTEM_CONTEXT%", isRunningInPortableMode() ?  getApplicationDirectory(false) : "")
	//         .add("%SUGGESTED_DATA_DIRECTORY%", isRunningInPortableMode() ?  getApplicationDirectory(false) + "/data" : "/var/vss/data")
	//     ;

	theConfs.AddPlaceHolders(map[string]string{
		"%PROJECT_DIR%":              getApplicationDirectory(false),
		"%APP_DIR%":                  getApplicationDirectory(false),
		"%SUGGESTED_DATA_DIRECTORY%": suggestDataDirectory(),
	})

	theConfs.CreateConfig("maxLogFileSize",
		confs.WithPossibleNames([]string{"max-log-file-size", "--max-log-file-size", "VSS_MAX_LOG_FILE_SIZE"}),
		confs.WithDefaultValue(misc.NewDynamicVar("52428800")), //50 MiB
	)

	theConfs.CreateConfig("fileLogLevel",
		confs.WithPossibleNames([]string{"file-log-level", "--file-log-level", "VSS_FILE_LOG_LEVEL"}),
		confs.WithDefaultValue(misc.NewDynamicVar("info2")),
		confs.WithValueMap(map[misc.DynamicVar]misc.DynamicVar{
			misc.NewDynamicVar("trace"):    misc.NewDynamicVar(logger.LEVEL_TRACE),
			misc.NewDynamicVar("debug"):    misc.NewDynamicVar(logger.LEVEL_DEBUG2),
			misc.NewDynamicVar("info"):     misc.NewDynamicVar(logger.LEVEL_INFO),
			misc.NewDynamicVar("info2"):    misc.NewDynamicVar(logger.LEVEL_INFO2),
			misc.NewDynamicVar("warning"):  misc.NewDynamicVar(logger.LEVEL_WARNING),
			misc.NewDynamicVar("error"):    misc.NewDynamicVar(logger.LEVEL_ERROR),
			misc.NewDynamicVar("critical"): misc.NewDynamicVar(logger.LEVEL_CRITICAL),
		}),
	)

	theConfs.CreateConfig("stdoutLogLevel",
		confs.WithPossibleNames([]string{"stdout-log-level", "--stdout-log-level", "VSS_STDOUT_LOG_LEVEL"}),
		confs.WithDefaultValue(misc.NewDynamicVar("info")),
		confs.WithValueMap(map[misc.DynamicVar]misc.DynamicVar{
			misc.NewDynamicVar("trace"):    misc.NewDynamicVar(logger.LEVEL_TRACE),
			misc.NewDynamicVar("debug"):    misc.NewDynamicVar(logger.LEVEL_DEBUG2),
			misc.NewDynamicVar("info"):     misc.NewDynamicVar(logger.LEVEL_INFO),
			misc.NewDynamicVar("info2"):    misc.NewDynamicVar(logger.LEVEL_INFO2),
			misc.NewDynamicVar("warning"):  misc.NewDynamicVar(logger.LEVEL_WARNING),
			misc.NewDynamicVar("error"):    misc.NewDynamicVar(logger.LEVEL_ERROR),
			misc.NewDynamicVar("critical"): misc.NewDynamicVar(logger.LEVEL_CRITICAL),
		}),
	)

	theConfs.CreateConfig("serverDiscoveryPort",
		confs.WithPossibleNames([]string{"server-discovery-port", "--server-discovery-port", "VSS_SERVER_DISCOVERY_PORT"}),
		confs.WithDefaultValue(misc.NewDynamicVar("5022")),
	)

	theConfs.CreateConfig("maxTimeWaitingClient_seconds",
		confs.WithPossibleNames([]string{"max-time-waiting-client-seconds", "--max-time-waiting-for-clients", "VSS_MAX_TIME_WAITING_CLIENTS"}),
		confs.WithDefaultValue(misc.NewDynamicVar("43200")), //12 hours
	)

	theConfs.CreateConfig("DbDirectory",
		confs.WithPossibleNames([]string{"db-directory", "--db-directory", "VSS_DB_DIRECTORY"}),
		confs.WithDefaultValue(misc.NewDynamicVar("%SUGGESTED_DATA_DIRECTORY%/database")),
	)

	theConfs.CreateConfig("httpDataDir",
		confs.WithPossibleNames([]string{"http-data-directory", "--http-data-directory", "--http-data-dir", "VSS_HTTP_DATA_DIRECTORY"}),
		confs.WithDefaultValue(misc.NewDynamicVar("%SUGGESTED_DATA_DIRECTORY%/http_data")),
	)

	theConfs.CreateConfig("httpApiPort",
		confs.WithPossibleNames([]string{"http-api-port", "--http-api-port", "VSS_HTTP_API_PORT"}),
		confs.WithDefaultValue(misc.NewDynamicVar("5024")),
	)

	theConfs.CreateConfig("httpApiHttpsPort",
		confs.WithPossibleNames([]string{"http-api-https-port", "--http-api-https-port", "VSS_HTTP_API_HTTPS_PORT"}),
		confs.WithDefaultValue(misc.NewDynamicVar("5025")),
	)

	theConfs.CreateConfig("httpApiCertFile",
		confs.WithPossibleNames([]string{"http-api-cert-file", "--http-api-cert-file", "VSS_HTTP_API_CERT_FILE"}),
		confs.WithDefaultValue(misc.NewDynamicVar("%APP_DIR%/ssl/cert/vssCert.pem")),
	)

	theConfs.CreateConfig("httpApiKeyFile",
		confs.WithPossibleNames([]string{"http-api-key-file", "--http-api-key-file", "VSS_HTTP_API_KEY_FILE"}),
		confs.WithDefaultValue(misc.NewDynamicVar("%APP_DIR%/ssl/cert/vssKey.pem")),
	)

	theConfs.CreateConfig("httpApiReturnsFullPaths",
		confs.WithPossibleNames([]string{"http-api-returns-full-paths", "--http-api-returns-full-paths", "VSS_HTTP_API_RETURN_FULL_PATHS"}),
		confs.WithDefaultValue(misc.NewDynamicVar("false")),
	)

	theConfs.CreateConfig("vstpApiPort",
		confs.WithPossibleNames([]string{"vstp-api-port", "--vstp-api-port", "VSS_VSTP_API_PORT"}),
		confs.WithDefaultValue(misc.NewDynamicVar("5032")),
	)

	theConfs.CreateConfig("RamCacheDbDumpIntervalMs",
		confs.WithPossibleNames([]string{"ram-cache-db-dump-interval-ms", "--ram-cache-db-dump-interval-ms", "VSS_RAMCACHEDB_DUMP_INTERVAL_MS"}),
		confs.WithDefaultValue(misc.NewDynamicVar("60000")), //60 seconds
	)

	return theConfs
}

func initLogger(configs confs.IConfs) logger.ILogger {
	fileLogLevel := configs.Config("fileLogLevel").Value()
	consoleLogLevel := configs.Config("stdoutLogLevel").Value()

	maxLogSize := configs.Config("maxLogFileSize").Value()
	fileWriter, err := logwriters.NewFileWriter(
		determineLogFile(),
		fileLogLevel.GetInt(),
		true,
		maxLogSize.GetInt64(),
	)
	if err != nil {
		panic("Failed to initialize file writer: " + err.Error())
	}

	logManager := logger.NewLogger([]logger.ILogWriter{
		logwriters.NewConsoleWriter(consoleLogLevel.GetInt(), true, true, true, true),
		fileWriter,
	}, false, 0)

	return logManager
}

func setupStdoutAndStderrInterception(logManager logger.ILogger) {
	//intercept stdout and stderr to log them into the logger
	stdoutReader, stdoutWriter, err := os.Pipe()
	if err != nil {
		logManager.Error("Main", "Failed to create pipe for stdout interception: "+err.Error())
		return
	}
	stderrReader, stderrWriter, err := os.Pipe()
	if err != nil {
		logManager.Error("Main", "Failed to create pipe for stderr interception: "+err.Error())
		return
	}

	//redirect stdout and stderr
	os.Stdout = stdoutWriter
	os.Stderr = stderrWriter

	//start goroutine to read from stdout pipe
	go func() {
		buf := make([]byte, 1024)
		for {
			n, err := stdoutReader.Read(buf)
			if err != nil {
				break
			}
			if n > 0 {
				logManager.Info("STDOUT", string(buf[:n]))
			}
		}
	}()

	//start goroutine to read from stderr pipe
	go func() {
		buf := make([]byte, 1024)
		for {
			n, err := stderrReader.Read(buf)
			if err != nil {
				break
			}
			if n > 0 {
				logManager.Error("STDERR", string(buf[:n]))
			}
		}
	}()

	fmt.Println("Stdout and Stderr interception setup completed.")
}

func initStorage(configs confs.IConfs, logger logger.ILogger) storage.IStorage {

	dbDirectoryConfig := configs.Config("DbDirectory").Value()
	dbDumpIntervalConfig := configs.Config("RamCacheDbDumpIntervalMs").Value()
	_ = os.MkdirAll(dbDirectoryConfig.GetString(), 0755)
	theStorage := storage.NewRamCacheDB(logger, dbDirectoryConfig.GetString(), dbDumpIntervalConfig.GetInt())

	return theStorage
}

func getApplicationDirectory(shortenIfPossible bool) string {
	executablePath, _ := os.Executable()
	if shortenIfPossible {
		//get current working directory
		cwd, err := os.Getwd()
		if err == nil {
			relPath, err := filepath.Rel(cwd, executablePath)
			if err == nil && !strings.HasPrefix(relPath, "..") {
				ret := filepath.Dir(relPath)
				return ret
			}
		}
	}

	return filepath.Dir(executablePath)
}

func runningInPortableMode() bool {
	executablePath, _ := os.Executable()

	if strings.HasPrefix(executablePath, "/usr") || strings.HasPrefix(executablePath, "/bin") {
		return false
	}

	return true
}

func findConfigurationFile() string {
	if runningInPortableMode() {
		return filepath.Join(getApplicationDirectory(false), "config", "vss.conf")
	} else {
		return "/etc/vss/vss.conf"
	}
}

func determineLogFile() string {
	if runningInPortableMode() {
		return filepath.Join(getApplicationDirectory(false), "vss.log")
	} else {
		return "/var/log/vss.log"
	}
}

func suggestDataDirectory() string {
	if runningInPortableMode() {
		return getApplicationDirectory(false) + "/data"
	} else {
		return "/var/vss/data"
	}
}

package logger

//https://github.com/lxwagn/using-go-with-c-libraries

//https://stackoverflow.com/questions/2734719/how-to-compile-a-static-library-in-linux
//https://stackoverflow.com/questions/10454263/command-to-compile-c-files-with-a-files
//https://stackoverflow.com/questions/25084855/how-does-gcc-shared-option-affect-the-output

/* #cgo LDFLAGS: -ldl
#cgo CFLAGS:-I../../library/sources
#cgo LDFLAGS: -ldl
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

typedef ulong (*logger_new_logger_t)();
typedef const char* (*logger_setup_addOutputFile_t)(ulong instance, char* filename, int logLevel, int maxFileSize);
typedef const char* (*logger_setup_addConsoleOutput_t)(ulong instance, int logLevel, int useColors, int displayLinePrefix, int displayDateTime, int printMilisseconds);
typedef const char* (*logger_setup_done_t)(ulong instance);
typedef const char* (*logger_free_logger_t)(ulong instance);
typedef const char* (*logger_log_t)(ulong instance, int level, char* name, char* msgs);
typedef const char* (*logger_trace_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_debug2_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_debug_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_info2_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_info_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_warning_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_error_t)(ulong instance, char* name, char* msgs);
typedef const char* (*logger_critical_t)(ulong instance, char* name, char* msgs);

logger_setup_addOutputFile_t logger_setup_addOutputFile_p;
logger_new_logger_t logger_new_logger_p;
logger_setup_addOutputFile_t logger_setup_addOutputFile_p;
logger_setup_addConsoleOutput_t logger_setup_addConsoleOutput_p;
logger_setup_done_t logger_setup_done_p;
logger_free_logger_t logger_free_logger_p;
logger_log_t logger_log_p;
logger_trace_t logger_trace_p;
logger_debug2_t logger_debug2_p;
logger_debug_t logger_debug_p;
logger_info2_t logger_info2_p;
logger_info_t logger_info_p;
logger_warning_t logger_warning_p;
logger_error_t logger_error_p;
logger_critical_t logger_critical_p;



int libStarted = 0;
int init(char* soPath){
	if (libStarted){
		return 0;
	}

	libStarted = 1;

	//check if soPath is empty
	if(soPath == NULL || soPath[0] == '\0'){
		soPath = "logger.so";
	}

	void* libp = dlopen(soPath, RTLD_LAZY);
	if (libp == 0){
		printf("Failed to load library: %s\n", dlerror());
		return 1;
	}

	logger_new_logger_p = (logger_new_logger_t)dlsym(libp, "logger_new_logger");
	logger_setup_addOutputFile_p = (logger_setup_addOutputFile_t)dlsym(libp, "logger_setup_addOutputFile");
	logger_setup_addConsoleOutput_p = (logger_setup_addConsoleOutput_t)dlsym(libp, "logger_setup_addConsoleOutput");
	logger_setup_done_p = (logger_setup_done_t)dlsym(libp, "logger_setup_done");
	logger_free_logger_p = (logger_free_logger_t)dlsym(libp, "logger_free_logger");
	logger_log_p = (logger_log_t)dlsym(libp, "logger_log");
	logger_trace_p = (logger_trace_t)dlsym(libp, "logger_trace");
	logger_debug2_p = (logger_debug2_t)dlsym(libp, "logger_debug2");
	logger_debug_p = (logger_debug_t)dlsym(libp, "logger_debug");
	logger_info2_p = (logger_info2_t)dlsym(libp, "logger_info2");
	logger_info_p = (logger_info_t)dlsym(libp, "logger_info");
	logger_warning_p = (logger_warning_t)dlsym(libp, "logger_warning");
	logger_error_p = (logger_error_t)dlsym(libp, "logger_error");
	logger_critical_p = (logger_critical_t)dlsym(libp, "logger_critical");

	return 0;
}

ulong logger_new_logger()
{
	return logger_new_logger_p();
}

const char* logger_setup_addOutputFile(ulong instance, char* filename, int logLevel, int maxFileSize)
{
	return logger_setup_addOutputFile_p(instance, filename, logLevel, maxFileSize);
}

const char* logger_setup_addConsoleOutput(ulong instance, int logLevel, int useColors, int displayLinePrefix, int displayDateTime, int printMilisseconds)
{
	return logger_setup_addConsoleOutput_p(instance, logLevel, useColors, displayLinePrefix, displayDateTime, printMilisseconds);
}

const char* logger_setup_done(ulong instance)
{
	return logger_setup_done_p(instance);
}

const char* logger_free_logger(ulong instance)
{
	return logger_free_logger_p(instance);
}

const char* logger_log(ulong instance, int level, char* name, char* msgs)
{
	return logger_log_p(instance, level, name, msgs);
}

const char* logger_trace(ulong instance, char* name, char* msgs)
{
	return logger_trace_p(instance, name, msgs);
}

const char* logger_debug2(ulong instance, char* name, char* msgs)
{
	return logger_debug2_p(instance, name, msgs);
}

const char* logger_debug(ulong instance, char* name, char* msgs)
{
	return logger_debug_p(instance, name, msgs);
}

const char* logger_info2(ulong instance, char* name, char* msgs)
{
	return logger_info2_p(instance, name, msgs);
}

const char* logger_info(ulong instance, char* name, char* msgs)
{
	return logger_info_p(instance, name, msgs);
}

const char* logger_warning(ulong instance, char* name, char* msgs)
{
	return logger_warning_p(instance, name, msgs);
}

const char* logger_error(ulong instance, char* name, char* msgs)
{
	return logger_error_p(instance, name, msgs);
}

const char* logger_critical(ulong instance, char* name, char* msgs)
{
	return logger_critical_p(instance, name, msgs);
}
*/
import "C"
import (
	"encoding/json"
	"fmt"
	"reflect"
)

type INamedLogger interface {

	//Reports tracing messages. Trace is the most detailed log level. Anything and everything can be logged at this level.
	Trace(msgs ...any) string

	//Reports very detailed debug information, typically of interest only when diagnosing problems.
	//Messages to be used to understand the flow through the system and values of variables.
	//more verbose than Debug. ("pass there", "pass 1", "pass 2", ...)
	Debug2(msgs ...any) string

	//Reports detailed debug information, typically of interest only when diagnosing problems.
	//Messages to be used to understand the flow through the system and values of variables.
	Debug(msgs ...any) string

	//Report detailed information about what system is doing, typically of interest only when diagnosing problems.
	Info2(msgs ...any) string

	//Reports informations about things system is doing. Monitores the overall system health.
	Info(msgs ...any) string

	//Reports that something unexpected happened, or indicative of some problem in the near future (e.g. 'disk space low').
	//Also, errors that are handled but should be reported, errors that system can recover from.
	Warning(msgs ...any) string

	//Reports an error event that might still allow the application to continue running, but abort some operation. Higher
	//severity than Warning, but system can still continue.
	Error(msgs ...any) string

	//Reports a severe error event that will presumably lead the application to abort. Highest level of severity.
	//It is expected the application be terminated immediately after this log.
	Critical(msgs ...any) string
}

type ILogger interface {
	//Logs a message with a specific level and name.
	Log(level int, name string, msgs ...any) string

	//Reports tracing messages. Trace is the most detailed log level. Anything and everything can be logged at this level.
	Trace(name string, msgs ...any) string

	//Reports very detailed debug information, typically of interest only when diagnosing problems.
	//Messages to be used to understand the flow through the system and values of variables.
	//more verbose than Debug. ("pass there", "pass 1", "pass 2
	Debug2(name string, msgs ...any) string

	//Reports detailed debug information, typically of interest only when diagnosing problems.
	//Messages to be used to understand the flow through the system and values of variables.
	Debug(name string, msgs ...any) string

	//Report detailed information about what system is doing, typically of interest only when diagnosing problems.
	Info2(name string, msgs ...any) string

	//Reports informations about things system is doing. Monitores the overall system health.
	Info(name string, msgs ...any) string

	//Reports that something unexpected happened, or indicative of some problem in the near future (e.g. 'disk space low').
	//Also, errors that are handled but should be reported, errors that system can recover from.
	Warning(name string, msgs ...any) string

	//Reports an error event that might still allow the application to continue running, but abort some operation. Higher
	//severity than Warning, but system can still continue.
	Error(name string, msgs ...any) string

	//Reports a severe error event that will presumably lead the application to abort. Highest level of severity.
	//It is expected the application be terminated immediately after this log.
	Critical(name string, msgs ...any) string

	Finalize()

	GetNamedLogger(name string) INamedLogger
}

type Logger struct {
	instance              C.ulong
	enableJsonOutputIdent bool
	dynamicLibPath        string
}

const (
	LEVEL_ALL      = 0
	LEVEL_TRACE    = 9
	LEVEL_DEBUG2   = 10
	LEVEL_DEBUG    = 20
	LEVEL_INFO2    = 30
	LEVEL_INFO     = 40
	LEVEL_WARNING  = 50
	LEVEL_ERROR    = 60
	LEVEL_CRITICAL = 70
)

type NamedLogger struct {
	name   string
	logger *Logger
}

type ILogWriter interface {
	doRegistration(onstance C.ulong)
}

type ILogWriterBase struct {
	Loglevel int
}

type LoggerConsoleWriter struct {
	ILogWriterBase

	UseColors         bool
	DisplayLinePrefix bool
	DisplayDateTime   bool
	PrintMilisseconds bool
}

type LoggerFileWriter struct {
	ILogWriterBase

	FileName    string
	MaxFileSize int
}

var libStarted bool = false

// create a new logger. A logger is the main object that you will use to log messages. You need to specify at
// least one writer (otherwise, the logger will not write anything).
func NewLogger(writers []ILogWriter, options ...func(*Logger)) ILogger {
	ret := Logger{0, true, "logger.so"}
	for _, option := range options {
		option(&ret)
	}

	if !libStarted {
		libStarted = true
		initResult := C.init(C.CString(ret.dynamicLibPath))
		if initResult != 0 {
			panic("Failed to initialize logger library")
		}
	}

	ret.instance = C.logger_new_logger()

	for _, writer := range writers {
		writer.doRegistration(ret.instance)
	}

	C.logger_setup_done(ret.instance)

	return &ret

}

// when you pass structs to log functions, it will be converted to json. If you want to disable this, use this function.
// By default, the json output is idented.
func OpDisableJsonOutputIdent() func(logger *Logger) {
	return func(logger *Logger) {
		logger.enableJsonOutputIdent = false
	}
}

// use this function to specify a custom path to the dynamic library. By default, the logger will try to load the library
// from the current directory.
func OpCustomDynamicLibPath(path string) func(logger *Logger) {
	return func(logger *Logger) {
		logger.dynamicLibPath = path
	}
}

// create a named logger. A named logger is a wrapper to a logger that prevent you to pass the name every time you want to log something.
func NewNamedLogger(name string, logger *Logger) NamedLogger {
	return NamedLogger{name, logger}
}

// create a new console writer. A console writer is a writer that will write the logs to the stdout.
// You can enable of disable the use of colors for diferent log levels, set a prefix for each line,
// display the date and time and enable or disable the print of milisseconds. You can use the logLevel
// to specify the minimum log level that will be written.
func NewConsoleWriter(logLevel int, useColors bool, displayLinePrefix bool, displayDateTime bool, printMilisseconds bool) LoggerConsoleWriter {
	return LoggerConsoleWriter{ILogWriterBase{logLevel}, useColors, displayLinePrefix, displayDateTime, printMilisseconds}
}

// Create a new file writer. A file writer is a writer that will write the logs to a file. You can use
// the logLevel to specify the minimum log level that will be written.
func NewFileWriter(logLevel int, fileName string, maxFileSize int) LoggerFileWriter {
	return LoggerFileWriter{ILogWriterBase{logLevel}, fileName, maxFileSize}
}

//#region ILoggerWriters

func (writer LoggerConsoleWriter) doRegistration(instance C.ulong) {
	useColors := 0
	if writer.UseColors {
		useColors = 1
	}

	displayLinePrefix := 0
	if writer.DisplayLinePrefix {
		displayLinePrefix = 1
	}

	printMilisseconds := 0
	if writer.PrintMilisseconds {
		printMilisseconds = 1
	}

	displayDateTime := 0
	if writer.DisplayDateTime {
		displayDateTime = 1
	}

	C.logger_setup_addConsoleOutput(instance, C.int(writer.Loglevel), C.int(useColors), C.int(displayLinePrefix), C.int(displayDateTime), C.int(printMilisseconds))
}

func (writer LoggerFileWriter) doRegistration(instance C.ulong) {
	C.logger_setup_addOutputFile(instance, C.CString(writer.FileName), C.int(writer.ILogWriterBase.Loglevel), C.int(writer.MaxFileSize))
}

//#endregion ILoggerWriters

// #region Logger {
func (logger *Logger) Finalize() {
	C.logger_free_logger(logger.instance)
}

func (logger *Logger) GetNamedLogger(name string) INamedLogger {
	return &NamedLogger{name, logger}
}

func (logger *Logger) Log(level int, name string, msgs ...any) string {

	return C.GoString(C.logger_log(logger.instance, C.int(level), C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Trace(name string, msgs ...any) string {
	return C.GoString(C.logger_trace(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Debug2(name string, msgs ...any) string {
	return C.GoString(C.logger_debug2(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Debug(name string, msgs ...any) string {
	return C.GoString(C.logger_debug(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Info2(name string, msgs ...any) string {
	return C.GoString(C.logger_info2(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Info(name string, msgs ...any) string {
	return C.GoString(C.logger_info(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Warning(name string, msgs ...any) string {
	return C.GoString(C.logger_warning(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Error(name string, msgs ...any) string {
	return C.GoString(C.logger_error(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func (logger *Logger) Critical(name string, msgs ...any) string {
	return C.GoString(C.logger_critical(logger.instance, C.CString(name), C.CString(variadicToString(logger.enableJsonOutputIdent, msgs...))))
}

func variadicToString(enableJsonOutputIdent bool, args ...any) string {
	buffer := ""
	for _, msg := range args {
		valueStr := anyToString(msg, enableJsonOutputIdent)

		if buffer != "" {
			buffer += " "
		}

		buffer += valueStr
	}

	return buffer
}

func anyToString(value interface{}, enableJsonOutputIdent bool) string {

	msgsType := reflect.TypeOf(value)

	valueStr := ""

	switch msgsType.Kind() {
	case reflect.String:
		valueStr = value.(string)
	//case reflect.Struct:
	//	valueStr = fmt.Sprintf("%v", value)
	case reflect.Func:
		//call the function and get the return value
		fv := reflect.ValueOf(value)
		if fv.Kind() == reflect.Func {
			result := fv.Call(nil)
			return anyToString(result[0].Interface(), enableJsonOutputIdent)
		}
	default:
		var valueByte []byte
		var err error
		if enableJsonOutputIdent {
			valueByte, err = json.MarshalIndent(&value, "", "  ")
		} else {
			valueByte, err = json.Marshal(&value)
		}

		if err == nil {
			valueStr = string(valueByte)
		} else {
			valueStr = fmt.Sprintf("%v", value)
		}
	}

	//remove possible "[[" and "]]" from the valueString (begin and end)
	//if strings.HasPrefix(valueStr, "[[") && strings.HasSuffix(valueStr, "]]") {
	//	valueStr = valueStr[2 : len(valueStr)-2]
	//}

	return valueStr
}

//#endregion Logger

// #region named logger {
func (nlogger *NamedLogger) Log(level int, msgs ...any) string {
	return nlogger.logger.Log(level, nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Trace(msgs ...any) string {
	return nlogger.logger.Trace(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Debug2(msgs ...any) string {
	return nlogger.logger.Debug2(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Debug(msgs ...any) string {
	return nlogger.logger.Debug(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Info2(msgs ...any) string {
	return nlogger.logger.Info2(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Info(msgs ...any) string {
	return nlogger.logger.Info(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Warning(msgs ...any) string {
	return nlogger.logger.Warning(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Error(msgs ...any) string {
	return nlogger.logger.Error(nlogger.name, msgs...)
}

func (nlogger *NamedLogger) Critical(msgs ...any) string {
	return nlogger.logger.Critical(nlogger.name, msgs...)
}

//#endregion named logger }

package logger

import (
	"time"
)

const DEFAULT_LOG_NAME = ""

// ILogWriter is implemented by log destinations
type ILogWriter interface {
	Write(msg string, level int, name string, dateTime time.Time, timezoneoffset int)
}

// ILogger interface mirrors the C++ ILogger behavior in a Go style
type ILogger interface {
	Log(level int, name string, msgs ...any)
	Trace(name string, msgs ...any)
	Debug2(name string, msgs ...any)
	Debug(name string, msgs ...any)
	Info2(name string, msgs ...any)
	Info(name string, msgs ...any)
	Warning(name string, msgs ...any)
	Error(name string, msgs ...any)
	Critical(name string, msgs ...any)

	// convenience methods that default to DEFAULT_LOG_NAME
	LogDefault(level int, msg string)
	TraceDefault(msg string)

	GetNamedLogger(name string) INamedLogger

	//a helper to GetNamedLogger
	GetNLog(name string) INamedLogger

	KeepNextLineOpened()
}

type INamedLogger interface {
	SetName(name string)
	SetMainLogger(l ILogger)

	Log(level int, msgs ...any)
	Trace(msgs ...any)
	Debug2(msgs ...any)
	Debug(msgs ...any)
	Info2(msgs ...any)
	Info(msgs ...any)
	Warning(msgs ...any)
	Error(msgs ...any)
	Critical(msgs ...any)
	KeepNextLineOpened()
}

// NLogger is a named logger that forwards to a main ILogger
type NLogger struct {
	name       string
	mainLogger ILogger
}

func NewNLogger(name string, main ILogger) *NLogger {
	return &NLogger{name: name, mainLogger: main}
}

func (n *NLogger) SetName(name string)     { n.name = name }
func (n *NLogger) SetMainLogger(l ILogger) { n.mainLogger = l }

func (n *NLogger) Log(level int, msgs ...any) { n.mainLogger.Log(level, n.name, msgs...) }
func (n *NLogger) Trace(msgs ...any)          { n.mainLogger.Trace(n.name, msgs...) }
func (n *NLogger) Debug2(msgs ...any)         { n.mainLogger.Debug2(n.name, msgs...) }
func (n *NLogger) Debug(msgs ...any)          { n.mainLogger.Debug(n.name, msgs...) }
func (n *NLogger) Info2(msgs ...any)          { n.mainLogger.Info2(n.name, msgs...) }
func (n *NLogger) Info(msgs ...any)           { n.mainLogger.Info(n.name, msgs...) }
func (n *NLogger) Warning(msgs ...any)        { n.mainLogger.Warning(n.name, msgs...) }
func (n *NLogger) Error(msgs ...any)          { n.mainLogger.Error(n.name, msgs...) }
func (n *NLogger) Critical(msgs ...any)       { n.mainLogger.Critical(n.name, msgs...) }
func (n *NLogger) KeepNextLineOpened()        { n.mainLogger.KeepNextLineOpened() }

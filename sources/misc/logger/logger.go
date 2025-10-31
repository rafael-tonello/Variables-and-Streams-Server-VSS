package logger

import (
	"rtonello/vss/sources/misc/logger/loggerutils"
	"strings"
	"time"
)

const (
	LEVEL_TRACE    = 9
	LEVEL_DEBUG2   = 10
	LEVEL_DEBUG    = 20
	LEVEL_INFO2    = 30
	LEVEL_INFO     = 40
	LEVEL_WARNING  = 50
	LEVEL_ERROR    = 60
	LEVEL_CRITICAL = 70
)

// Logger is the main logger implementation
type message struct {
	msg            string
	level          int
	name           string
	dateTime       time.Time
	timezoneoffset int
}

type Logger struct {
	writers []ILogWriter

	// timezone options
	useCustomTimezone           bool
	customTimeZoneOffsetSeconds int
	buffer                      chan message
	//if true, next logged line will not receive a '\n' at end.
	// Useful when nextline should be composed with subsequent log calls
	keepNextLineOpened bool
}

func NewLogger(writers []ILogWriter, useCustomTimezone bool, tzOffsetSeconds int) *Logger {
	l := &Logger{writers: writers, useCustomTimezone: useCustomTimezone, customTimeZoneOffsetSeconds: tzOffsetSeconds}
	l.buffer = make(chan message, 100)

	go func() {
		for {
			nextMsg := <-l.buffer
			for _, w := range l.writers {
				w.Write(nextMsg.msg, nextMsg.level, nextMsg.name, nextMsg.dateTime, nextMsg.timezoneoffset)
			}
		}
	}()
	return l
}

// BeginComposedLine sets whether the next logged line will keep the line opened (no '\n' at end)
// Useful when nextline should be composed with subsequent log calls
//
//	when done, call EndComposedLine
func (l *Logger) KeepNextLineOpened() {
	l.keepNextLineOpened = true
}

// convenience methods
func (l *Logger) Log(level int, name string, msgs ...any) {
	now := time.Now()
	timezoneoffset := 0

	if l.useCustomTimezone {
		// apply custom timezone offset
		_, offset := now.Zone()
		totalOffset := l.customTimeZoneOffsetSeconds - offset
		now = now.Add(time.Duration(totalOffset) * time.Second)
		timezoneoffset = totalOffset
	} else {
		// use local timezone offset
		_, offset := now.Zone()
		timezoneoffset = offset
	}

	msg := ""
	for index, item := range msgs {
		if index > 0 {
			msg += " "
		}
		msg += loggerutils.AnyToString(item, true)
	}

	if !l.keepNextLineOpened {
		//ensure message ends with '\n'
		if !strings.HasSuffix(msg, "\n") {
			msg += "\n"
		}
	}

	l.buffer <- message{msg: msg, level: level, name: name, dateTime: now, timezoneoffset: timezoneoffset}
	l.keepNextLineOpened = false
}

func (l *Logger) Trace(name string, msgs ...any) {
	l.Log(LEVEL_TRACE, name, msgs...)
}
func (l *Logger) Debug2(name string, msgs ...any) {
	l.Log(LEVEL_DEBUG2, name, msgs...)
}
func (l *Logger) Debug(name string, msgs ...any) {
	l.Log(LEVEL_DEBUG, name, msgs...)
}
func (l *Logger) Info2(name string, msgs ...any) {
	l.Log(LEVEL_INFO2, name, msgs...)
}
func (l *Logger) Info(name string, msgs ...any) {
	l.Log(LEVEL_INFO, name, msgs...)
}
func (l *Logger) Warning(name string, msgs ...any) {
	l.Log(LEVEL_WARNING, name, msgs...)
}
func (l *Logger) Error(name string, msgs ...any) {
	l.Log(LEVEL_ERROR, name, msgs...)
}
func (l *Logger) Critical(name string, msgs ...any) {
	l.Log(LEVEL_CRITICAL, name, msgs...)
}

// Ensure Logger implements ILogger
var _ ILogger = (*Logger)(nil)

// Implement the convenience default methods
func (l *Logger) LogDefault(level int, msg string) { l.Log(level, DEFAULT_LOG_NAME, msg) }
func (l *Logger) TraceDefault(msg string)          { l.Trace(DEFAULT_LOG_NAME, msg) }

func (l *Logger) GetNamedLogger(name string) INamedLogger {
	return NewNLogger(name, l)
}

func (l *Logger) GetNLog(name string) INamedLogger {
	return NewNLogger(name, l)
}

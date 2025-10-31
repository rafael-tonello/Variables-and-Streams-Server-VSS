package logwriters

import "time"

type LambdaWriter struct {
	fn func(msg string, level int, name string, dateTime time.Time, timezoneoffset int)
}

func NewLambdaWriter(f func(msg string, level int, name string, dateTime time.Time, timezoneoffset int)) *LambdaWriter {
	return &LambdaWriter{fn: f}
}

func (l *LambdaWriter) Write(msg string, level int, name string, dateTime time.Time, timezoneoffset int) {
	if l.fn != nil {
		l.fn(msg, level, name, dateTime, timezoneoffset)
	}
}

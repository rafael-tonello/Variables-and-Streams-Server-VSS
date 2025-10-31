package logwriters

import (
	"fmt"
	"os"
	"os/exec"
	"sync"
	"time"

	"rtonello/vss/sources/misc/logger/loggerutils"
)

type commandEntry struct {
	header string
	text   string
	name   string
	level  int
}

type CommandCallWriter struct {
	command     string
	minInterval time.Duration
	mu          sync.Mutex
	running     bool
	cache       []commandEntry
}

func NewCommandCallWriter(command string, minInterval time.Duration) *CommandCallWriter {
	w := &CommandCallWriter{command: command, minInterval: minInterval, running: true, cache: make([]commandEntry, 0)}
	go w.loop()
	return w
}

func (w *CommandCallWriter) Write(msg string, level int, name string, dateTime time.Time, timezoneoffset int) {
	header := loggerutils.GenerateLineBeginning(loggerutils.LevelToString(level, "INFO"), name, true, dateTime, timezoneoffset, true)
	w.mu.Lock()
	w.cache = append(w.cache, commandEntry{header: header, text: msg, name: name, level: level})
	w.mu.Unlock()
}

func (w *CommandCallWriter) loop() {
	for w.running {
		time.Sleep(w.minInterval)
		w.mu.Lock()
		if len(w.cache) == 0 {
			w.mu.Unlock()
			continue
		}
		local := w.cache
		w.cache = nil
		w.mu.Unlock()

		f, err := os.CreateTemp("", "logger_command_*.csv")
		if err != nil {
			continue
		}
		for _, e := range local {
			fmt.Fprintf(f, "\"%s\",\"%s\",\"%s\",%d\n", loggerutils.EscapeString(e.header), loggerutils.EscapeString(e.text), loggerutils.EscapeString(e.name), e.level)
		}
		f.Close()
		cmd := exec.Command("/bin/sh", "-c", w.command+" \""+f.Name()+"\"")
		cmd.Output()
		os.Remove(f.Name())
	}
}

package logwriters

import (
	"os"
	"os/exec"
	"strings"
	"sync"
	"time"

	"rtonello/vss/sources/misc/logger/loggerutils"
)

type FileWriter struct {
	filePath          string
	file              *os.File
	mu                sync.Mutex
	logLevel          int
	printMilliseconds bool
	maxFileSize       int64 // bytes
	lastLineOpened    bool
	currIdentPrefix   string
}

func NewFileWriter(path string, logLevel int, printMilliseconds bool, maxFileSize int64) (*FileWriter, error) {
	f, err := os.OpenFile(path, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return nil, err
	}
	return &FileWriter{filePath: path, file: f, logLevel: logLevel, printMilliseconds: printMilliseconds, maxFileSize: maxFileSize}, nil
}

func (fw *FileWriter) Write(msg string, level int, name string, dateTime time.Time, timezoneoffset int) {
	if level < fw.logLevel {
		return
	}

	header := ""
	if !fw.lastLineOpened {
		header = loggerutils.GenerateLineBeginning(loggerutils.LevelToString(level, "INFO"), name, true, dateTime, timezoneoffset, fw.printMilliseconds)
		fw.currIdentPrefix = strings.Repeat(" ", len(header))
	}

	out := header + loggerutils.Ident(msg, fw.currIdentPrefix)

	fw.mu.Lock()
	defer fw.mu.Unlock()
	if fw.file != nil {
		fw.file.WriteString(out)
		if fw.maxFileSize > 0 {
			if fi, err := fw.file.Stat(); err == nil {
				if fi.Size() > fw.maxFileSize {
					fw.startCompaction()
				}
			}
		}
	}

	fw.lastLineOpened = strings.HasSuffix(msg, "\n") == false
}

func (fw *FileWriter) startCompaction() {
	// Placeholder for compaction logic if needed
	//get datetime in the format YYYY-MM-DD-HH-MM-SS

	now := time.Now()
	dateTimeString := now.Format("2006-01-02-15-04-05")

	//separate the extension and the filename
	extension := ""
	fileNameWithNoExtension := fw.filePath
	if strings.Contains(fw.filePath, ".") {
		pos := strings.LastIndex(fw.filePath, ".")
		extension = fw.filePath[pos:]
		fileNameWithNoExtension = fw.filePath[:pos]
	}

	fileNameWithoutExtensionAndWithoutPath := fileNameWithNoExtension
	if strings.Contains(fileNameWithNoExtension, "/") {
		pos := strings.LastIndex(fileNameWithNoExtension, "/")
		fileNameWithoutExtensionAndWithoutPath = fileNameWithNoExtension[pos+1:]
	}

	//move the file to a temporary file and create a new one for the logs
	newFname := fileNameWithNoExtension + "-" + dateTimeString + extension
	if fw.file != nil {
		fw.file.Close()
	}

	os.Rename(fw.filePath, newFname)
	fw.file, _ = os.OpenFile(fw.filePath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)

	//create the destination folder if not exists
	historyDir := fw.filePath + ".history"
	os.MkdirAll(historyDir, os.ModePerm)

	//compact the file
	go func() {
		tarXzFile := historyDir + "/" + fileNameWithoutExtensionAndWithoutPath + "-" + dateTimeString + ".tar.xz"
		cmd := "nice -n 19 tar -cJf \"" + tarXzFile + "\" \"" + newFname + "\""
		exec.Command("sh", "-c", cmd).Run()
		//remove the temporary file
		os.Remove(newFname)
	}()

}

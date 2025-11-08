package sources

import (
	"bufio"
	"os"
	"rtonello/vss/sources/misc"
	"strings"
)

// FileSource reads configuration from a simple key=value file. Lines starting
// with '#' are ignored. Keys and values are trimmed.
type FileSource struct {
	kv map[string]string
}

// NewFileSource loads the file at path. If there is an error reading the file,
// the source will behave as empty.
func NewFileSource(path string) *FileSource {
	fs := &FileSource{kv: make(map[string]string)}
	f, err := os.Open(path)
	if err != nil {
		return fs
	}
	defer f.Close()
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		if parts := strings.SplitN(line, "=", 2); len(parts) == 2 {
			key := strings.TrimSpace(parts[0])
			val := strings.TrimSpace(parts[1])
			fs.kv[key] = val
		}
	}
	return fs
}

// Find looks for a key in the loaded map and a dots->underscore variant.
func (f *FileSource) Find(possibleNames []string) (misc.DynamicVar, string, bool) {
	for _, n := range possibleNames {
		if v, ok := f.kv[n]; ok {
			return misc.NewDynamicVar(v), n, true
		}
		alt := strings.ReplaceAll(n, ".", "_")
		if v, ok := f.kv[alt]; ok {
			return misc.NewDynamicVar(v), alt, true
		}
	}
	return misc.NewDynamicVar(""), "", false
}

func (f *FileSource) GetSourceInfo() string {
	return "Configuration File (" + f.kv["file"] + ")"
}

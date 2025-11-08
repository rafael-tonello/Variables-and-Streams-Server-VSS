package sources

import (
	"os"
	"rtonello/vss/sources/misc"
	"strings"
)

// CommandLineSource reads configuration from command line arguments.
// It accepts arguments in the form `key=value` or `--key=value`.
type CommandLineSource struct {
	kv map[string]string
}

// NewCommandLineSource parses the provided args slice. If args is nil, it
// uses os.Args[1:].
func NewCommandLineSource(args []string) *CommandLineSource {
	if args == nil {
		args = os.Args[1:]
	}
	kv := make(map[string]string)
	for i, a := range args {
		if strings.Contains(a, "=") {
			if parts := strings.SplitN(a, "=", 2); len(parts) == 2 {
				key := parts[0]
				val := parts[1]
				kv[key] = val
			}
			continue
		} else if strings.Contains(a, ":") {
			if parts := strings.SplitN(a, ":", 2); len(parts) == 2 {
				key := parts[0]
				val := parts[1]
				kv[key] = val
			}
		} else {
			//val is the next argument
			if len(args) > i+1 {
				key := a
				val := args[i+1]
				kv[key] = val
			}
		}
	}
	return &CommandLineSource{kv: kv}
}

// Find implements ConfsSource. It checks each possible name in order and
// returns the first matching value.
func (c *CommandLineSource) Find(possibleNames []string) (misc.DynamicVar, string, bool) {
	for _, n := range possibleNames {
		if v, ok := c.kv[n]; ok {
			return misc.NewDynamicVar(v), n, true
		}
		// also try common variant replacing dots with underscores
		alt := strings.ReplaceAll(n, ".", "_")
		if v, ok := c.kv[alt]; ok {
			return misc.NewDynamicVar(v), alt, true
		}
	}
	return misc.NewDynamicVar(""), "", false
}

func (c *CommandLineSource) GetSourceInfo() string {
	return "Command Line Arguments"
}

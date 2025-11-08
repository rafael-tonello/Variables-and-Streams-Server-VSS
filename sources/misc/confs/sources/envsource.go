package sources

import (
	"os"
	"rtonello/vss/sources/misc"
	"strings"
)

// EnvSource reads configuration from environment variables.
type EnvSource struct{}

func NewEnvSource() *EnvSource { return &EnvSource{} }

// Find implements ConfsSource. It checks each possible name and common
// variants (uppercased, dots -> underscores) in the environment.
func (e *EnvSource) Find(possibleNames []string) (misc.DynamicVar, string, bool) {
	for _, n := range possibleNames {
		// direct lookup
		if v := os.Getenv(n); v != "" {
			return misc.NewDynamicVar(v), n, true
		}

		// try uppercase + dots->_ to follow common env conventions
		alt := strings.ToUpper(strings.ReplaceAll(n, ".", "_"))
		if v := os.Getenv(alt); v != "" {
			return misc.NewDynamicVar(v), alt, true
		}
	}
	return misc.NewDynamicVar(""), "", false
}

func (e *EnvSource) GetSourceInfo() string {
	return "Environment Variables"
}

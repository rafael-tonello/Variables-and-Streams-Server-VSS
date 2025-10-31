package loggerutils

import (
	"encoding/json"
	"fmt"
	"reflect"
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

func Ident(source, prefix string) string {
	ret := strings.ReplaceAll(source, "\n", "\n"+prefix)

	//remove the last prefix if source ends with \n
	if strings.HasSuffix(source, "\n") {
		ret = ret[:len(ret)-len(prefix)]
	}

	return ret
}

func EscapeString(s string) string {
	s = strings.ReplaceAll(s, "\\", "\\\\")
	s = strings.ReplaceAll(s, "\"", "\\\"")
	s = strings.ReplaceAll(s, "\n", "\\n")
	s = strings.ReplaceAll(s, "\r", "\\r")
	s = strings.ReplaceAll(s, "\t", "\\t")
	return s
}

// Generate line beginning, similar to C++ generateLineBegining
func GenerateLineBeginning(levelStr, name string, includeDateTime bool, dateTime time.Time, timezoneoffset int, includeMillis bool) string {
	result := ""
	if includeDateTime {
		result += "["
		if dateTime.IsZero() {
			dateTime = time.Now()
			//get current timezone off
			_, offset := dateTime.Zone()
			//apply the timezone offset
			dateTime = dateTime.Add(time.Duration(timezoneoffset-offset) * time.Second)
		}

		if includeMillis {
			result += dateTime.Format("2006-01-02T15:04:05.000")
		} else {
			result += dateTime.Format("2006-01-02T15:04:05")
		}

		//convert timezoneoffset (in seconds) to format +hhmm or -hhmm
		hours := timezoneoffset / 3600
		minutes := (timezoneoffset % 3600) / 60
		tmpRResult := fmt.Sprintf("%+03d%02d", hours, minutes)

		result += tmpRResult + "] "
	}
	result += "[" + levelStr + "] "
	if name != "" {
		result += "[" + name + "] "
	}
	return result
}

// helper wrapper so writers can get the textual level
func LevelToString(level int, defaultName string) string {
	switch level {
	case LEVEL_TRACE:
		return "TRACE"
	case LEVEL_DEBUG2:
		return "DEBUG2"
	case LEVEL_DEBUG:
		return "DEBUG"
	case LEVEL_INFO2:
		return "INFO2"
	case LEVEL_INFO:
		return "INFO"
	case LEVEL_WARNING:
		return "WARNING"
	case LEVEL_ERROR:
		return "ERROR"
	case LEVEL_CRITICAL:
		return "CRITICAL"
	default:
		return defaultName
	}
}

func AnyToString(value interface{}, enableJsonOutputIdent bool) string {

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
			return AnyToString(result[0].Interface(), enableJsonOutputIdent)
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

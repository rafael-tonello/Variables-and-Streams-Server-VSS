package misc

import (
	"encoding/json"
	"fmt"
	"reflect"
	"strconv"
	"strings"
)

type DynamicVar struct {
	data string
}

// Creates a new DynamicVar with a initial data. You can use the functions
// 'WithString', 'WithInt', 'WitFloat' and 'WithBool' to specify the initial
// value
func NewDynamicVar(data any) DynamicVar {
	ret := DynamicVar{}
	ret.data = anyToString(data, false)
	return ret
}

func NewDynamicVar2(option func(*DynamicVar)) DynamicVar {
	ret := DynamicVar{}
	option(&ret)
	return ret
}

func anyToString(value interface{}, enableJsonOutputIdent bool) string {

	msgsType := reflect.TypeOf(value)

	valueStr := ""

	switch msgsType.Kind() {
	case reflect.String:
		valueStr = value.(string)
	//case reflect.Struct:
	//	valueStr = fmt.Sprintf("%v", value)
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

//func stringToAny[T any](valueStr string) T {
//	var value interface{}
//	err := json.Unmarshal([]byte(valueStr), &value)
//	if err != nil {
//		value = valueStr
//	}
//	return value.(T)
//}

func NewEmptyDynamicVar() DynamicVar {
	ret := DynamicVar{}
	return ret
}

func WithString(value string) func(*DynamicVar) {
	return func(dn *DynamicVar) {
		dn.SetString(value)
	}
}

func WithInt(value int64) func(*DynamicVar) {
	return func(dn *DynamicVar) {
		dn.SetInt64(value)
	}
}

func WithFloat(value float64) func(*DynamicVar) {
	return func(dn *DynamicVar) {
		dn.SetFloat64(value)
	}
}

func WithBool(value bool) func(*DynamicVar) {
	return func(dn *DynamicVar) {
		dn.SetBool(value)
	}
}

func (dVar *DynamicVar) SetString(value string) {
	dVar.data = value
}

func (dVar *DynamicVar) GetString() string {
	return dVar.data
}

func (dVar *DynamicVar) SetInt(value int) {
	dVar.SetString(strconv.Itoa(int(value)))
}

func (dVar *DynamicVar) GetInt() int {
	r, _ := dVar.GetInte()
	return r
}

func (dVar *DynamicVar) GetInte() (int, error) {
	result, err := strconv.Atoi(dVar.data)
	if err != nil {
		return 0, err
	}

	return int(result), nil

}

func (dVar *DynamicVar) SetInt64(value int64) {
	dVar.SetString(strconv.Itoa(int(value)))
}

func (dVar *DynamicVar) GetInt64e() (int64, error) {
	result, err := strconv.Atoi(dVar.data)
	if err != nil {
		return 0, err
	}

	return int64(result), nil

}

func (dVar *DynamicVar) GetInt64() int64 {
	r, _ := dVar.GetInt64e()
	return r
}

func (dVar *DynamicVar) SetFloat64(value float64) {
	//strconv.FormatFloat(value, 'g', -1, 64);
	dVar.data = strconv.FormatFloat(value, 'f', -1, 64)
}

func (dVar *DynamicVar) GetFloat64e() (float64, error) {
	result, err := strconv.ParseFloat(dVar.data, 64)
	if err != nil {
		return 0, err
	}

	return result, nil
}

func (dVar *DynamicVar) GetFloat64() float64 {
	r, _ := dVar.GetFloat64e()
	return r

}

func (dVar *DynamicVar) SetBool(value bool) {
	if value {
		dVar.data = "1"
	} else {
		dVar.data = "0"
	}
}

func (dVar *DynamicVar) GetBool() bool {
	ret := strings.Contains("1trueyesok", strings.ToLower(dVar.data))
	return ret
}

func (dVar *DynamicVar) Set(value any) {
	dVar.data = anyToString(value, false)
}

// infer type
// possible return values: reflect.Int, reflect.Float64, reflect.Bool, reflect.String
func (dVar *DynamicVar) InferType() reflect.Kind {
	//try int
	if _, err := strconv.Atoi(dVar.data); err == nil {
		return reflect.Int
	}

	//try float
	if _, err := strconv.ParseFloat(dVar.data, 64); err == nil {
		return reflect.Float64
	}

	//try bool
	lowerData := strings.ToLower(dVar.data)
	if lowerData == "1" || lowerData == "0" || lowerData == "true" || lowerData == "false" || lowerData == "yes" || lowerData == "no" {
		return reflect.Bool
	}

	//default: string
	return reflect.String
}

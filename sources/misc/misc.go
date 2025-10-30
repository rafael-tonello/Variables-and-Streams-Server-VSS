package misc

import "strings"

func GetOnly(source string, validChars string) string {
	result := ""
	for _, c := range source {
		if strings.ContainsRune(validChars, c) {
			result += string(c)
		}
	}
	return result
}

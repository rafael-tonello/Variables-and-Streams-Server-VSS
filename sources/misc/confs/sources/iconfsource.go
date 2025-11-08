package sources

import "rtonello/vss/sources/misc"

type IConfsSource interface {
	Find(possibleNames []string) (misc.DynamicVar, string, bool)
	GetSourceInfo() string
}

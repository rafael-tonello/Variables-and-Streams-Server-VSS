package apis

import "rtonello/vss/sources/misc"

// Minimal ApiInterface used by ControllerClientHelper. Real APIs can implement this.
type IApi interface {
	GetApiId() string
	CheckAlive(clientId string) bool
	NotifyClient(clientId string, varsAndValues []misc.Tuple[string]) bool
}

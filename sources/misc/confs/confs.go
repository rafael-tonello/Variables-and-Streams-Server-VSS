package confs

import (
	"rtonello/vss/sources/misc"
	"strings"
)

type IConfItem interface {
	ConfName() string
	Value() misc.DynamicVar
	NotMappedValue() misc.DynamicVar
	SetPossibleNames(possibleNames []string) IConfItem
	SetDefaultValue(v misc.DynamicVar) IConfItem

	AddValueMap(placeHolders map[misc.DynamicVar]misc.DynamicVar)
}

type IConfs interface {
	// Get a configurations (create if not exists). Options can be provided to
	// set possible names in the sources and default value.
	Config(name string, options ...func(IConfItem)) IConfItem

	// Idiomatic alias for config to be used when defining configurations
	CreateConfig(name string, options ...func(IConfItem)) IConfItem

	// AddPlaceHolders adds new placeholders to the configuration system.
	// Placeholders are strings that will be replaced in configurations values.
	AddPlaceHolders(placeHolders map[string]string)

	// AddNewSource adds a new configuration source to the system.
	AddNewSource(source IConfsSource)

	// AllConfigs returns all defined configuration items.
	AllConfigs() map[string]IConfItem

	//AllPlaceholders returns all defined placeholders.
	AllPlaceHolders() map[string]string
}

// IConfsSource is the interface implemented by individual configuration
// sources (command line, environment, files). They live in package
// sources but we accept them here via this interface.
type IConfsSource interface {
	Find(possibleNames []string) (misc.DynamicVar, bool)
}

func WithPossibleNames(possibleNames []string) func(IConfItem) {
	return func(c IConfItem) {
		c.SetPossibleNames(possibleNames)
	}
}

func WithDefaultValue(v misc.DynamicVar) func(IConfItem) {
	return func(c IConfItem) {
		c.SetDefaultValue(v)
	}
}

func WithValueMap(placeHolders map[misc.DynamicVar]misc.DynamicVar) func(IConfItem) {
	return func(c IConfItem) {
		c.AddValueMap(placeHolders)
	}
}

type Confs struct {
	items        map[string]*ConfItem
	sources      []IConfsSource
	placeHolders map[string]string
}

type ConfItem struct {
	PossibleNames []string
	DefaultValue  misc.DynamicVar
	theValue      misc.DynamicVar
	Name          string
	ValueMap      map[string]misc.DynamicVar
}

// the configuration system looks for configurations in the sources. The first one that provides a value
// for a given configuration item "wins", making sources order important.
func NewConfs(sources []IConfsSource) IConfs {
	return &Confs{
		items:        make(map[string]*ConfItem),
		sources:      sources,
		placeHolders: make(map[string]string),
	}
}

func (c *Confs) AddNewSource(source IConfsSource) {
	c.sources = append(c.sources, source)
}

// Get a configurations (create if not exists). Options can be provided to
// set possible names in the sources and default value.
func (c *Confs) Config(name string, options ...func(IConfItem)) IConfItem {
	if item, exists := c.items[name]; exists {
		return item
	}

	newItem := &ConfItem{}
	newItem.Name = name
	newItem.ValueMap = make(map[string]misc.DynamicVar)

	for _, opt := range options {
		opt(newItem)
	}

	// resolve value from sources
	found := false
	for _, source := range c.sources {
		if v, ok := source.Find(newItem.PossibleNames); ok {
			newItem.theValue = misc.NewDynamicVar(c.applyPlaceHolders(v.GetString()))
			found = true
			break
		}
	}

	// if not found in sources, use default value
	if !found {
		newItem.theValue = misc.NewDynamicVar(c.applyPlaceHolders(newItem.DefaultValue.GetString()))
	}
	c.items[name] = newItem
	return newItem
}

// redirect to Config
func (c *Confs) CreateConfig(name string, options ...func(IConfItem)) IConfItem {
	return c.Config(name, options...)
}

func (c *Confs) AllPlaceHolders() map[string]string {
	return c.placeHolders
}

func (c *Confs) AddPlaceHolders(placeHolders map[string]string) {
	for name, v := range placeHolders {
		c.placeHolders[name] = v
	}
}

func (c *Confs) AllConfigs() map[string]IConfItem {
	result := make(map[string]IConfItem)
	for name, item := range c.items {
		result[name] = item
	}
	return result
}

func (c *Confs) applyPlaceHolders(value string) string {
	strVal := value
	for phName, phValue := range c.placeHolders {
		strVal = strings.ReplaceAll(strVal, phName, phValue)
	}
	return strVal
}

func (c *ConfItem) Value() misc.DynamicVar {
	if c.ValueMap != nil {
		if v, ok := c.ValueMap[strings.ToLower(c.theValue.GetString())]; ok {
			return v
		}
	}

	return c.theValue
}

func (c *ConfItem) SetPossibleNames(possibleNames []string) IConfItem {
	c.PossibleNames = possibleNames
	return c
}

func (c *ConfItem) SetDefaultValue(v misc.DynamicVar) IConfItem {
	c.DefaultValue = v
	return c
}

func (c *ConfItem) ConfName() string {
	return c.Name
}

func (c *ConfItem) AddValueMap(placeHolders map[misc.DynamicVar]misc.DynamicVar) {
	for k, v := range placeHolders {
		c.ValueMap[strings.ToLower(k.GetString())] = v
	}
}

func (c *ConfItem) NotMappedValue() misc.DynamicVar {
	return c.theValue
}

package confs

import (
	"rtonello/vss/sources/misc"
	"slices"
	"strings"
)

type IConfItem interface {
	ConfName() string

	//apply maps and returns the final value
	Value() *misc.DynamicVar

	// NotMappedValue returns the raw (not mapped) value of the configuration item.
	NotMappedValue() *misc.DynamicVar
	// SetPossibleNames sets the possible names for this configuration item in the sources.
	SetPossibleNames(possibleNames []string) IConfItem

	// SetDefaultValue sets the default value for this configuration item. Do not call validation here.
	SetDefaultValue(v misc.DynamicVar) IConfItem

	AddValueMap(placeHolders map[misc.DynamicVar]misc.DynamicVar) IConfItem

	// Set a validation function that will be called when retrieving configuration values.
	SetValidation(func(conf IConfItem) error) IConfItem

	// ValidateThisValue validates a given value using the configuration's validation function.
	ValidateThisValue(value misc.DynamicVar) error

	//Set value and validate and (if validate=true). Will override current value (defaultValue or any other found
	//by sources)
	ChangeCurrentValue(v misc.DynamicVar, validate bool) error

	ValidateCurrentValue() error

	GetUsedNameAnsSourceInfo() (string, string)
}

type IConfs interface {
	// Gets a configuration item by name. If it does not exist, it is created.
	GetConfig(name string, options ...func(IConfItem)) IConfItem

	// Idiomatic alias for config to be used when defining configurations
	CreateConfig(name string, options ...func(IConfItem)) (IConfItem, error)

	// FindConfByParamName looks up a configuration item by one of its possible names.
	FindConfByParamName(paramName string) (IConfItem, bool)

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
	Find(possibleNames []string) (misc.DynamicVar, string, bool)
	GetSourceInfo() string
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

func WithValidationFunc(f func(Conf IConfItem) error) func(IConfItem) {
	return func(c IConfItem) {
		c.SetValidation(f)
	}
}

type Confs struct {
	items        map[string]*ConfItem
	sources      []IConfsSource
	placeHolders map[string]string
}

type ConfItem struct {
	PossibleNames    []string
	DefaultValue     misc.DynamicVar
	theValue         misc.DynamicVar
	Name             string
	ValueMap         map[string]misc.DynamicVar
	ValidateFunc     func(conf IConfItem) error
	UsedNameInSource string
	SourceInfo       string
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
func (c *Confs) CreateConfig(name string, options ...func(IConfItem)) (IConfItem, error) {
	if item, exists := c.items[name]; exists {
		return item, nil
	}

	newItem := NewConfItem(name, options...).(*ConfItem)

	// resolve value from sources
	found := false
	for _, source := range c.sources {
		if v, foundName, ok := source.Find(newItem.PossibleNames); ok {
			newItem.theValue = misc.NewDynamicVar(c.applyPlaceHolders(v.GetString()))
			newItem.UsedNameInSource = foundName
			newItem.SourceInfo = source.GetSourceInfo()
			found = true
			break
		}
	}

	// if not found in sources, use default value
	if !found {
		newItem.theValue = misc.NewDynamicVar(c.applyPlaceHolders(newItem.DefaultValue.GetString()))
	}

	validationError := newItem.ValidateThisValue(newItem.theValue)
	if validationError != nil {
		// revert to default value if validation fails
		newItem.theValue = misc.NewDynamicVar(c.applyPlaceHolders(newItem.DefaultValue.GetString()))
	}

	c.items[name] = newItem
	return newItem, validationError
}

// redirect to Config
func (c *Confs) GetConfig(name string, options ...func(IConfItem)) IConfItem {
	conf, _ := c.CreateConfig(name, options...)
	return conf
}

func (c *Confs) FindConfByParamName(paramName string) (IConfItem, bool) {
	for _, item := range c.items {
		if slices.Contains(item.PossibleNames, paramName) {
			return item, true
		}
	}
	return nil, false
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

func NewConfItem(name string, options ...func(IConfItem)) IConfItem {
	newItem := &ConfItem{}
	newItem.Name = name
	newItem.ValueMap = make(map[string]misc.DynamicVar)
	newItem.ValidateFunc = func(conf IConfItem) error { return nil }
	newItem.SourceInfo = "No source"
	newItem.UsedNameInSource = name + " (default value, no source)"

	for _, opt := range options {
		opt(newItem)
	}

	return newItem
}

func (c *ConfItem) Value() *misc.DynamicVar {
	if c.ValueMap != nil {
		if v, ok := c.ValueMap[strings.ToLower(c.theValue.GetString())]; ok {
			return &v
		}
	}

	return &c.theValue
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

func (c *ConfItem) AddValueMap(placeHolders map[misc.DynamicVar]misc.DynamicVar) IConfItem {
	for k, v := range placeHolders {
		c.ValueMap[strings.ToLower(k.GetString())] = v
	}

	return c
}

func (c *ConfItem) NotMappedValue() *misc.DynamicVar {
	return &c.theValue
}

func (c *ConfItem) SetValidation(f func(conf IConfItem) error) IConfItem {
	c.ValidateFunc = f
	return c
}

func (c *ConfItem) ValidateThisValue(value misc.DynamicVar) error {
	if c.ValidateFunc != nil {
		return c.ValidateFunc(c)
	}
	return nil
}

func (c *ConfItem) ChangeCurrentValue(v misc.DynamicVar, validate bool) error {
	if validate {
		if err := c.ValidateThisValue(v); err != nil {
			return err
		}
	}
	c.theValue = v
	return nil
}

func (c *ConfItem) ValidateCurrentValue() error {
	return c.ValidateThisValue(c.theValue)
}

func (c *ConfItem) GetUsedNameAnsSourceInfo() (string, string) {
	return c.UsedNameInSource, c.SourceInfo
}

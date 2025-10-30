package misc

// Tuple represents a generic collection of elements.
type Tuple[T any] struct {
	elements []T
}

// NewTuple creates a new tuple from a list of elements.
func NewTuple[T any](elements ...T) Tuple[T] {
	return Tuple[T]{elements: elements}
}

// ElementAt returns the element at the specified position.
// Panics if the index is out of bounds.
func (t Tuple[T]) ElementAt(index int) T {
	return t.elements[index]
}

func (t Tuple[T]) At(index int) T {
	return t.elements[index]
}

// Len returns the number of elements in the tuple.
func (t Tuple[T]) Len() int {
	return len(t.elements)
}

func (t Tuple[T]) Unpack() []T {
	return t.elements
}

// First returns the first element of the tuple.
func (t Tuple[T]) First() T {
	return t.elements[0]
}

// Second returns the second element of the tuple.
func (t Tuple[T]) Second() T {
	return t.elements[1]
}

func (t Tuple[T]) Third() T {
	return t.elements[2]
}

func (t Tuple[T]) Fourth() T {
	return t.elements[3]
}

func (t Tuple[T]) Fifth() T {
	return t.elements[4]
}

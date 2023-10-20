#include "../include/Stack.h"

template <typename T>
Stack<T>::Stack()
{
	// Initialize vector size tracker to 0
	currentSize = 0;
}

template <typename T>
bool Stack<T>::isEmpty() const
{
	// If current size of vector is 0, it is empty
	return currentSize == 0;
}

template <typename T>
size_t Stack<T>::size() const
{
	// Self-explanatory
	return currentSize;
}

template <typename T>
void Stack<T>::push(const T& element)
{
	// Shoves the given element into the end of the vector
	// [1, 3, 2].push_back(4) -> [1, 3, 2, 4] 
	buffer.push_back(element);
	// Increment the current size when you insert an element into the vector
	currentSize++;
}

template <typename T>
void Stack<T>::pop()
{
	// Check if there are any elements in the vector first
	if (!isEmpty())
	{
		// Remove the element at the end of the vector
		// [1, 3, 2].pop_back -> [1, 3]
		buffer.pop_back();
		// Decrement the current size when you remove an element from the vector
		currentSize--;
	}
}

template <typename T>
T& Stack<T>::top()
{
	// Returns the element at the end of the vector as a reference
	return buffer.back();
}

template <typename T>
const T& Stack<T>::top() const
{
	// Returns the element at the end of the vector as a non-modifiable reference
	return buffer.back();
}
#ifndef HEAP_H
#define HEAP_H

#include <cstddef>
#include <vector>

/**
 * Abstract heap class.
 */
template <typename T>
class Heap {
public:

    /**
     * Method to insert a new value into the heap.
     *
     * @param val Value to insert into the heap.
     * @throw length_error
     */
    virtual void insert(const T &val) = 0;

    /**
     * Method that removes the top item from the heap.
     * This method removes and returns the value on the top of the heap.
     *
     * @return Value of the item removed from the top of the heap or NULL if
     * empty.
     */
    virtual T pop() = 0;

    /**
     * Method that returns the value of the top element on the heap.
     * This method does not remove the element from the heap.
     *
     * @return Value of the item on the top of the heap or NULL if empty.
     */
    T peek();

    /**
     * Method that checks if the heap is empty.
     *
     * @return True if heap is empty, false otherwise.
     */
    bool empty();

protected:
    std::vector<T> heap;

    /**
     * Method to heapify the heap.
     * This method will make sure the item at the specified index is placed
     * correctly in the heap.
     *
     * @param index Index to heapify.
     */
    virtual void heapify(const size_t &index) = 0;

    /**
     * UPDATE THIS!
     *
     * @param index Index of the child.
     * @return Index of the parent.
     */
    size_t getParentIndex(const size_t &index);

    /**
     * UPDATE THIS!
     *
     * @param index Index of the parent.
     * @return Index of the left child.
     * @throw out_of_range
     */
    size_t getLeftChildIndex(const size_t &index);

    /**
     * UPDATE THIS!
     *
     * @param index Index of the parent.
     * @return Index of the right child.
     * @throw out_of_range
     */
    size_t getRightChildIndex(const size_t &index);
};

#endif //HEAP_H


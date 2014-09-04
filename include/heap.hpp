#ifndef HEAP_H
#define HEAP_H

#include <cstddef>
#include <vector>
#include <stdexcept>

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
     * @return Value of the item removed from the top of the heap.
     * @throw logic_error
     */
    T pop() {
        T result;
        if (!heap.empty()) {
            result = heap[0];
            heap[0] = heap.back();
            heap.pop_back();
            heapify(0);
        } else {
            throw std::logic_error("Pop: Heap is empty.");
        }
        return result;
    }

    /**
     * Method that returns the value of the top element on the heap.
     * This method does not remove the element from the heap.
     *
     * @return Value of the item on the top of the heap.
     * @throw logic_error
     */
    T peek() {
        if (!heap.empty()) {
            return heap[0];
        } else {
            throw std::logic_error("Peek: Heap is empty.");
        }
    }

    /**
     * Method that checks if the heap is empty.
     *
     * @return True if heap is empty, false otherwise.
     */
    inline bool empty() {
        return heap.empty();
    }

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
     * Returns the parent index of the provided index.
     * (index + 1)/2 -1;
     *
     * @param index Index of the child.
     * @return Index of the parent.
     */
    inline size_t getParentIndex(const size_t &index) {
        return (index + 1)/2 - 1;
    }

    /**
     * Returns the right child index of the provided index.
     * (2 * index) + 1;
     *
     * @param index Index of the parent.
     * @return Index of the left child.
     * @throw out_of_range
     */
    inline size_t getLeftChildIndex(const size_t &index) {
        if (index > heap.max_size()/2 - 2) {
            throw std::out_of_range("Child index is too large.");
        }

        return (2 * index) + 1;
    }

    /**
     * Returns the right child index of the provided index.
     * (2 * index) + 2;
     *
     * @param index Index of the parent.
     * @return Index of the right child.
     * @throw out_of_range
     */
    inline size_t getRightChildIndex(const size_t &index) {
        if (index > heap.max_size()/2 - 2) {
            throw std::out_of_range("Child index is too large.");
        }

        return (2 * index) + 2;
    }
};

#endif //HEAP_H


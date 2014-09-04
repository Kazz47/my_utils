#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include "heap.hpp"

/**
 * Min-Heap class.
 */
template <typename T>
class MinHeap : public Heap<T> {
public:

    void insert(const T &val);
    T pop();

private:

    void heapify(const size_t &index);
};

#endif //MIN_HEAP_H


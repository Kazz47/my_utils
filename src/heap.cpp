#include "heap.hpp"

#include <stdexcept>

template <typename T>
T Heap<T>::peek() {
    if (!heap.empty()) {
        return heap[0];
    } else {
        return NULL;
    }
}

template <typename T>
bool Heap<T>::empty() {
    return heap.empty();
}

template <typename T>
size_t Heap<T>::getParentIndex(const size_t &index) {
    return (index + 1)/2 - 1;
}

template <typename T>
size_t Heap<T>::getLeftChildIndex(const size_t &index) {
    if (index > heap.max_size()/2 - 1) {
        throw std::out_of_range("Child index is too large.");
    }

    return (2 * index) + 1;
}

template <typename T>
size_t Heap<T>::getRightChildIndex(const size_t &index) {
    if (index > heap.max_size()/2 - 2) {
        throw std::out_of_range("Child index is too large.");
    }

    return (2 * index) + 2;
}

// List all Kernels used here.
template class Heap<int>;


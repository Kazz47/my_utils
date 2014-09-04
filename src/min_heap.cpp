#include "min_heap.hpp"

template <typename T>
void MinHeap<T>::insert(const T &val) {
    this->heap.push_back(val);
    size_t current_pos = this->heap.size() - 1;
    size_t parent_pos = this->getParentIndex(current_pos);

    while (this->heap[current_pos] < this->heap[parent_pos] && current_pos > 0)  {
        this->heap[current_pos] = this->heap[parent_pos];
        this->heap[parent_pos] = val;

        current_pos = parent_pos;
        parent_pos = this->getParentIndex(current_pos);
    }
}

template <typename T>
T MinHeap<T>::pop() {
    T result = NULL;
    if (!this->heap.empty()) {
        result = this->heap[0];
        this->heap[0] = this->heap.back();
        this->heap.pop_back();
        heapify(0);
    }
    return result;
}

template <typename T>
void MinHeap<T>::heapify(const size_t &index) {
}

// List all Kernels used here.
template class MinHeap<int>;


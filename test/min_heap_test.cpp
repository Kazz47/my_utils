#include "gtest/gtest.h"
#include "min_heap.hpp"

TEST(MinHeapTest, InitiallyEmpty) {
    MinHeap<int> heap;
    ASSERT_EQ(true, heap.empty());
}

TEST(MinHeapTest, PeekReturnsNullWhenEmpty) {
    MinHeap<int> heap;
    ASSERT_EQ(NULL, heap.peek());
}

TEST(MinHeapTest, PopReturnsNullWhenEmpty) {
    MinHeap<int> heap;
    ASSERT_EQ(NULL, heap.pop());
}

TEST(MinHeapTest, NotEmptyAfterInsert) {
    MinHeap<int> heap;
    heap.insert(1);
    ASSERT_EQ(false, heap.empty());
}

TEST(MinHeapTest, PeekReturnsValueAfterInsert) {
    MinHeap<int> heap;
    int expected_val = 1;
    heap.insert(expected_val);
    ASSERT_EQ(expected_val, heap.peek());
}

TEST(MinHeapTest, PopReturnsValueAfterInsert) {
    MinHeap<int> heap;
    int expected_val = 1;
    heap.insert(expected_val);
    ASSERT_EQ(expected_val, heap.pop());
}

TEST(MinHeapTest, HeapIsEmptyAfterPop) {
    MinHeap<int> heap;
    heap.insert(1);
    heap.pop();
    ASSERT_EQ(true, heap.empty());
}

TEST(MinHeapTest, PeekReturnsSmallestElement1) {
    MinHeap<int> heap;
    int expected_val = 1;
    int other_val = 2;
    heap.insert(other_val);
    heap.insert(expected_val);
    ASSERT_EQ(expected_val, heap.peek());
}

TEST(MinHeapTest, PeekReturnsSmallestElement2) {
    MinHeap<int> heap;
    int expected_val = 1;
    int other_val = 2;
    heap.insert(expected_val);
    heap.insert(other_val);
    ASSERT_EQ(expected_val, heap.peek());
}

TEST(MinHeapTest, PopReturnsSmallestElement1) {
    MinHeap<int> heap;
    int expected_val = 1;
    int other_val = 2;
    heap.insert(other_val);
    heap.insert(expected_val);
    ASSERT_EQ(expected_val, heap.pop());
}

TEST(MinHeapTest, PopReturnsSmallestElement2) {
    MinHeap<int> heap;
    int expected_val = 1;
    int other_val = 2;
    heap.insert(expected_val);
    heap.insert(other_val);
    ASSERT_EQ(expected_val, heap.pop());
}


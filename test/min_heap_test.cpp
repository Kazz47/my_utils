#include "gtest/gtest.h"
#include "min_heap.hpp"

#include <ctime>
#include <cstdlib>

TEST(MinHeapTest, InitiallyEmpty) {
    MinHeap<int> heap;
    ASSERT_EQ(true, heap.empty());
}

TEST(MinHeapTest, PeekThrowsWhenEmpty) {
    MinHeap<int> heap;
    try {
        heap.peek();
        FAIL();
    } catch (std::logic_error &e) {
        SUCCEED();
    }
}

TEST(MinHeapTest, PopThrowsWhenEmpty) {
    MinHeap<int> heap;
    try {
        heap.pop();
        FAIL();
    } catch (std::logic_error &e) {
        SUCCEED();
    }
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

TEST(MinHeapTest, SimpleUse1) {
    MinHeap<int> heap;
    heap.insert(0);
    heap.insert(1);
    heap.insert(5);
    heap.insert(8);
    heap.insert(5);
    heap.insert(9);
    ASSERT_EQ(0, heap.pop());
    ASSERT_EQ(1, heap.pop());
    ASSERT_EQ(5, heap.pop());
    ASSERT_EQ(5, heap.pop());
    ASSERT_EQ(8, heap.pop());
    ASSERT_EQ(9, heap.pop());
    ASSERT_EQ(true, heap.empty());
}

TEST(MinHeapTest, SimpleUse2) {
    MinHeap<int> heap;
    heap.insert(0);
    heap.insert(1);
    heap.insert(5);
    heap.insert(8);
    heap.insert(5);
    heap.insert(9);
    ASSERT_EQ(0, heap.pop());
    ASSERT_EQ(1, heap.pop());
    ASSERT_EQ(5, heap.pop());
    heap.insert(7);
    heap.insert(2);
    heap.insert(-8);
    ASSERT_EQ(-8, heap.pop());
    ASSERT_EQ(2, heap.pop());
    ASSERT_EQ(5, heap.pop());
    ASSERT_EQ(7, heap.pop());
    ASSERT_EQ(8, heap.pop());
    ASSERT_EQ(9, heap.pop());
    ASSERT_EQ(true, heap.empty());
}

TEST(MinHeapFuzzer, DISABLED_RandomFuzz) {
    MinHeap<int> heap;
    srand(time(NULL));
    for (size_t i = 0; i < 1000000; i++) {
        float p = (double)rand()/RAND_MAX;
        if (p < 0.8) {
            heap.insert(1);
        } else {
            try {
                heap.pop();
            } catch(std::logic_error &e) {
                ASSERT_EQ(true, heap.empty());
            }
        }
    }
    while (true) {
        try {
            heap.pop();
        } catch(std::logic_error &e) {
            break;
        }
    }
    ASSERT_EQ(true, heap.empty());
}

//This test will fill the heap.
//WARNING: Uses a rediculious amount of memory and is disabled by default.
TEST(MinHeapFuzzer, DISABLED_FillIt) {
    MinHeap<int> heap;
    while(true) {
        heap.insert(0);
    }
}


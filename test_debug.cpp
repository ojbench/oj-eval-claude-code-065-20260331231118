#include "src.hpp"
#include <iostream>

int usedBlocks = 0;
int usedSpace = 0;
int maxUsedSpace = 0;

int* getNewBlock(int n) {
    if (n <= 0) return nullptr;
    usedBlocks += n;
    std::cout << "getNewBlock called with n=" << n << ", total blocks=" << usedBlocks << std::endl;
    return new int[n * 4096 / sizeof(int)];
}

void freeBlock(const int* block, int n) {
    if (block == nullptr || n <= 0) return;
    std::cout << "freeBlock called with n=" << n << std::endl;
    delete[] block;
}

int* allocate(Allocator& allocator, int n) {
    usedSpace += n;
    if (usedSpace > maxUsedSpace) maxUsedSpace = usedSpace;
    std::cout << "allocate(" << n << "), usedSpace=" << usedSpace << ", maxUsedSpace=" << maxUsedSpace << std::endl;
    return allocator.allocate(n);
}

void deallocate(Allocator& allocator, int* pointer, int n) {
    usedSpace -= n;
    std::cout << "deallocate(" << n << "), usedSpace=" << usedSpace << std::endl;
    allocator.deallocate(pointer, n);
}

bool check(Allocator& allocator) {
    bool result = (usedBlocks - 1) * 4096 / sizeof(int) / 2 < maxUsedSpace;
    std::cout << "Check: usedBlocks=" << usedBlocks
              << ", (usedBlocks-1)*1024/2=" << (usedBlocks - 1) * 4096 / sizeof(int) / 2
              << ", maxUsedSpace=" << maxUsedSpace
              << ", result=" << (result ? "PASS" : "FAIL") << std::endl;
    return result;
}

// Test 4: Multiple small data requests
void test4() {
    std::cout << "\n=== Test 4: Multiple small allocations ===" << std::endl;
    usedBlocks = 0;
    usedSpace = 0;
    maxUsedSpace = 0;

    Allocator allocator;
    for (int i = 0; i < 5000; ++i) {
        int* a = allocate(allocator, 1);
        *a = i;
    }

    if (!check(allocator)) {
        std::cout << "Test 4 FAILED: Using too much space" << std::endl;
    } else {
        std::cout << "Test 4 PASSED" << std::endl;
    }
}

int main() {
    test4();
    return 0;
}

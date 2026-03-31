#include "src.hpp"
#include <iostream>

int usedBlocks = 0;

int* getNewBlock(int n) {
    if (n <= 0) return nullptr;
    usedBlocks += n;
    std::cout << "getNewBlock called with n=" << n << ", total blocks=" << usedBlocks << std::endl;
    return new int[n * 4096 / sizeof(int)];
}

void freeBlock(const int* block, int n) {
    if (block == nullptr || n <= 0) return;
    usedBlocks -= n;
    std::cout << "freeBlock called with n=" << n << ", total blocks=" << usedBlocks << std::endl;
    delete[] block;
}

// Test 5: Last allocation reuse
void test5() {
    std::cout << "\n=== Test 5: Last allocation reuse ===" << std::endl;
    usedBlocks = 0;

    Allocator allocator;
    int* a = allocator.allocate(50);
    std::cout << "Allocated 50 at " << a << std::endl;

    int* b = allocator.allocate(2000);
    std::cout << "Allocated 2000 at " << b << std::endl;

    allocator.deallocate(b, 2000);
    std::cout << "Deallocated 2000" << std::endl;

    int* c = allocator.allocate(2000);
    std::cout << "Allocated 2000 at " << c << std::endl;

    if (b == c) {
        std::cout << "PASS: Reused same space" << std::endl;
    } else {
        std::cout << "FAIL: Did not reuse space (b=" << b << ", c=" << c << ")" << std::endl;
    }

    allocator.deallocate(c, 2000);
    std::cout << "Deallocated 2000" << std::endl;
}

int main() {
    test5();
    return 0;
}

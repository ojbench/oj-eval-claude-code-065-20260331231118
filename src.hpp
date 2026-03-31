#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <vector>
#include <algorithm>

/**
 * Get specified 4096 * n bytes from the memory.
 * @param n
 * @return the address of the block
 */
int* getNewBlock(int n);

/**
 * Free specified 4096 * n bytes from the memory.
 * @param block the pointer to the block
 * @param n
 */
void freeBlock(const int* block, int n);

class Allocator {
private:
    struct Block {
        int* start;           // Start of the block
        int blockCount;       // Number of 4096-byte blocks
        int capacity;         // Total ints in this block
        int used;             // Number of ints currently used

        Block(int* s, int bc, int cap) : start(s), blockCount(bc), capacity(cap), used(0) {}
    };

    std::vector<Block> blocks;
    int* lastAllocatedPtr;      // Track the last allocated pointer
    int lastAllocatedSize;       // Track the size of last allocation

public:
    Allocator() : lastAllocatedPtr(nullptr), lastAllocatedSize(0) {
    }

    ~Allocator() {
        // Free all blocks
        for (auto& block : blocks) {
            freeBlock(block.start, block.blockCount);
        }
    }

    /**
     * Allocate a sequence of memory space of n int.
     * @param n
     * @return the pointer to the allocated memory space
     */
    int* allocate(int n) {
        // Check if we can reuse the last allocated space if it was deallocated
        if (lastAllocatedPtr != nullptr) {
            // Find which block this belongs to
            for (auto& block : blocks) {
                if (lastAllocatedPtr >= block.start &&
                    lastAllocatedPtr < block.start + block.capacity) {
                    // Check if there's enough space
                    int offset = lastAllocatedPtr - block.start;
                    if (offset + n <= block.capacity) {
                        int* result = lastAllocatedPtr;
                        // Update the block's used counter if needed
                        if (offset + n > block.used) {
                            block.used = offset + n;
                        }

                        // Update last allocated
                        lastAllocatedPtr = result;
                        lastAllocatedSize = n;

                        return result;
                    }
                    break;
                }
            }
            // If we reach here, the reuse didn't work, continue to normal allocation
            lastAllocatedPtr = nullptr;
        }

        // Try to find space in existing blocks
        for (auto& block : blocks) {
            if (block.capacity - block.used >= n) {
                int* result = block.start + block.used;
                block.used += n;

                // Track as last allocated
                lastAllocatedPtr = result;
                lastAllocatedSize = n;

                return result;
            }
        }

        // Need to allocate a new block
        int intsPerBlock = 4096 / sizeof(int);  // 1024 ints per 4096 bytes
        int blocksNeeded = (n + intsPerBlock - 1) / intsPerBlock;

        int* newBlock = getNewBlock(blocksNeeded);
        int capacity = blocksNeeded * intsPerBlock;

        blocks.push_back(Block(newBlock, blocksNeeded, capacity));
        Block& block = blocks.back();

        int* result = block.start;
        block.used = n;

        // Track as last allocated
        lastAllocatedPtr = result;
        lastAllocatedSize = n;

        return result;
    }

    /**
     * Deallocate the memory that is allocated by the allocate member
     * function. If n is not the number that is called when allocating,
     * the behaviour is undefined.
     */
    void deallocate(int* pointer, int n) {
        // Check if this is the last allocation
        if (pointer == lastAllocatedPtr && n == lastAllocatedSize) {
            // Mark that this space is available for reuse
            // Keep lastAllocatedPtr pointing to this location
            // so it can be reused in the next allocate call
            return;
        }

        // For other deallocations, check if it's at the end of a block
        for (auto& block : blocks) {
            if (pointer >= block.start && pointer < block.start + block.capacity) {
                // Check if this deallocation is at the end of the used space
                if (pointer + n == block.start + block.used) {
                    // Reclaim this space
                    block.used -= n;

                    // Update last allocated pointer
                    if (block.used > 0) {
                        lastAllocatedPtr = block.start + block.used;
                        lastAllocatedSize = 0;
                    } else {
                        lastAllocatedPtr = nullptr;
                        lastAllocatedSize = 0;
                    }
                }
                break;
            }
        }
    }
};

#endif // ALLOCATOR_HPP

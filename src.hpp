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
        int used;             // Number of ints currently used (high water mark)

        Block(int* s, int bc, int cap) : start(s), blockCount(bc), capacity(cap), used(0) {}
    };

    struct FreeRegion {
        int* start;
        int size;

        FreeRegion(int* s, int sz) : start(s), size(sz) {}
    };

    std::vector<Block> blocks;
    std::vector<FreeRegion> freeRegions;  // Track deallocated regions
    int* lastAllocatedPtr;                 // Track the last allocated pointer
    int lastAllocatedSize;                  // Track the size of last allocation

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
        // First, try to reuse the last allocated space if it was deallocated
        if (lastAllocatedPtr != nullptr) {
            // Find which block this belongs to
            for (auto& block : blocks) {
                if (lastAllocatedPtr >= block.start &&
                    lastAllocatedPtr < block.start + block.capacity) {
                    int offset = lastAllocatedPtr - block.start;
                    if (offset + n <= block.capacity) {
                        int* result = lastAllocatedPtr;
                        if (offset + n > block.used) {
                            block.used = offset + n;
                        }
                        lastAllocatedPtr = result;
                        lastAllocatedSize = n;
                        return result;
                    }
                    break;
                }
            }
            lastAllocatedPtr = nullptr;
        }

        // Try to find space in free regions
        for (size_t i = 0; i < freeRegions.size(); ++i) {
            if (freeRegions[i].size >= n) {
                int* result = freeRegions[i].start;
                // Update or remove the free region
                if (freeRegions[i].size == n) {
                    freeRegions.erase(freeRegions.begin() + i);
                } else {
                    freeRegions[i].start += n;
                    freeRegions[i].size -= n;
                }
                lastAllocatedPtr = result;
                lastAllocatedSize = n;
                return result;
            }
        }

        // Try to find space in existing blocks (at the end of used space)
        for (auto& block : blocks) {
            if (block.capacity - block.used >= n) {
                int* result = block.start + block.used;
                block.used += n;
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
        // Check if this is the last allocation - special handling for reuse
        if (pointer == lastAllocatedPtr && n == lastAllocatedSize) {
            // Keep lastAllocatedPtr pointing here so it can be reused
            return;
        }

        // For other deallocations, check if it's at the end of a block
        for (size_t blockIdx = 0; blockIdx < blocks.size(); ++blockIdx) {
            Block& block = blocks[blockIdx];
            if (pointer >= block.start && pointer < block.start + block.capacity) {
                // Check if this is at the end of the used space
                if (pointer + n == block.start + block.used) {
                    // Reclaim this space by reducing used
                    block.used -= n;

                    // Update lastAllocatedPtr to point to the new end
                    if (block.used > 0) {
                        lastAllocatedPtr = block.start + block.used;
                        lastAllocatedSize = 0;  // Unknown size
                    } else {
                        // Block is now empty, mark the beginning as available
                        lastAllocatedPtr = block.start;
                        lastAllocatedSize = 0;
                    }
                } else {
                    // Not at the end - add to free regions
                    freeRegions.push_back(FreeRegion(pointer, n));

                    // Try to merge adjacent free regions
                    for (size_t i = 0; i < freeRegions.size(); ++i) {
                        for (size_t j = i + 1; j < freeRegions.size(); ++j) {
                            // Check if regions are adjacent
                            if (freeRegions[i].start + freeRegions[i].size == freeRegions[j].start) {
                                freeRegions[i].size += freeRegions[j].size;
                                freeRegions.erase(freeRegions.begin() + j);
                                --j;
                            } else if (freeRegions[j].start + freeRegions[j].size == freeRegions[i].start) {
                                freeRegions[i].start = freeRegions[j].start;
                                freeRegions[i].size += freeRegions[j].size;
                                freeRegions.erase(freeRegions.begin() + j);
                                --j;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
};

#endif // ALLOCATOR_HPP

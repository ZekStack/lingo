#pragma once

#include <cstddef>
#include <cstdlib>

#define MALLOC_CAP_SPIRAM 0x1
#define MALLOC_CAP_8BIT 0x2

namespace lingo_psram_test {

inline std::size_t spiramTotal = 0;
inline bool failSpiramAllocation = false;
inline std::size_t spiramAllocations = 0;
inline std::size_t internalAllocations = 0;

inline void reset() {
	spiramTotal = 0;
	failSpiramAllocation = false;
	spiramAllocations = 0;
	internalAllocations = 0;
}

} // namespace lingo_psram_test

inline std::size_t heap_caps_get_total_size(int caps) {
	if ((caps & MALLOC_CAP_SPIRAM) != 0) {
		return lingo_psram_test::spiramTotal;
	}
	return 0;
}

inline void *heap_caps_malloc(std::size_t size, int caps) {
	if ((caps & MALLOC_CAP_SPIRAM) != 0) {
		++lingo_psram_test::spiramAllocations;
		if (lingo_psram_test::failSpiramAllocation) {
			return nullptr;
		}
		return std::malloc(size);
	}

	++lingo_psram_test::internalAllocations;
	return std::malloc(size);
}

inline void heap_caps_free(void *ptr) {
	std::free(ptr);
}

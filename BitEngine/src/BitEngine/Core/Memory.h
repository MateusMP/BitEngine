#pragma once

#include "bitengine/Core/Assert.h"

namespace BitEngine {

	class MemoryArena {
		public:

		void init(u8* b, ptrsize s) {
			base = b;
			size = s;
			used = 0;
		}

		template<typename T, typename ... Args>
		T* push(Args &&... args) {			
			T* ptr = (T*) alloc(sizeof(T));

			// Initialize type
			new (ptr) T(std::forward<Args&&>(args)...);

			return ptr;
		}

		template<typename T, u32 length>
		T* pushArray() {
			BE_ASSERT(length > 0);
			T* ptr = (T*)alloc(sizeof(T) * length);

			new (ptr) T[length];
		
			return ptr;
		}

		void clear() {
			used = 0;
		}

		u8* endPtr() {
			return base + size;
		}

		void* alloc(ptrsize allocSize) {
			BE_ASSERT(used + allocSize < size);

			void* ptr = base + used;
			used += allocSize;
			return ptr;
		}


		u8* base;
		ptrsize size;
		ptrsize used;
	};
}
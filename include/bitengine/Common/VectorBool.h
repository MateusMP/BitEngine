#pragma once

#include <vector>

#include "Common/TypeDefinition.h"

namespace BitEngine {

	class VectorBool
	{
	    typedef uint8 baseType;
	    static constexpr uint32 Nb = sizeof(baseType)*8;
	public:

	    static constexpr baseType BIT_AT(baseType b){
	        return BIT_MASK << b;
	    }

		enum {
			BIT_MASK = 0x01,
		};

		VectorBool()
		{
			numElements = 0;
		}

		// number of bits in use
		uint32 size() const {
			return numElements;
		}

		// set given bit index to 1
		void set(uint32 i) {
			data[i / Nb] |= BIT_AT(i%Nb);
		}

		// set given bit index to 0
		void unset(uint32 i) {
			data[i / Nb] &= ~BIT_AT(i%Nb);
		}

		bool test(uint32 i) {
			return (data[i / Nb] & BIT_AT(i%Nb)) > 0;
		}

		// get bit value at index i
		bool operator[](uint32 i) const {
			return (data[i / Nb] & BIT_AT(i%Nb)) > 0;
		}

		template<typename T>
		T getAtIndex(uint32 i) const{
            return *(reinterpret_cast<const T*>(&data[i]));
		}

		// Resize to contain at least size bits
		void resize(uint32 size)
		{
			int nBytes = size / Nb;
			if (size % Nb != 0)
				++nBytes;

			data.resize(nBytes, 0);
			numElements = size;
		}

		// Add another bit at the end
		void push_back(bool value)
		{
			if (numElements / Nb == data.size())
			{
				data.emplace_back(value);
				++numElements;
				return;
			}

			if (value) {
				set(numElements);
			}
			else {
				unset(numElements);
			}

			++numElements;
		}

	private:
		std::vector<baseType> data;
		uint32 numElements;
	};

}

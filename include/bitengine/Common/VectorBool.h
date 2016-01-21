#pragma once

#include <vector>

#include "Common/TypeDefinition.h"

namespace BitEngine {

	class VectorBool
	{
	public:
		enum {
			BIT_MASK = 0x01,
		};

		VectorBool()
		{
			numElements = 0;
		}

		uint32 size() const {
			return numElements;
		}

		void set(uint32 i) {
			data[i / 8] |= (BIT_MASK << (i % 8));
		}

		void unset(uint32 i) {
			data[i / 8] &= ~(BIT_MASK << (i % 8));
		}

		void resize(uint32 size)
		{
			int nBytes = size / 8;
			if (size % 8 != 0)
				++nBytes;

			data.resize(nBytes, 0);
			numElements = size;
		}

		void push_back(bool value)
		{
			if (numElements / 8 == data.size())
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

		const bool operator[](uint32 i) const {
			return (data[i / 8] >> (i % 8)) & 1;
		}

	private:
		std::vector<uint8> data;
		uint32 numElements;
	};

}
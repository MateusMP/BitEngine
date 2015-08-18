#pragma once

#include "TypeDefinition.h"
#include <vector>

class VectorBool
{
public:
	enum{
		BIT_MASK = 0x01,
	};

	VectorBool()
	{
		numElements = 0;
	}

	int getSize() const {
		return numElements;
	}

	void set(int i){
		data[i / 8] |= (BIT_MASK << (i % 8));
	}

	void unset(int i){
		data[i / 8] &= ~(BIT_MASK << (i % 8));
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

	const bool operator[](int i) const{
		return (data[i / 8] >> (i % 8)) & 1;
	}

private:
	std::vector<uint8> data;
	int numElements;
};
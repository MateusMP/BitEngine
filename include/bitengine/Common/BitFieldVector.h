#pragma once

#include "Common/VectorBool.h"
#include "Core/Logger.h"

namespace BitEngine{

	union BitMask
	{
		uint64 b64;
        uint8 b8[8]; // 64 bits
	};

	class ObjBitField
	{
		public:
		ObjBitField(uint32 numBitsPerObj)
			: m_numBitsPerObj(numBitsPerObj), m_numObjs(0)
		{
			m_baseMask.b64 = 0;
			for (uint32 i = 0; i < m_numBitsPerObj; ++i) {
				m_baseMask.b8[i / 8] |= VectorBool::BIT_AT(i % 8);
			}
		}

		BitMask getBaseBitMask() const {
			return m_baseMask;
		}

		// Clear any bit that may not be used on an object
		// Ex:
		// using 6 bits per obj, applying a mask to
		// 1110 1111 would output 0010 1111
		void applyMask(BitMask& a) {
			a.b64 &= m_baseMask.b64;
		}

		uint32 getBitPerObj() const {
			return m_numBitsPerObj;
		}

		void push() {
			++m_numObjs;
			m_bits.resize(m_numObjs*m_numBitsPerObj + 8*8);
		}

		uint32 objCount() const {
			return m_numObjs;
		}

		bool test(uint32 objIndex, uint16 bitIndex) const {
			return m_bits[objIndex*m_numBitsPerObj + bitIndex];
		}

		void set(uint32 objIndex, uint16 bitIndex) {
			m_bits.set(objIndex*m_numBitsPerObj + bitIndex);
		}

		void unset(uint32 objIndex, uint16 bitIndex) {
			m_bits.unset(objIndex*m_numBitsPerObj + (uint32)bitIndex);
		}

		void unsetAll(uint32 objIndex)
		{
			const uint32 obji = objIndex*m_numBitsPerObj;

			// TODO: optimize using masks

			for (uint32 i = 0; i < m_numBitsPerObj; ++i)
				m_bits.unset(obji + i);
		}

		bool objContains(uint32 objIndex, BitMask a) const
		{
			a.b64 = a.b64 & m_baseMask.b64;
			return (getObj(objIndex).b64 & a.b64) == a.b64;
		}

		BitMask getObj(uint32 objIndex) const
		{
		    const constexpr uint32 BitMax = 64;
			const uint32 bitIndex = objIndex*m_numBitsPerObj;
			const uint32 byteIndex = bitIndex/8;
            const uint32 rotation = bitIndex%8;
            const uint32 rotationR = 8-rotation;

            BitMask obj{0};

			uint64 b1;
			uint8 b2;
			b1 = m_bits.getAtIndex<uint64>(byteIndex);
			b2 = m_bits.getAtIndex<uint8>(byteIndex + 8);

			// debug
			//if (objIndex == 1 && m_numBitsPerObj == 63){
            //    for (int x = 0; x < 8; ++x)
            //        printf("%d: %u\n", x, m_bits.getAtIndex<uint8>(x));
            //    printf("b1: %016llX -- hexa\n", b1);
            //    printf("b2: %u\n", b2);
			//}

			obj.b64 = (b1 >> rotation);
			b2 <<= rotationR;
			obj.b64 |= (uint64(b2) << 56); // (MaxBytes-1)*8

			obj.b64 &= m_baseMask.b64;

			return obj;
		}

		private:
            const uint32 m_numBitsPerObj; // up to 2kkk bits
			uint32 m_numObjs;

            BitMask m_baseMask;
            VectorBool m_bits;
	};
}


#include <iostream>
#include <bitset>
#include <random>

#include "bitengine.h"

using namespace BitEngine;

Logger TestLog("TestBitSet", std::cout);


std::string BitMaskStr(const BitMask& s){
    std::ostringstream str;
    for (int i = 0; i < 8; ++i){
        str << std::bitset<8>(s.b8[i]);
    }
    return str.str();
}

bool checkIfAllZeros(const ObjBitField& obf, const std::string& idf)
{
    const u32 nObjs = obf.objCount();
    const u32 nBits = obf.getBitPerObj();


    // check every bit
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 b = 0; b < nBits; ++b)
        {
            if (obf.test(i, b))
            {
                LOG(TestLog, BE_LOG_ERROR) << "ERROR: "<<idf<<" object at index " << i << " not zeroed [bit check].";
                return false;
            }
        }
    }

    // check object mask
    for (u32 i = 0; i < nObjs; ++i)
    {
        u64 x = obf.getObj(i).b64;
        if (x != 0)
        {
            LOG(TestLog, BE_LOG_INFO) << "ERROR: "<<idf<<" obj at " << i << " not zeroed! Found: " << x;
            return false;
        }
    }

    return true;
}

bool testBitCase(u16 nBits)
{
    const u32 nObjs = 10000;
    u64 seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::mt19937_64 generator(seed);

    ObjBitField bitfield(nBits);

    const BitMask bitmask = bitfield.getBaseBitMask();
    BitMask compareMask;
    compareMask.b64 = 0;
    for (u32 i = 0; i < nBits; ++i){
        compareMask.b8[i/8] |= VectorBool::BIT_AT(i%8);
    }
    if ( compareMask.b64 != bitmask.b64 ){
        LOG(TestLog, BE_LOG_ERROR) << "ERROR: Wrong bitmask " << bitmask.b64 << " expected: " << compareMask.b64;
        return false;
    }

    //LOG(TestLog, BE_LOG_INFO) << "Creating " << nObjs << " bitfield objects with " << nBits << " each";
    for (u32 i = 0; i < nObjs; ++i)
    {
        bitfield.push();
    }

    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 b = 0; b < nBits; ++b)
        {
            bitfield.unset(i, b);
            if (bitfield.test(i, b)){
                return false;
            }
        }
    }

    // LOG(TestLog, BE_LOG_INFO) << "Testing if all objects are set to 0";
    if (!checkIfAllZeros(bitfield, "[1]")){
        return false;
    }

    // LOG(TestLog, BE_LOG_INFO) << "OK: all objects bits are 0.";
//    bitfield.set(0, 0);
//    bitfield.set(0, 1);
//    bitfield.set(0, 3);
//    bitfield.set(0, 5);
//    bitfield.set(0, 7);
//    bitfield.set(0, 9);
//    bitfield.set(0, 10);
    //LOG(TestLog, BE_LOG_ERROR) << "BITS: " << std::bitset<8>(bitfield.getObj(0).b8[0]) << std::bitset<8>(bitfield.getObj(0).b8[1]);

    //LOG(TestLog, BE_LOG_INFO) << "Set even bits to 1";
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 b = 0; b < nBits; ++b)
        {
            if (b%2 == 0)
            {
                bitfield.set(i, b);
            } else {
                bitfield.unset(i, b);
            }
        }
    }

    //LOG(TestLog, BE_LOG_INFO) << "Test if even bits are 1";
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 b = 0; b < nBits; ++b)
        {
            bool test = bitfield.test(i, b);
            bool expected = (b%2==0);
            if (test != expected)
            {
                LOG(TestLog, BE_LOG_ERROR) << "ERROR: object at index " << i << " have bit " << b << " value: " << bitfield.test(i, b);
                return false;
            }
        }
    }

    //LOG(TestLog, BE_LOG_INFO) << "Clear bitset for some objects and check if all were changed";
    //u32 objClear = 3;// generator()%7 + 1;
    u32 objRandom = 5;// generator()%7 + 1;

    u32 nRandomBits = nBits / 2 + 1;

    std::vector<u16> randomBits;
    for (u32 i = 0; i < nRandomBits; ++i){
        u16 rbit = ((u16)generator())%nBits;
        randomBits.emplace_back(rbit);
        // LOG(TestLog, BE_LOG_ERROR) << "Random Bit: " << rbit;
    }

    // Clear all
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 j = 0; j < nBits; ++j)
        {
            bitfield.unset(i, j);
        }
    }

    // Check if all zeros
    if (!checkIfAllZeros(bitfield,"[2]")){
        return false;
    }

    // Set random objects, all objects multiple of objRandom are set to a defined bit sequence
    for (u32 i = 0; i < nObjs; ++i)
    {
        if (i % objRandom == 0)
        {
            for (u32 j = 0; j < randomBits.size(); ++j)
            {
                bitfield.set(i, randomBits[j]);
            }
        }
    }

    // Verify if all objects have the expected values
    for (u32 i = 0; i < nObjs; ++i)
    {
        if (i % objRandom == 0)
        {
            for (u32 j = 0; j < randomBits.size(); ++j)
            {
                if (!bitfield.test(i, randomBits[j])){
                    LOG(TestLog, BE_LOG_ERROR) << "ERROR: object at index " << i << " should have bit " << randomBits[j] << " set!";
                    return false;
                }
            }
        }
        else
        {
            BitMask x = bitfield.getObj(i);
            if (x.b64 != 0){
                LOG(TestLog, BE_LOG_ERROR) << "ERROR: object at index " << i << " should be zero! Found: " << BitMaskStr(x);
                return false;
            }
        }
    }

    // Set all bits to 1
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 j = 0; j < nBits; ++j)
        {
            bitfield.set(i, j);
        }
    }

    // Verify if all objects have all bits set
    for (u32 i = 0; i < nObjs; ++i)
    {
        for (u16 j = 0; j < nBits; ++j)
        {
            if (!bitfield.test(i, j)){
                LOG(TestLog, BE_LOG_ERROR) << "ERROR: object at index " << i << " should have bit " << randomBits[j] << " set!";
                return false;
            }
        }


        BitMask a = bitfield.getObj(i);
        if (a.b64 != bitmask.b64){
            LOG(TestLog, BE_LOG_ERROR) << "ERROR: object at index " << i << " should have all bits equal to the bitmask " << a.b64 << " expected: " << bitmask.b64;
            return false;
        }
    }

    return true;
}

bool errorTest(u32 nBits, u32 nObjs, u32 obj)
{
    u64 seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 generator(seed);  // mt19937 is a standard mersenne_twister_engine

    ObjBitField bitfield(nBits);
    for (u32 i = 0; i < nObjs; ++i)
    {
        bitfield.push();
    }

    for (u16 i = 0; i < nBits; ++i)
    {
        bitfield.set(obj, i);
    }

    bitfield.getObj(obj);

    return true;
}

int main(int argc, char* argv[])
{
    // errorTest(59, 8, 2);
	int errors = 0;

    for (u16 i = 1; i <= 64; ++i)
    {
        if (!testBitCase(i)){
            LOG(TestLog, BE_LOG_INFO) << "Failed: testBitCase " << i;
			++errors;
        }
    }

    return errors;
}

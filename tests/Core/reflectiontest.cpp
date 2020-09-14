
#include <iostream>

#include "bitengine/Core/Reflection.h"

#include "gtest/gtest.h"

// Define some test classes
class MyClass
{
public:
        int x;
        float y;
        int* ptr;
};

class Type2
{

};

class OtherClass
{
public:
        int k;
        MyClass c;
        Type2 m;
};


// Type2 will be printed as <pointer>
DefineToStringFor(Type2, {
    std::stringstream s;
    s << "<" << value << ">";
    return s.str();
})

// Override default int ToString to format "int(value)".
// Just an example!!
DefineToStringFor(int, {
    std::stringstream s;
    if (value) {
        s << "int(" << *typedValue << ")";
    } else {
        s << "int(NULL)";
    }
    return s.str(); 
})

// Reflection code
REFLECT_START(MyClass)
	ADD_MEMBER_VARIABLE(x);
	ADD_MEMBER_VARIABLE(y);
REFLECT_END(MyClass)

REFLECT_START(OtherClass)
	ADD_MEMBER_VARIABLE(k);
	ADD_MEMBER_VARIABLE(c);
	ADD_MEMBER_VARIABLE(m);
REFLECT_END(OtherClass)

// Test
TEST(ReflectionTest, ReflectedClassNameProperlyDefined)
{
    ASSERT_EQ("MyClass", BitEngine::ClassName<MyClass>::Get());
    ASSERT_EQ("OtherClass", BitEngine::ClassName<OtherClass>::Get());
}

TEST(ReflectionTest, CustomToString)
{
    int myInt = 7;
    std::string str = BitEngine::Reflection::TypeToString<int>::ToString(&myInt);
    ASSERT_EQ(str, "int(7)"); // Was overriden above

    unsigned int myUint = 7;
    str = BitEngine::Reflection::TypeToString<unsigned int>::ToString(&myUint);
    ASSERT_EQ(str, "7");

    int* someIntPtr = (int*)~0;
    str = BitEngine::Reflection::TypeToString<int*>::ToString(&someIntPtr);
    ASSERT_TRUE(std::all_of(str.begin(), str.end(), [](char i) { return i == 'F'; })); // Independent of arch size

    Type2 type2;
    str = BitEngine::Reflection::TypeToString<Type2>::ToString(&type2);
    std::stringstream expectedType2;
    expectedType2 << "<" << &type2 << ">";
    ASSERT_EQ(str, expectedType2.str());
}

TEST(ReflectionTest, Jsonize)
{
    MyClass myClass;
    myClass.x = 5;
    myClass.y = 117.68f;
    myClass.ptr = (int*)0xff; // not reflected!!

    BitEngine::Reflection::Reflected reflected = BitEngine::Reflection::Class::FromInstance(myClass);

    std::string json = reflected.jsonize();
    ASSERT_EQ(json, "{x: int(5),y: 117.68}"); // ptr is not reflected!
}

TEST(ReflectionTest, GetDataFromReflectedInstance)
{
    MyClass myClass;
    myClass.x = 5;
    myClass.y = 117.68f;

    BitEngine::Reflection::Reflected reflected = BitEngine::Reflection::Class::FromInstance(myClass);
    ASSERT_EQ(myClass.x, reflected.get<int>("x"));
    ASSERT_EQ(myClass.y, reflected.get<float>("y"));
}

TEST(ReflectionTest, SetDataInReflectedInstance)
{
    MyClass myClass;

    BitEngine::Reflection::Reflected reflected = BitEngine::Reflection::Class::FromInstance(myClass);

    reflected.set("x", 5);
    reflected.set("y", 677.22f);
    ASSERT_EQ(5, reflected.get<int>("x"));
    ASSERT_EQ(677.22f, reflected.get<float>("y"));
    ASSERT_EQ(5, myClass.x);
    ASSERT_EQ(677.22f, myClass.y);
}

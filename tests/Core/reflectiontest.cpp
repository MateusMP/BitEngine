
#include <iostream>

#include "bitengine/Core/Reflection.h"

#include "gtest/gtest.h"

// Define some test classes
class MyClass
{
public:
        int x;
        float y;
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

// Override default int ToString to always return "INT".
// Just an example!!
DefineToStringFor(int, return "INT")

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
}

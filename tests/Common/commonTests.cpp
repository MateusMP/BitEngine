
#include "bitengine/Common/MacroHelpers.h"

#include "gtest/gtest.h"

class SomeRandomClass {

};

TEST(CommonTests, ClassNameGet)
{
    ASSERT_EQ("SomeRandomClass", BitEngine::ClassName<SomeRandomClass>::Get());
}

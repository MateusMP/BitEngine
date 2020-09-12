
#include "BitEngine/Common/MacroHelpers.h"

#include "gtest/gtest.h"

class SomeRandomClass {

public:
    class InnerClass {

    };

};

namespace Namespaced {
    class NamespaceClass {
        public:
            class InnerClass {

            };
    };
}

TEST(CommonTests, ClassNameGet)
{
    ASSERT_EQ("SomeRandomClass", BitEngine::ClassName<SomeRandomClass>::Get());
}

TEST(CommonTests, InnerClassNameGet)
{
    ASSERT_EQ("SomeRandomClass::InnerClass", BitEngine::ClassName<SomeRandomClass::InnerClass>::Get());
}

TEST(CommonTests, NamespacedClassNameGet)
{
    ASSERT_EQ("Namespaced::NamespaceClass", BitEngine::ClassName<Namespaced::NamespaceClass>::Get());
}

TEST(CommonTests, NamespacedInnerClassNameGet)
{
    ASSERT_EQ("Namespaced::NamespaceClass::InnerClass", BitEngine::ClassName<Namespaced::NamespaceClass::InnerClass>::Get());
}

TEST(CommonTests, UsingNamespaceNamespacedClassNameGet)
{
    using namespace Namespaced;
    ASSERT_EQ("Namespaced::NamespaceClass", BitEngine::ClassName<NamespaceClass>::Get());
}

TEST(CommonTests, UsingNamespaceNamespacedInnerClassNameGet)
{
    using namespace Namespaced;
    ASSERT_EQ("Namespaced::NamespaceClass::InnerClass", BitEngine::ClassName<NamespaceClass::InnerClass>::Get());
}
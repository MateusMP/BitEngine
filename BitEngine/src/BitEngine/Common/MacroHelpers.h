#pragma once

#include <string>

#define SINGLE_ARG(...) __VA_ARGS__

// Get correct preprocessor define based on compiler
#if defined(__GNUC__) || defined(__GNUG__)
    #define BE_FUNCTION_FULL_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define BE_FUNCTION_FULL_NAME __FUNCTION__
#else
    #define BE_FUNCTION_FULL_NAME __FUNCTION__
#endif // defined

namespace BitEngine
{
    namespace StaticUtils {
        template<size_t N>
        constexpr size_t strlen(const char (&)[N]) {
            return N - 1;
        }
    }

    template <class T>
    class ClassName
    {
        public:
            static const std::string& Get()
            {
                static const std::string name = innerGet();
                return name;
            }

        private:
            static std::string innerGet()
            {
                std::string name = BE_FUNCTION_FULL_NAME;

                // Some parsing to get class name
#if defined(_MSC_VER)
                size_t pos = name.find_first_of('<');
                if (pos != std::string::npos) {
                        name = name.substr(pos + 7);
                }

                pos = name.find_last_of('>');
                if (pos != std::string::npos) {
                        name = name.substr(0, pos);
                }
#elif defined(__GNUC__)
                constexpr char token[] = "with T = ";
                size_t pos = name.find(token);
                size_t endPos = name.find_first_of(';', pos);
                if (pos != std::string::npos) {
                    size_t begin = pos + StaticUtils::strlen(token);
                    name = name.substr(begin, endPos - begin);
                }
#else
    //?
#endif
                return name;
            }
    };

    template <class T>
    const std::string& GetClassNameB(void (T::*f)()) {
        return ClassName<T>::Get();
    }

    template <class T>
    const std::string& GetClassName() {
        return ClassName<T>::Get();
    }

    class MacroParser{
        public:
        static std::string GetNamespace(const char* str)
        {
            std::string name = str;
            size_t pos = name.find_last_of(':');
            if (pos != std::string::npos)
                name = name.substr(0, pos-1-11-1);
            if (name.empty())
                name = "_";
            return name;
        }
    };

    template<typename S, typename T>
    class is_streamable
    {
        template<typename SS, typename TT>
        static auto test(int)
                -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

        template<typename, typename>
        static auto test(...)->std::false_type;

        public:
        static const bool value = decltype(test<S, T>(0))::value;
    };

#define GetCurrentNamespace()			\
        MacroParser::GetNamespace(BE_FUNCTION_FULL_NAME)
}

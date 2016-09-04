#pragma once

#include <string>

#define SINGLE_ARG(...) __VA_ARGS__

namespace BitEngine
{
	template <class T>
	class ClassName
	{
		public:
		static std::string Get()
		{
			std::string name = __FUNCTION__;

			// Some parsing to get class name
			size_t pos = name.find_first_of('<');
			if (pos != std::string::npos) {
				name = name.substr(pos + 7);
			}

			pos = name.find_last_of('>');
			if (pos != std::string::npos) {
				name = name.substr(0, pos);
			}
			return name;
		}
	};

	template <class T>
	std::string GetClassName(void (T::*f)())
	{
		static const std::string name = ClassName<T>::Get();
		return name;
	}

	template <class T>
	std::string GetClassName()
	{
		static const std::string name = ClassName<T>::Get();
		return name;
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
	MacroParser::GetNamespace(__FUNCTION__)
}

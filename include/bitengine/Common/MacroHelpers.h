#pragma once

#include <string>

namespace BitEngine
{
	template <class T>
	class ClassName
	{
		public:
		static std::string Get()
		{
			std::string name = __FUNCTION__;

			// Parte to get class name
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

#define GetCurrentNamespace()			\
	MacroParser::GetNamespace(__FUNCTION__)
}

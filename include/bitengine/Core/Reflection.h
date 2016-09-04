#pragma once

#include <stddef.h>     /* offsetof */
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <type_traits>

#include <string>
#include <map>
#include <set>

#include "Common/TypeDefinition.h"
#include "Common/MacroHelpers.h"

namespace BitEngine {
namespace Reflection {

	extern u32 globalUniqueValue;

	class Reflected;
	class Class;

	typedef int TypeId;
	typedef Reflected(*ReflecterBuilder)(char*);
	typedef std::string(*StrGenFunc)(void* value);

	class MemberType
	{
		friend class Reflected;
		template<typename> friend class ClassReflection;

		public:
		MemberType(const std::string& n, TypeId t, int off)
			: name(n), type(t), offset(off)
		{}

		TypeId getType() const {
			return type;
		}
		const std::string& getName() const {
			return name;
		}

		private:
		std::string name;
		TypeId type;
		int offset;
	};

	struct ReflectionData {
		TypeId classId;
		std::string className;
		std::map<std::string, MemberType> m_members;
	};


	std::map<TypeId, ReflectionData*>& GetReflectedClasses();

	template<typename T>
	static int GetUniqueID() {
		static int value = ++globalUniqueValue;
		//printf("Type %s has id: %d\n", typeid(T).name(), value);
		return value;
	}

	static std::string ToStringForTypeId(TypeId typeId, void* value, StrGenFunc func = nullptr)
	{
		static std::map<TypeId, StrGenFunc> generators;
		if (func != nullptr)
		{
			printf("Str Generator for type %d - %p\n", typeId, func);
			generators[typeId] = func;
		}

		if (value == nullptr)
		{
			return "<NULL>";
		}
		else
		{
			auto it = generators.find(typeId);
			if (it == generators.end()) {
				throw "INVALID TYPE";
			}
			else {
				return (*it->second)(value);
			}
		}
	}

	class Reflected
	{
		friend class Class;
		template<typename> friend class ClassReflection;

		struct MemberValue {
			MemberValue(MemberType* m, void* v)
				: member(m), value(v)
			{}
			MemberType* member;
			void* value;
		};

		public:
		Reflected(char* instance, ReflectionData* reflectionData)
			: classData(reflectionData)
		{
			std::map<std::string, MemberType>& members = reflectionData->m_members;
			for (auto& it : members)
			{
				MemberType& m = it.second;
				m_members.emplace(std::piecewise_construct,
					std::forward_as_tuple(it.first),
					std::forward_as_tuple(&m, (instance + m.offset)));
			}
		}

		template<typename X>
		X& get(const std::string& memberName)
		{
			auto it = m_members.find(memberName);
			if (it != m_members.end())
			{
				if (GetUniqueID<X>() == it->second.member->getType())
				{
					void* value = it->second.value;
					return *((X*)value);
				}
				else
				{
					std::cout << "Invalid member type?" << std::endl;
					exit(-1);
				}
			}
			else
			{
				std::cout << "Invalid member name?" << std::endl;
				exit(-1);
			}
		}

		std::string getValueAsStr(const std::string& memberName)
		{
			auto it = m_members.find(memberName);
			if (it != m_members.end())
			{
				return ToStringForTypeId(it->second.member->getType(), it->second.value);
			}
			else
			{
				std::cout << "Invalid member name?" << std::endl;
				exit(-1);
			}
		}

		template<typename X>
		void set(const std::string& memberName, const X& newValue)
		{
			auto it = m_members.find(memberName);
			if (it != m_members.end())
			{
				if (GetUniqueID<X>() == it->second.member->getType())
				{
					void* value = it->second.value;
					*((X*)value) = newValue;
					return;
				}
				else
				{
					std::cout << "Invalid member type?" << std::endl;
					exit(-2);
				}
			}
			else
			{
				std::cout << "Invalid member name?" << std::endl;
				exit(-2);
			}
		}

		const std::map<std::string, MemberValue>& getMembers() const {
			return m_members;
		}
		std::map<std::string, MemberValue>& getMembers() {
			return m_members;
		}

		std::string jsonize() {
			std::stringstream ss;
			ss << "{";

			auto it = m_members.begin();
			if (it != m_members.end())
			{
				do
				{
					MemberValue& m = it->second;
					ss << it->first << ": " << ToStringForTypeId(m.member->getType(), m.value);
					++it;
					if (it != m_members.end()) {
						ss << ",";
					}
				} while (it != m_members.end());
			}

			ss << "}";
			return ss.str();
		}

		const ReflectionData* getClass() const {
			return classData;
		}

		protected:
			std::map<std::string, MemberValue> m_members;
		private:
			const ReflectionData* classData;
	};

	template<typename T>
	struct TypeToString {

		// This covers all basic types.
		template<class Q = T>
		typename std::enable_if<is_streamable<std::stringstream, Q>::value, std::string >::type
			static ToString(void* value)
		{
			std::stringstream ss;
			ss << *static_cast<T*>(value);
			return ss.str();
		}

		// Reflected classes or user type defined
		template<class Q = T>
		typename std::enable_if<!is_streamable<std::stringstream, Q>::value, std::string >::type
			static ToString(void* value)
		{
			TypeId typeId = GetUniqueID<T>();
			std::stringstream ss;
			auto it = GetReflectedClasses().find(typeId);
			if (it == GetReflectedClasses().end()) {
				ss << "\"NoToString(TypeId: " << typeId << " '" << (typeid(T).name()) << "'; Addr: " << value << ")\"";
			}
			else {
				Reflected r((char*)value, it->second);
				ss << r.jsonize();
			}
			return ss.str();
		}
	};

	template<typename T>
	bool static TypeDefine()
	{
		static bool firstTime = true;
		if (firstTime)
		{
			ToStringForTypeId(GetUniqueID<T>(), nullptr, &Reflection::TypeToString<T>::ToString);
			firstTime = false;
		}
		return firstTime;
	}
	
	template<typename T>
	class ClassReflection
	{
		static_assert(std::is_pod<T>::value, "Class should be POD.");
		using CRC__ = T;
		friend class Class;

		public:
		ClassReflection();
		int Define(); // Use REFLECT_START and REFLECT_END macros.

		void Mark() {
			reflectionData.classId = GetUniqueID<T>();
			reflectionData.className = ClassName<T>::Get();
			GetReflectedClasses().emplace(reflectionData.classId, &reflectionData);
			printf("Marked: %d: %s.\n", reflectionData.classId, reflectionData.className.c_str());
		}

		protected:
		static void addMemberVariable(const std::string& name, TypeId typeId, int offset, bool ignored)
		{
			// std::cout << "Registering member variable: " << name << std::endl;
			CR.reflectionData.m_members.emplace(std::piecewise_construct,
				std::forward_as_tuple(name),
				std::forward_as_tuple(name, typeId, offset));
		}

		static MemberType* getMemberType(const std::string& name)
		{
			auto it = CR.reflectionData.m_members.find(name);
			if (it != CR.reflectionData.m_members.end()) {
				return &it->second;
			}
			return nullptr;
		}

		static Reflected buildReflectedInstance(char* instance)
		{
			printf("Reflecting instance %p\n", instance);
			return Reflected(instance, &CR.reflectionData);
		}

		protected:
		ReflectionData reflectionData;
		static ClassReflection CR;
	};

	class Class
	{
		public:
		template<typename T>
		static Reflected FromInstance(T &instance)
		{
			return ClassReflection<T>::buildReflectedInstance((char*)&instance);
		}
	};
}} // namespaces

#define ADD_MEMBER_VARIABLE(a) \
		addMemberVariable(#a, GetUniqueID<decltype(CRC__::a)>(), offsetof(CRC__, a), TypeDefine<decltype(CRC__::a)>());
#define ADD_MEMBER_VARIABLE_PTR(a) \
		addMemberVariable(#a, EMT_PTR, offsetof(CRC__, a), TypeDefine<decltype(CRC__::a)>());

#define REFLECT_START(className)											\
namespace BitEngine { namespace Reflection { 								\
ClassReflection<className> ClassReflection<className>::CR;					\
template<>																	\
ClassReflection<className>::ClassReflection(){								\
Mark(); Define();															\
}																			\
template<>																	\
int ClassReflection<className>::Define(){

#define REFLECT_END() return 1;}}}

#define DefineToStringFor(Type, Func)					\
namespace BitEngine { namespace Reflection{ template<>  \
	struct TypeToString<Type>							\
	{													\
		static std::string ToString(void* value)		\
		{												\
			Func;         								\
		};												\
	};}}

/*
Example Usage:

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


DefineToStringFor(Type2, {
	std::stringstream s;
	s << "<" << value << ">";
	return s.str();
});

DefineToStringFor(int, return "INT");

REFLECT_START(MyClass)
ADD_MEMBER_VARIABLE(x);
ADD_MEMBER_VARIABLE(y);
REFLECT_END()

REFLECT_START(OtherClass)
ADD_MEMBER_VARIABLE(k);
ADD_MEMBER_VARIABLE(c);
ADD_MEMBER_VARIABLE(m);
REFLECT_END()

*/
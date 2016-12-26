#include "Core/Reflection.h"

namespace BitEngine {
namespace Reflection{

	u32 globalUniqueValue = 0;

	std::string ToStringForTypeId(TypeId typeId, void* value, StrGenFunc func)
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

	std::map<TypeId, ReflectionData*>& GetReflectedClasses()
	{
		static std::map<TypeId, ReflectionData*> reflected;
		return reflected;
	}

	//
	// Reflected
	//

	Reflected::Reflected(char* instance, ReflectionData* reflectionData)
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

	bool Reflected::hasMember(const std::string& memberName) const
	{
		auto it = m_members.find(memberName);
		return (it != m_members.end());
	}

	std::string Reflected::getValueAsStr(const std::string& memberName) const
	{
		auto it = m_members.find(memberName);
		if (it != m_members.end())
		{
			return ToStringForTypeId(it->second.member->getType(), it->second.value);
		}
		else
		{
			throw std::runtime_error("No member found for given name");
		}
	}

	std::string Reflected::jsonize() const {
		std::stringstream ss;
		ss << "{";

		auto it = m_members.begin();
		if (it != m_members.end())
		{
			do
			{
				const MemberValue& m = it->second;
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
}}
#include "BitEngine/Core/Reflection.h"

namespace BitEngine {
namespace Reflection {

    u32 GetNextUniqueId() {
        static u32 uniqueId = 0;
        return ++uniqueId;
    }

    std::string ToStringForTypeId(TypeId typeId, void* value, StrGenFunc func)
    {
        static std::map<TypeId, StrGenFunc> generators;
        if (func != nullptr) {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Str Generator for type " << typeId << " func: " << func;
            generators[typeId] = func;
            return func(value); // test nullptr should not break!
        }
        else {
            const auto it = generators.find(typeId);
            if (it == generators.end()) {
                throw std::invalid_argument("INVALID TYPE: " + typeId);
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

    Reflected::Reflected(void* instance, ReflectionData* reflectionData)
        : classData(reflectionData)
    {
        const std::map<std::string, MemberType>& members = reflectionData->members;
        for (const auto& it : members) {
            const MemberType& m = it.second;
            m_members.emplace(std::piecewise_construct,
                std::forward_as_tuple(it.first),
                std::forward_as_tuple(&m, (((char*)instance) + m.offset)));
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
        if (it != m_members.end()) {
            return ToStringForTypeId(it->second.member->getType(), it->second.value);
        }
        else {
            std::stringstream str;
            str << "No member found for given name: " << memberName;
            throw std::runtime_error(str.str());
        }
    }

    std::string Reflected::jsonize() const
    {
        std::stringstream ss;
        ss << "{";

        auto it = m_members.begin();
        if (it != m_members.end()) {
            do {
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
}
}

#pragma once

#include <stddef.h> /* offsetof */
#include <iostream>
#include <sstream>
#include <type_traits>

#include <string>
#include <map>
#include <set>
#include <any>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Common/MacroHelpers.h"
#include "BitEngine/Core/Logger.h"

namespace BitEngine {
namespace Reflection {

    u32 GetNextUniqueId();

    class Reflected;
    class Class;

    typedef int TypeId;
    typedef Reflected (*ReflecterBuilder)(char*);
    typedef std::string (*StrGenFunc)(void* value);
    typedef std::set<int> Attributes;
    typedef std::map<std::string, std::any> Metadata;

    /**
	* MemberType
	* @brief Holds information related to a class field
	*/
    class MemberType {
        friend class Reflected;
        template <typename>
        friend class ClassReflection;

    public:
        MemberType(const std::string& n, TypeId t, int off)
            : name(n)
            , type(t)
            , offset(off)
        {
        }

        TypeId getType() const
        {
            return type;
        }
        const std::string& getName() const
        {
            return name;
        }

        const std::set<int>& getAttributes() const
        {
            return attributes;
        }

        bool hasAttribute(int attr) const
        {
            return attributes.find(attr) != attributes.end();
        }

        const Metadata& getMetadata() const
        {
            return metadata;
        }

        MemberType& WithAttributes(const std::set<int>& attrs)
        {
            attributes = attrs;
            return *this;
        }

        MemberType& WithMetadata(const Metadata& attrs)
        {
            metadata = attrs;
            return *this;
        }

    private:
        std::string name;
        std::set<int> attributes;
        Metadata metadata;
        TypeId type;
        int offset;
    };

    struct ReflectionData {
        TypeId classId;
        std::string className;
        std::map<std::string, MemberType> members;
    };

    std::map<TypeId, ReflectionData*>& GetReflectedClasses();
    std::string ToStringForTypeId(TypeId typeId, void* value, StrGenFunc func = nullptr);

    template <typename T>
    static int GetUniqueID()
    {
        static int value = GetNextUniqueId();
        return value;
    }

    class Reflected {
    private:
        friend class Class;
        template <typename>
        friend class ClassReflection;

        struct MemberValue {
            MemberValue(const MemberType* m, void* v)
                : member(m)
                , value(v)
            {
            }
            const MemberType* member;
            void* value;
        };

    public:
        Reflected(void* instance, ReflectionData* reflectionData);

        bool hasMember(const std::string& memberName) const;
        std::string getValueAsStr(const std::string& memberName) const;
        std::string jsonize() const;

        template <typename X>
        X& get(const char memberName[]) const
        {
            return get<X>(std::string(memberName));
        }

        template <typename X>
        X& get(const std::string& memberName) const
        {
            auto it = m_members.find(memberName);
            if (it != m_members.end()) {
                const TypeId xType = GetUniqueID<X>();
                if (xType == it->second.member->getType()) {
                    void* value = it->second.value;
                    return *((X*)value);
                }
                else {
                    std::stringstream str;
                    str << "Invalid <get> member type " << xType << ", correct type: " << it->second.member->getType();
                    throw std::invalid_argument(str.str());
                }
            }
            else {
                std::stringstream str;
                str << "No <get> member found for given name" << memberName;
                throw std::invalid_argument(str.str());
            }
        }

        template <typename X>
        void set(const std::string& memberName, const X& newValue)
        {
            auto it = m_members.find(memberName);
            if (it != m_members.end()) {
                const TypeId xType = GetUniqueID<X>();
                if (xType == it->second.member->getType()) {
                    void* value = it->second.value;
                    *((X*)value) = newValue;
                    return;
                }
                else {
                    std::stringstream str;
                    str << "Invalid <set> member type " << xType << ", correct type: " << it->second.member->getType();
                    throw std::invalid_argument(str.str());
                }
            }
            else {
                std::stringstream str;
                str << "No <set> member  found for given name: " << memberName;
                throw std::invalid_argument(str.str());
            }
        }

        const std::map<std::string, MemberValue>& getMembers() const
        {
            return m_members;
        }
        std::map<std::string, MemberValue>& getMembers()
        {
            return m_members;
        }
        const ReflectionData* getClass() const
        {
            return classData;
        }

    protected:
        std::map<std::string, MemberValue> m_members;

    private:
        const ReflectionData* classData;
    };

    template <typename T>
    struct TypeToString {
        // This covers all basic types.
        template <class Q = T>
        typename std::enable_if<is_streamable<std::stringstream, Q>::value, std::string>::type static ToString(void* value)
        {
            std::stringstream ss;
            if (value != nullptr) {
                ss << *static_cast<T*>(value);
            }
            else {
                ss << "(nullptr)";
            }
            return ss.str();
        }

        // Reflected classes or user type defined
        template <class Q = T>
        typename std::enable_if<!is_streamable<std::stringstream, Q>::value, std::string>::type static ToString(void* value)
        {
            TypeId typeId = GetUniqueID<T>();
            std::stringstream ss;
            auto it = GetReflectedClasses().find(typeId);
            if (it == GetReflectedClasses().end()) {
                ss << "\"NoToString(TypeId: " << typeId << " '" << (typeid(T).name()) << "'; Addr: " << value << ")\"";
            }
            else {
                if (value) {
                    Reflected r((char*)value, it->second);
                    ss << r.jsonize();
                }
                else {
                    ss << "0";
                }
            }
            return ss.str();
        }
    };

    template <typename T>
    bool static TypeDefine()
    {
        static bool initialized = false;
        if (!initialized) {
            // Save reflection function
            ToStringForTypeId(GetUniqueID<T>(), nullptr, &Reflection::TypeToString<T>::ToString);
            initialized = true;
        }
        return initialized;
    }

    template <typename T>
    class ClassReflection {
        static_assert(std::is_pod<T>::value, "Class should be POD.");
        using CRC__ = T;
        friend class Class;

    public:
        ClassReflection();

    private:
        int Define(); // Use REFLECT_START and REFLECT_END macros.

        void Mark()
        {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Marking class...";
            reflectionData.classId = GetUniqueID<T>();
            reflectionData.className = BitEngine::GetClassName<T>();
            GetReflectedClasses().emplace(reflectionData.classId, &reflectionData);
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Marked: Class id " << reflectionData.classId << ": " << reflectionData.className;
        }

        MemberType& addMemberVariable(const std::string& name, TypeId typeId, int offset)
        {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Registering member variable: " << name << " Type: " << typeId << " Offset " << offset << std::endl;
            auto it = reflectionData.members.emplace(name, MemberType(name, typeId, offset));
            return it.first->second;
        }

        const MemberType* getMemberType(const std::string& name)
        {
            auto it = reflectionData.members.find(name);
            if (it != reflectionData.members.end()) {
                return &it->second;
            }
            return nullptr;
        }

    protected:
        static Reflected buildReflectedInstance(void* instance)
        {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Reflecting instance " << (instance);
            return Reflected(instance, &CR().reflectionData);
        }

        static ClassReflection<T>& CR();

    private:
        friend class Class;
        ReflectionData reflectionData;
    };

    class Class {
    public:
        template <typename T>
        static Reflected FromInstance(T& instance)
        {
            return ClassReflection<T>::buildReflectedInstance((char*)&instance);
        }

        template <typename T>
        static const ReflectionData& Info()
        {
            return ClassReflection<T>::CR().reflectionData;
        }
    };
}
} // namespaces

#define ADD_MEMBER_FIELD(a)           \
    TypeDefine<decltype(CRC__::a)>(); \
    addMemberVariable(#a, GetUniqueID<decltype(CRC__::a)>(), offsetof(CRC__, a))

#define REFLECTION_NAMESPACE_BEGIN \
    namespace BitEngine {          \
        namespace Reflection {

#define REFLECTION_NAMESPACE_END \
    }                            \
    }

#define REFLECTION_DEFINITION_BEGIN(className) \
    template <>                                \
    int ClassReflection<className>::Define()   \
    {

#define REFLECTION_DEFINITION_END \
    return 1;                     \
    }

#define REFLECTION_CLASS_REFLECTION_CONSTRUCTOR(className) \
    template <>                                            \
    ClassReflection<className>::ClassReflection()          \
    {                                                      \
        Mark();                                            \
        Define();                                          \
    }

#define REFLECTION_STATIC_CR_INSTANCE(className)                 \
    template <>                                                  \
    ClassReflection<className>& ClassReflection<className>::CR() \
    {                                                            \
        static ClassReflection<className> instance;              \
        return instance;                                         \
    }

#define REGISTER_CLASS_NAME(className)                                                   \
    namespace BitEngine {                                                                \
        template <>                                                                      \
        std::string ClassName<className>::innerGet() { return std::string(#className); } \
    }

#define REFLECT_START(className)   \
    REGISTER_CLASS_NAME(className) \
    REFLECTION_NAMESPACE_BEGIN     \
    REFLECTION_DEFINITION_BEGIN(className)

#define REFLECT_END(className)                         \
    REFLECTION_DEFINITION_END                          \
    REFLECTION_CLASS_REFLECTION_CONSTRUCTOR(className) \
    REFLECTION_STATIC_CR_INSTANCE(className)           \
    REFLECTION_NAMESPACE_END

/*!
 * \macro DefineToStringFor
 * Provide a definition of ToString method for a given type.
 * Usually used if the type is not a simple type like int/float/string.
 * \param Type First parameter is the Type (eg: MyClass).
 * \param Func is the code to generate the std::string to be returned.
 *        this Func receives a parameter named "value" which represents the pointer (void*)
 *        to the field which is being serialized. Also, there is a "typedValue" variable
 *        which represents the Type*.
 */
#define DefineToStringFor(Type, Func)                                   \
    namespace BitEngine {                                               \
        namespace Reflection {                                          \
            template <>                                                 \
            struct TypeToString<Type> {                                 \
                static std::string ToString(void* value)                \
                {                                                       \
                    const Type* typedValue = static_cast<Type*>(value); \
                    Func;                                               \
                };                                                      \
            };                                                          \
        }                                                               \
    }

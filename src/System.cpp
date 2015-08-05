

#include "System.h"

namespace BitEngine{


System::System(const std::string& name)
	: m_name(name), configuration(name)
{
}

System::~System(){
}

const std::string& System::getName() const
{
	return m_name;
}

}

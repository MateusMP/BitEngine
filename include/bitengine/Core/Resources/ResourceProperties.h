#pragma once

#include <string>
#include <memory>

namespace BitEngine {

	class ResourceManager;

	struct ResourceProperty;
	struct ResourcePropertyContainer;

	struct ResourceProperty
	{
		virtual ~ResourceProperty() {}
		virtual ResourcePropertyContainer getProperty(const std::string& str) = 0;
		virtual ResourcePropertyContainer getProperty(int index) = 0;
		virtual const std::string& getValueString() const = 0;
		virtual double getValueDouble() const = 0;
		virtual int getValueInt() const = 0;
		virtual int getNumberOfProperties() const = 0;
		//virtual int getKeys(std::vector<std::string>& keys) = 0;

		virtual ResourcePropertyContainer operator[](const std::string& str) = 0; // { return this->getProperty(str); }
	};

	struct ResourcePropertyContainer
	{
		ResourcePropertyContainer()
		{}
		ResourcePropertyContainer(ResourceProperty* p)
			: prop(p)
		{}


		ResourcePropertyContainer operator[](const std::string& str) {
			return prop->getProperty(str);
		}
		const ResourcePropertyContainer operator[](const std::string& str) const {
			return prop->getProperty(str);
		}
		ResourcePropertyContainer operator[](int index) {
			return prop->getProperty(index);
		}
		const ResourcePropertyContainer operator[](int index) const {
			return prop->getProperty(index);
		}
		std::string getValueString() const { return prop->getValueString(); }
		double getValueDouble() const { return prop->getValueDouble(); }
		int getValueInt() const { return prop->getValueInt(); }
		int getNumberOfProperties() const { return prop->getNumberOfProperties(); }

		bool isValid() { return prop.get() != nullptr; }

		private:
		std::shared_ptr<ResourceProperty> prop;
	};
}
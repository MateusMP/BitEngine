#pragma once

#include <vector>

#include "TypeDefinition.h"
typedef uint32 ComponentHandle;

template <typename T>
class ComponentCollection
{
	public:
		ComponentCollection(){
			components.emplace_back(); // element 0 is invalid
		}

		ComponentHandle newComponent(){
			if (freeHandles.empty()){
				ComponentHandle hdl = components.size();
				components.emplace_back();

				validComponents.push_back(&components[hdl]);
				return hdl;
			}
			else {
				ComponentHandle hdl = freeHandles.back();
				components.emplace(components.begin() + hdl);
				freeHandles.pop_back();

				validComponents.push_back(&components[hdl]);
				return hdl;
			}
		}

		void removeComponent(ComponentHandle hdl){
			freeHandles.push_back(hdl);
			auto it = std::find(validComponents.begin(), validComponents.end(), &components[hdl]);

			if (validComponents.size() > 1){
				*it = validComponents.back();
				validComponents.pop_back();
			}
			else {
				validComponents.clear();
			}
		}

		std::vector<T*>& getValidComponents(){
			return validComponents;
		}

		T* getComponent(ComponentHandle hdl){
			return &components[hdl];
		}

	private:
		std::vector<T> components;
		std::vector<T*> validComponents;
		std::vector<ComponentHandle> freeHandles;
};
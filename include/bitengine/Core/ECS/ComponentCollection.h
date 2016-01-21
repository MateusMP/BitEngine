#pragma once

#include <vector>

#include "Common/TypeDefinition.h"

namespace BitEngine{

	template <typename T>
	class ComponentCollection
	{
	public:
		ComponentCollection() :validRef(true){
			components.emplace_back(); // element 0 is invalid
		}

		template<typename... Args>
		ComponentHandle newComponent(Args ... args){
			if (freeHandles.empty())
			{
				ComponentHandle hdl = components.size();
				if (validRef){ // Needs to guarantee if the final vector address did not change
					T* _reallocTest = &components[0];
					components.emplace_back(args...);
					T* _reallocTestEnd = &components[0];
				
					validComponents.push_back(hdl);
					if (_reallocTest != _reallocTestEnd){ // Vector realloc! Invalid references!
						validRef = false;
					} else { // Add reference if the vector continues at the same address
						validComponentsRef.push_back(&components[hdl]);
					}
				} else { // References are invalid, so just ignore them for now
					components.emplace_back(args...);
					validComponents.push_back(hdl);
				}

				return hdl;
			}
			else 
			{
				ComponentHandle hdl = freeHandles.back();
				components.emplace(components.begin() + hdl);
				freeHandles.pop_back();

				validComponents.push_back(hdl);
				validComponentsRef.push_back(&components[hdl]);
				return hdl;
			}
		}

		void removeComponent(ComponentHandle hdl){
			unsigned int index;
			for (index = 0; index < validComponents.size(); ++index){
				if (validComponents[index] == hdl){
					break;
				}
			}

			// Not found? Should not happen.
			if (index == validComponents.size())
				return;

			freeHandles.push_back(hdl);

			// Put something on the cleaned space to mantain a sequential order
			if (index < validComponents.size() - 1)
			{
				// NOTE: THE REAL COMPONENT DATA DO NOT CHANGE POSITION
				// Last valid index now ocuppy the freed index
				ComponentHandle substBy = validComponents.back();

				// components[hdl] = components[substBy];
				validComponents[index] = substBy;
				validComponentsRef[index] = &components[substBy];

				validComponents.pop_back();
				validComponentsRef.pop_back();
			}
		}

		const std::vector<ComponentHandle>& getValidComponents() const{
			return validComponents;
		}

		std::vector<T*>& getValidComponentsRef(){
			if (!validRef)
			{
				validComponentsRef.clear();
				for (ComponentHandle h : validComponents){
					validComponentsRef.emplace_back( &components[h] );
				}
				validRef = true;
			}

			return validComponentsRef;
		}

		T* getComponent(ComponentHandle hdl){
			return &components[hdl];
		}

		const T* getComponent(ComponentHandle hdl) const {
			return &components[hdl];
		}

	private:
		std::vector<T> components;
		std::vector<T*> validComponentsRef;
		std::vector<ComponentHandle> validComponents;
		std::vector<ComponentHandle> freeHandles;

		bool validRef;
	};

}
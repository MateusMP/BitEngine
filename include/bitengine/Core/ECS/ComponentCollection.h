#pragma once

#include <vector>

#include "Common/TypeDefinition.h"

namespace BitEngine{

	template <typename T>
	class ComponentCollection
	{
	public:
		ComponentCollection()
            : validRef(false)
		{
			components.emplace_back(); // element 0 is invalid
		}

		template<typename... Args>
		ComponentHandle newComponent(Args ... args)
		{
			if (freeHandles.empty())
			{
				ComponentHandle hdl = components.size();

				// Needs to guarantee if the final vector address did not change
				if (validRef)
                {
					const T* _reallocTest = &components[0];
					components.emplace_back(args...);
					const T* _reallocTestEnd = &components[0];

					// Vector realloc! Invalid references!
					if (_reallocTest != _reallocTestEnd)
                    {
						validRef = false;
					}
					else // Add reference if the vector continues at the same address
					{
                        validComponents.push_back(hdl);
						validComponentsRef.push_back(&components[hdl]);
					}
				} else { // References are invalid, so just ignore them for now
					components.emplace_back(args...);
				}

				return hdl;
			}
			else
			{
				ComponentHandle hdl = freeHandles.back();
				components.emplace(components.begin() + hdl);
				freeHandles.pop_back();

                validRef = false;

				return hdl;
			}
		}

		void removeComponent(ComponentHandle hdl)
		{
			freeHandles.push_back(hdl);
			components[hdl].entity = 0;

            validRef = false;
		}

		inline const std::vector<ComponentHandle>& getValidComponents() const {
		    orderLists();
			return validComponents;
		}

		inline const std::vector<T*>& getValidComponentsRef() const {
            orderLists();
			return validComponentsRef;
		}

		inline T* getComponent(ComponentHandle hdl){
			return &components[hdl];
		}

		inline const T* getComponent(ComponentHandle hdl) const {
			return &components[hdl];
		}

	private:
	    void orderLists() const
	    {
            if (!validRef)
			{
			    // Rebuild the validComponents and validComponentsRef lists in order
			    uint32 totalComponents = components.size() - freeHandles.size() - 1;
			    validComponentsRef.resize(totalComponents);
			    validComponents.resize(totalComponents);

			    // First component is always invalid
				for (uint32 i = 1; totalComponents != 0; ++i)
                {
                    if (components[i].entity != 0)
                    {
                        const uint32 id1 = i-1;
                        validComponents[id1] = i;
                        validComponentsRef[id1] = const_cast<T*>(&components[i]);
                        --totalComponents;
                    }
				}
				validRef = true;
			}
	    }

		std::vector<T> components;
		mutable std::vector<T*> validComponentsRef; // always in memory order
		mutable std::vector<ComponentHandle> validComponents;

		std::vector<ComponentHandle> freeHandles;

		mutable bool validRef;
	};

}

#pragma once

#include <array>
#include <unordered_map>
#include <set>

namespace BitEngine
{
	template<typename ResourceType, int Maximum>
	class ResourceIndexer
	{
		public:
			ResourceIndexer()
				: m_currentIndex(0)
			{}

			~ResourceIndexer()
			{}

			std::array<ResourceType, Maximum>& getResources() {
				return resources;
			}

			ResourceType& getResourceAt(uint32 index) {
				return resources[index];
			}

			ResourceType* getResourceAddress(uint32 index) {
				return &resources[index];
			}

			ResourceType* findResource(const std::string& name) {
				auto it = m_byName.find(name);
				if (it == m_byName.end()) {
					return nullptr;
				}

				return &(resources[it->second]);
			}

			uint16 addResource(const std::string& name) {
				uint16 id = getNextIndex();
				m_byName.emplace(name, id);
				return id;
			}

		private:
			uint16 getNextIndex()
			{
				uint16 id;

				if (m_freeIndices.empty())
				{
					id = ++m_currentIndex;
				}
				else
				{
					id = m_freeIndices.back();
					m_freeIndices.pop_back();
				}

				return id;
			}

			uint16 m_currentIndex;
			std::array<ResourceType, Maximum> resources;
			std::vector<uint16> m_freeIndices;
			std::unordered_map<std::string, uint16> m_byName;

	};

}
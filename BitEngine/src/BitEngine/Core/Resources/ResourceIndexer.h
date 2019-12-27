#pragma once

#include <array>
#include <unordered_map>
#include <set>

namespace BitEngine
{
	class ResourceMeta;

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

			ResourceType& getResourceAt(u32 index) {
				return resources[index];
			}

			ResourceType* getResourceAddress(u32 index) {
				return &resources[index];
			}

			ResourceType* findResource(const ResourceMeta* meta) {
				const auto& it = m_byMeta.find(meta);
				if (it == m_byMeta.end()) {
					return nullptr;
				}

				return &(resources[it->second]);
			}

			u16 addResource(const ResourceMeta* meta) {
				u16 id = getNextIndex();
				m_byMeta.emplace(meta, id);
				return id;
			}

		private:
			u16 getNextIndex()
			{
				u16 id;

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

			u16 m_currentIndex;
			std::array<ResourceType, Maximum> resources;
			std::vector<u16> m_freeIndices;
			std::unordered_map<const ResourceMeta*, u16> m_byMeta;

	};

}
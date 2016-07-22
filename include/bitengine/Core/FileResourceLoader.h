#pragma once

#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <fstream>

#include "Core/Resources/ResourceManager.h"
#include "Core/Logger.h"

namespace BitEngine
{
	/// Load resources from file path
	/// This class load files from the file system
	class FileResourceLoader : public ResourceLoader
	{
		public:
			FileResourceLoader()
				: m_globalResourceID(0), stop(false)
			{
				m_thLoader = std::thread(&FileResourceLoader::work, this);
			}

			~FileResourceLoader()
			{
			}

			void shutdown() override
			{
				stop = true;
				m_requests.push("", nullptr, nullptr, 0); // stop processing request
				
				m_thLoader.join();
			}

			// Release all preloaded resource memory
			void releaseAll() override
			{
				for (auto &it : m_resources) {
					//it.second->clearBaseData();
				}
			}

			// Release all preloaded memory for this resource
			void releaseResource(uint32 id) override
			{
				//m_resources[id]->clearBaseData();
			}

			// Returns the request ID
			// Queue the request to be loaded by another thread
/*			uint32 loadRequest(const std::string& path, BaseResource* into, ResourceManager* callback) override
			{
				uint32 rid = ++m_globalResourceID;

				// Create the resource data reference
				m_resources[rid] = into; 

				// Queue the request
				m_requests.push(path, into, callback, rid);

				return rid;
			}*/

			bool reloadResource(uint32 resourceID, ResourceManager* callback)
			{
				auto it = m_resources.find(resourceID);
				if (it != m_resources.end()) 
				{
					//m_requests.push(it->second->getPath(), it->second, callback, resourceID);

					return true;
				}

				return false;
			}

			void waitForAll() override
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				while (!m_requests.empty()) {
					m_completed.wait(lock);
				}
			}

		private:
			void work()
			{
				std::vector<char> buffer;

				while (!stop)
				{
					Request request = m_requests.pop();
					if (request.into == nullptr && request.callback == nullptr) {
						m_completed.notify_all();
						continue;
					}

					// Process request
					std::ifstream file(request.file, std::ios_base::in | std::ios_base::binary);
					file.seekg(0, std::ios::end);
					uint32 size = (uint32)file.tellg();
					buffer.resize(size);
					file.seekg(0, std::ios::beg);
					file.read(buffer.data(), size);
					file.close();

					// Set data on the resource
					BaseResource* d = m_resources[request.resourceID];
					//d->set(request.resourceID, request.file, buffer);

					// Callback now that resource is loaded.
					//request.callback->onResourceLoaded(d->getResourceId());
					
					if (m_requests.empty())
					{
						m_completed.notify_all();
					}

					LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Resource " << d->getResourceId() << " loaded.";
				}

				LOG(BitEngine::EngineLog, BE_LOG_INFO) << "File Load Thread finished!";
			}

			struct Request 
			{
				Request(const std::string& f, BaseResource* i, ResourceManager* c, uint32 id)
				: file(f), into(i), callback(c), resourceID(id) {}

				std::string file;
				BaseResource* into;
				ResourceManager* callback;
				uint32 resourceID;
			};

			std::atomic<uint32> m_globalResourceID;
			std::thread m_thLoader;
			ThreadSafeQueue<Request> m_requests;
			std::condition_variable m_completed;
			std::mutex m_mutex;
			std::unordered_map<uint32, BaseResource*> m_resources;
			bool stop;
			bool onLoading;
	};
}
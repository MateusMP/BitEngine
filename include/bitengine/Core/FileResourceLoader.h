#pragma once

#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <fstream>

#include "Core/IResourceManager.h"
#include "Core/Logger.h"

namespace BitEngine
{
	class FileResourceLoader : public IResourceLoader
	{
		public:
			FileResourceLoader()
				: m_globalResourceID(0), stop(false)
			{
				m_thLoader = std::thread(&FileResourceLoader::work, this);
			}

			void releaseAll(uint32 id) override
			{

			}

			// Returns the request ID
			// Queue the request to be loaded by another thread
			uint32 loadRequest(const std::string& path, BaseResource* into, IResourceManager* callback) override
			{
				uint32 rid = ++m_globalResourceID;

				m_resources[rid] = into; // create the resource data

				m_requests.push(path, into, callback, rid);

				return rid;
			}

			bool reloadResource(uint32 resourceID, IResourceManager* callback)
			{
				auto it = m_resources.find(resourceID);
				if (it != m_resources.end()) 
				{
					m_requests.push(it->second->path, it->second, callback, resourceID);

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
				char* buffer;
				while (!stop)
				{
					Request request = m_requests.pop();

					// Process request
					std::ifstream file(request.file, std::ios_base::in | std::ios_base::binary);
					file.seekg(0, std::ios::end);
					uint32 size = (uint32)file.tellg();
					buffer = new char[size];
					file.seekg(0, std::ios::beg);
					file.read(buffer, size);
					file.close();

					BaseResource* d = m_resources[request.resourceID];
					d->setBaseData(buffer, size);
					d->path = request.file;
					d->resourceId = request.resourceID;
					request.callback->onResourceLoaded(d->resourceId);
					

					if (m_requests.empty())
					{
						m_completed.notify_all();
					}

					LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Resource " << d->resourceId << " loaded";
				}
			}

			struct Request 
			{
				Request(const std::string& f, BaseResource* i, IResourceManager* c, uint32 id)
				: file(f), into(i), callback(c), resourceID(id) {}

				std::string file;
				BaseResource* into;
				IResourceManager* callback;
				uint32 resourceID;
			};

			std::atomic<uint32> m_globalResourceID;
			std::thread m_thLoader;
			ThreadSafeQueue<Request> m_requests;
			std::condition_variable m_completed;
			std::mutex m_mutex;
			std::unordered_map<uint32, BaseResource*> m_resources;
			bool stop;
	};
}
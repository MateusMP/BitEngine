#pragma once

#include "Core/Resources/ResourceManager.h"
#include "Core/Graphics/ITexture.h"

namespace BitEngine {

	enum DataType 
	{
		INVALID_DATA_TYPE,
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		LONG,
		FLOAT,

		// Vertex only
		VEC2,
		VEC3,
		VEC4,
		MAT3,
		MAT4,
	};

	enum DataUseMode : uint32 {
		Vertex,
		Uniform,

		TotalModes
	};


	DataUseMode useModeFromString(const std::string& str);
	DataType dataTypeFromString(const std::string& str);


	enum GraphicAdapterType {
		NULL_GRAPHIC,
		SOFTWARE,
		GL2,
		GL3,
		GL4,

		VULKAN,

		DX9,
		DX11,
		DX12,

		LAST_RESERVED,
	};

	class ShaderDataDefinition
	{
		public:
		friend class Shader;

		struct DefinitionReference {
			struct Hasher {
				bool operator() (const DefinitionReference& lhs, const DefinitionReference& rhs) const {
					return (*this)(lhs) < (*this)(rhs);
				}
				size_t operator()(const ShaderDataDefinition::DefinitionReference& t) const {
					//calculate hash here.
					return t.mode >> 16 | t.container >> 8 | t.index;
				}
			};

			DefinitionReference(){}
			DefinitionReference(DataUseMode m, uint32 at, uint32 id)
			: mode(m), container(at), index(id)
			{}
			bool operator==(const DefinitionReference& o) const {
				return mode == o.mode && container == o.container && index == o.index;
			}

			DataUseMode mode;
			uint32 container;
			uint32 index;
		};

		struct DefinitionData {
			DefinitionData(const std::string& n, DataType dt, int s)
				: name(n), type(dt), size(s)
			{}
			std::string name;
			DataType type;
			int size; // number of times the type appears. Ex: vec2[2] -> type VEC2 size=2
		};

		struct DefinitionContainer
		{
			friend class ShaderDataDefinition;

			DefinitionContainer(uint32 id, DataUseMode m, int inst)
				: container(id), mode(m), instanced(inst)
			{}

			DefinitionContainer& addDataDef(const std::string& name, DataType type, int size) {
				definitionData.emplace_back(name, type, size);
				return *this;
			}

			//protected:
				std::vector<DefinitionData> definitionData;
				uint32 container;
				DataUseMode mode;
				int instanced;
		};

		const std::vector<DefinitionContainer>& getContainers(DataUseMode mode) const {
			return m_containers[mode];
		}

		DefinitionContainer& addContainer(DataUseMode mode = Vertex, int instanced = 0) {
			const int ct = m_containers[mode].size();
			m_containers[mode].emplace_back(ct, mode, instanced);
			return m_containers[mode].back();
		}

		DefinitionReference findReference(const std::string& name) const
		{
			for (uint32 um = 0; um < TotalModes; ++um)
			{
				const std::vector<DefinitionContainer>& v = m_containers[um];
				for (const DefinitionContainer& dc : v)
				{
					for (uint32 i = 0; i <  dc.definitionData.size(); ++i)
					{
						const DefinitionData& d = dc.definitionData[i];
						// compare with offset of 2 characters. We ignore the shader code prefix.
						if (name.compare(2, std::string::npos, d.name) == 0)
						{
							return DefinitionReference((DataUseMode)um, dc.container, i);
						}
					}
				}
			}

			return DefinitionReference(DataUseMode::TotalModes, 0, 0); // invalid
		}

		DefinitionData& getData(const DefinitionReference& ref)
		{
			return m_containers[ref.mode][ref.container].definitionData[ref.index];
		}

		// Returns true if a reference is valid
		bool checkRef(const DefinitionReference& r) const {
			if (r.mode < DataUseMode::TotalModes)
			{
				if (r.container < m_containers[r.mode].size())
				{
					return r.index < m_containers[r.mode][r.container].definitionData.size();
				}
			}
			return false;
		}

		DefinitionReference getReferenceToContainer(DataUseMode mode, uint32 container) const {
			return DefinitionReference(mode, container, 0);
		}

		private:
			std::vector<DefinitionContainer> m_containers[DataUseMode::TotalModes];
	};

	// BATCH
	class IBatchSector
	{
		public:
			template<typename T>
			T* getConfigValueAs(const ShaderDataDefinition::DefinitionReference& ref) {
				return (T*)getConfigValue(ref);
			}

			template<typename T>
			const T* getConfigValueAs(const ShaderDataDefinition::DefinitionReference& ref) const {
				return (T*)getConfigValue(ref);
			}
			
			virtual void end(uint32 finalInstance) = 0;

			virtual void configure(Shader* shader) = 0;

		protected:
			virtual void* getConfigValue(const ShaderDataDefinition::DefinitionReference& ref) = 0;
	};

	// Interface used to load batch of data
	class IGraphicBatch
	{
		public:
			// Clear all sectors
			// The internal data for the batch is unchanged.
			virtual void clear() = 0;

			// Load data into internal buffer
			// Note this will cause a full memory copy.
			// If possible, prefer using getDataAddressAs, to get the internal buffer address
			// and write to it instead.
			template<typename T>
			void loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, std::vector<T> &data)
			{
				loadVertexData(ref, (char*)data.data(), data.size() * sizeof(T), sizeof(T));
			}

			// Prepare the batch to be rendered, resize internal buffers
			// to fit at least numberOfInstances.
			virtual void prepare(uint32 numberOfInstances) = 0;

			/// \param begin The instance number the sector starts at
			virtual IBatchSector* addSector(uint32 begin) = 0;

			// Retrieve the data address for given instance on the internal buffer
			// \param ref The reference related with the shader data definition
			// \inst the instance to get data for
			template<typename T>
			T* getVertexDataAddressAs(const ShaderDataDefinition::DefinitionReference& ref, uint32 inst) {
				return (T*)getVertexDataAddress(ref, inst);
			}

			// Retrieve the data address for given instance on the internal buffer
			// \param ref The reference related with the shader data definition
			// \inst the instance to get data for
			template<typename T>
			T* getConfigDataAddressAs(const ShaderDataDefinition::DefinitionReference& ref) {
				return (T*)getConfigData(ref);
			}

			// Load all data to gpu
			// Must call this before render
			// This send all data from internal buffers to the gpu
			// When rendering multiple times, this only need to be called once.
			virtual void load() = 0;

			// Do the real rendering
			// load() must have been called before!
			// @see load()
			virtual void render(Shader* shader) = 0;

		protected:
			// Load data into vertex buffer
			// The vector may be changed
			// Implementation will usually swap the vector with an internal one
			// so we avoid extra copies
			virtual void loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, char *data, uint32 nBytes, uint32 strideSize) = 0;

			// Get the address for a vertex data for given instance
			virtual void* getVertexDataAddress(const ShaderDataDefinition::DefinitionReference& ref, uint32 inst) = 0;

			// Get the address for a config
			virtual void* getConfigData(const ShaderDataDefinition::DefinitionReference& ref) = 0;
	};

	// Base shader class for a graphic adapter
	class Shader : public BaseResource
	{
		public:
		/// Binds the shader
		/// Calls OnBind()
		virtual void Bind() = 0;

		/// Unbinds the shader
		virtual void Unbind() = 0;

		virtual IGraphicBatch* createBatch() = 0;

		virtual ShaderDataDefinition& getDefinition() = 0;
		
		protected:
		static const std::vector<ShaderDataDefinition::DefinitionContainer>& getContainers(DataUseMode mode, const ShaderDataDefinition& def)
		{
			return def.m_containers[mode];
		}

	};

	// Handle the load of shader information for a graphic adapter
	// Shader are loaded differently by different APIs
	/*class ShaderManager : public ResourceManager
	{
		public:
			virtual ~ShaderManager() {};
			virtual void Update() = 0;

			virtual void setResourceLoader(ResourceLoader* loader) = 0;

			// Always returns a valid pointer.
			// The shader may be invalid until it is fully loaded
			virtual IShader* getShader(const std::string& str) = 0;

			// by resource id
			virtual IShader* getShader(uint32 id) = 0;
	};*/


}
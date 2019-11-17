#pragma once

#include "bitengine/Core/Graphics/VideoRenderer.h"

namespace BitEngine {
    
    class Shader;
		
	struct ShaderDataReference {
		struct Hasher {
			bool operator() (const ShaderDataReference& lhs, const ShaderDataReference& rhs) const {
				return (*this)(lhs) < (*this)(rhs);
			}
			size_t operator()(const ShaderDataReference& t) const {
				return t.mode >> 16 | t.container >> 8 | t.index;
			}
		};

		ShaderDataReference() : mode(DataUseMode::Vertex), container(0), index(0) {}
        ShaderDataReference(const ShaderDataReference& copy) 
            : mode(copy.mode), container(copy.container), index(copy.index)
        {}
		ShaderDataReference(DataUseMode m, u32 _container, u32 id)
			: mode(m), container(_container), index(id)
		{}
		bool operator==(const ShaderDataReference& o) const {
			return mode == o.mode && container == o.container && index == o.index;
		}

		DataUseMode mode;
		u32 container;
		u32 index;
	};

	class ShaderDataDefinition
	{
		public:
		friend class Shader;
		
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

			DefinitionContainer(u32 id, DataUseMode m, int inst)
				: container(id), mode(m), instanced(inst)
			{}

			DefinitionContainer& addDataDef(const std::string& name, DataType type, int size) {
				definitionData.emplace_back(name, type, size);
				return *this;
			}

			std::vector<DefinitionData> definitionData;
			u32 container;
			DataUseMode mode;
			int instanced;
		};

		const std::vector<DefinitionContainer>& getContainers(DataUseMode mode) const {
			return m_containers[mode];
		}

		DefinitionContainer& addContainer(DataUseMode mode = Vertex, int instanced = 0) {
			const size_t ct = m_containers[mode].size();
			m_containers[mode].emplace_back(ct, mode, instanced);
			return m_containers[mode].back();
		}

		ShaderDataReference findReference(const std::string& name) const
		{
			for (u32 um = 0; um < TotalModes; ++um)
			{
				const std::vector<DefinitionContainer>& v = m_containers[um];
				for (const DefinitionContainer& dc : v)
				{
					for (u32 i = 0; i <  dc.definitionData.size(); ++i)
					{
						const DefinitionData& d = dc.definitionData[i];
						// compare with offset of 2 characters. We ignore the shader code prefix.
						if (name.compare(2, std::string::npos, d.name) == 0)
						{
							return ShaderDataReference((DataUseMode)um, dc.container, i);
						}
					}
				}
			}

			return ShaderDataReference(DataUseMode::TotalModes, 0, 0); // invalid
		}

		DefinitionData& getData(const ShaderDataReference& ref)
		{
			return m_containers[ref.mode][ref.container].definitionData[ref.index];
		}

		// Returns true if a reference is valid
		bool checkRef(const ShaderDataReference& r) const {
			if (r.mode < DataUseMode::TotalModes)
			{
				if (r.container < m_containers[r.mode].size())
				{
					return r.index < m_containers[r.mode][r.container].definitionData.size();
				}
			}
			return false;
		}

		ShaderDataReference getReferenceToContainer(DataUseMode mode, u32 container) const {
			return ShaderDataReference(mode, container, 0);
		}

		private:
			std::vector<DefinitionContainer> m_containers[DataUseMode::TotalModes];
	};

	// Interface used to load batch of data
	class IGraphicBatch
	{
		public:
			virtual ~IGraphicBatch() {}

			// Clear all sectors
			// The internal data for the batch is unchanged.
			virtual void clear() = 0;

			// Set the shader data to be used on the given container reference
			// Internally, the shader will handle where the data will be used by the container value.
			// The batch won't be the new owner for this pointer. It's the callers responsability to keep
			// the data alive while it's in use by the batch (until load() is called).
			template<typename T>
			T* getShaderDataAs(const ShaderDataReference& ref)
			{
				return static_cast<T*>(getShaderData(ref));
			}

			// Prepare the batch to be rendered, resize internal buffers
			// to fit at least numberOfInstances.
			virtual void prepare(u32 numberOfInstances) = 0;

			// Load all data to gpu
			// Must call this before render
			// This send all data from internal buffers to the gpu
			// When rendering multiple times, this only need to be called once.
			virtual void load() = 0;

			// Do the real rendering
			// load() must have been called before!
			// @see load()
			virtual void render(Shader* shader) = 0;

			virtual void setVertexRenderMode(VertexRenderMode mode) = 0;

		protected:
			// Load data into vertex buffer
			// The vector may be changed
			// Implementation will usually swap the vector with an internal one
			// so we avoid extra copies
			virtual void* getShaderData(const ShaderDataReference& ref) = 0;
	};

	// Base shader class for a graphic adapter
	class Shader : public BaseResource
	{
		public:
		Shader(ResourceMeta* meta) : BaseResource(meta) {}
		virtual ~Shader() {}
		virtual bool isReady() = 0;

		/// Binds the shader
		/// Calls OnBind()
		virtual void bind() = 0;

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
	using RShader = RR<Shader>;
}

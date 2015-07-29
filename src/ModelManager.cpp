#include "ModelManager.h"

namespace BitEngine{
	/*
	MeshHolder* MeshManager::LoadMesh_Standard3DShader(const std::string& file)
	{
		auto it = staticMeshes.find(file);
		if (it != staticMeshes.end())
			return it->second;

		Assimp::Importer Importer;

		const aiScene* pScene = Importer.ReadFile(file.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
		if (!pScene)
			return nullptr;

		MeshHolder* meshHolder = new MeshHolder();

		// Initialize the meshes in the scene one by one
		for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
		{
			std::vector<uint32> indices;
			std::vector<StaticMesh::Vertex> vertices;

			const aiMesh* meshi = pScene->mMeshes[i];

			loadStaticMeshData(meshi, vertices, indices);

			GLuint vbo;
			GLuint ibo;

			mesh->meshes.emplace_back(indices.size(), meshi->mMaterialIndex, vbo, ibo);
		}

		// Materials
		meshHolder->materials.resize(pScene->mNumMaterials);
		std::string Dir = ".";
		for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
		{
			const aiMaterial* pMaterial = pScene->mMaterials[i];

			mesh->materials[i] = NULL;

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				aiString Path;

				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string FullPath = Dir + "/" + Path.data;
					Texture *texture = m_textureManager->LoadTexture2D(FullPath);
					m_Textures[i] = texture;

					if (!m_Textures[i]->Load()) {
						printf("Error loading texture '%s'\n", FullPath.c_str());
						delete m_Textures[i];
						m_Textures[i] = NULL;

						return nullptr;
					}
				}
			}

		}

		return nullptr;
	}
	*/
}
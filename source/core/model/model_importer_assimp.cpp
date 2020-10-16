#include "model_importer_assimp.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "model.hpp"

#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <iostream>
#include <set>
#include <sstream>
#include <vector>

namespace model {

static inline float3 aiVector3_to_float3(aiVector3D const& v) noexcept;

Model* Importer_assimp::read(std::string const& name) noexcept {
    //    std::vector<aiNode const*> nodes;
    //    guess_light_nodes(name, nodes);

    //    std::stringstream excludes;

    //    for (auto const n : nodes) {
    //        excludes << n->mName.C_Str() << " ";
    //    }

    importer_.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                 aiComponent_COLORS /*| aiComponent_NORMALS*/);

    //   importer_.SetPropertyString(AI_CONFIG_PP_OG_EXCLUDE_LIST, excludes.str());

    aiScene const* scene = importer_.ReadFile(
        name, aiProcess_ConvertToLeftHanded | aiProcess_RemoveComponent | aiProcess_Triangulate |
                  aiProcess_RemoveRedundantMaterials | aiProcess_PreTransformVertices |
                  aiProcess_JoinIdenticalVertices | aiProcess_FixInfacingNormals |
                  aiProcess_GenSmoothNormals |
                  aiProcess_CalcTangentSpace
                  //    | aiProcess_ImproveCacheLocality
                  | aiProcess_OptimizeMeshes /*| aiProcess_OptimizeGraph*/);

    if (!scene) {
        std::cout << "Could not read \"" << name << "\". " << importer_.GetErrorString()
                  << std::endl;
        return nullptr;
    }

    Model* model = new Model();

    uint32_t const num_parts = scene->mNumMeshes;

    model->allocate_parts(num_parts);

    memory::Buffer<uint32_t> group_vertex_offset(num_parts);

    uint32_t num_materials = 0;
    uint32_t num_vertices  = 0;
    uint32_t num_indices   = 0;

    for (uint32_t m = 0; m < num_parts; ++m) {
        aiMesh const& mesh = *scene->mMeshes[m];

        Model::Part part{num_indices, mesh.mNumFaces * 3, mesh.mMaterialIndex};

        model->set_part(m, part);

        group_vertex_offset[m] = num_vertices;

        num_materials = std::max(num_materials, mesh.mMaterialIndex + 1);
        num_vertices += mesh.mNumVertices;
        num_indices += part.num_indices;
    }

    model->allocate_materials(num_materials);

    for (uint32_t m = 0; m < num_parts; ++m) {
        aiMesh const& mesh = *scene->mMeshes[m];

        uint32_t const mi = mesh.mMaterialIndex;

        if (model->materials()[mi].empty()) {
            model->set_material(mi, *scene->mMaterials[mi]);
        }
    }

    model->set_num_vertices(num_vertices);

    model->allocate_indices(num_indices);

    bool const has_positions = scene->mMeshes[0]->HasPositions();
    bool const has_uvs       = scene->mMeshes[0]->HasTextureCoords(0);
    bool const has_normals   = scene->mMeshes[0]->HasNormals();
    bool const has_tangents  = has_normals && scene->mMeshes[0]->HasTangentsAndBitangents();

    bool const has_uvs_and_tangents = has_uvs && has_tangents;

    if (has_positions) {
        model->allocate_positions();
    }

    if (has_uvs_and_tangents) {
        model->allocate_texture_coordinates();
    }

    if (has_normals) {
        model->allocate_normals();
    }

    if (has_uvs_and_tangents) {
        model->allocate_tangents();
    }

    uint32_t current_vertex = 0;
    uint32_t current_index  = 0;

    for (uint32_t m = 0; m < num_parts; ++m) {
        const aiMesh& mesh = *scene->mMeshes[m];

        // copy per vertex data
        for (uint32_t v = 0, len = mesh.mNumVertices; v < len; ++v, ++current_vertex) {
            if (has_positions && mesh.HasPositions()) {
                model->set_position(current_vertex, aiVector3_to_float3(mesh.mVertices[v]));
            }

            if (has_uvs_and_tangents && mesh.HasTextureCoords(0)) {
                model->set_texture_coordinate(current_vertex, float2(mesh.mTextureCoords[0][v].x,
                                                                     mesh.mTextureCoords[0][v].y));
            }

            if (has_normals && mesh.HasNormals()) {
                if (!has_uvs_and_tangents) {
                    model->set_normal(current_vertex, aiVector3_to_float3(mesh.mNormals[v]));
                } else {
                    if (mesh.mTangents) {
                        float3 const tangent   = aiVector3_to_float3(mesh.mTangents[v]);
                        float3 const bitangent = aiVector3_to_float3(mesh.mBitangents[v]);
                        float3 const normal    = aiVector3_to_float3(mesh.mNormals[v]);

                        model->set_tangent(current_vertex, tangent, bitangent, normal);
                    } else {
                        float3 const normal = aiVector3_to_float3(mesh.mNormals[v]);

                        auto const [tangent, bitangent] = orthonormal_basis(normal);

                        model->set_tangent(current_vertex, tangent, bitangent, normal);
                    }
                }
            }
        }

        // copy the indices
        for (uint32_t f = 0, len = mesh.mNumFaces; f < len; ++f) {
            // after triangulation this should always by 3
            for (uint32_t i = 0, fi = mesh.mFaces[f].mNumIndices; i < fi; ++i, ++current_index) {
                uint32_t const index = group_vertex_offset[m] + mesh.mFaces[f].mIndices[i];

                model->set_index(current_index, index);
            }
        }
    }

    return model;
}

bool contains_material(aiNode const* node, aiScene const* scene,
                       std::set<uint32_t> const& materials) noexcept {
    for (uint32_t i = 0, len = node->mNumMeshes; i < len; ++i) {
        uint32_t const mi = scene->mMeshes[node->mMeshes[i]]->mMaterialIndex;

        if (materials.end() != materials.find(mi)) {
            return true;
        }
    }

    return false;
}

void gather_nodes(aiNode const* node, aiScene const* scene, std::set<uint32_t> const& materials,
                  std::vector<aiNode const*>& nodes) noexcept {
    uint32_t const num_children = node->mNumChildren;

    if (0 == num_children) {
        if (contains_material(node, scene, materials)) {
            nodes.push_back(node);
        }
    } else {
        for (uint32_t i = 0; i < num_children; ++i) {
            gather_nodes(node->mChildren[i], scene, materials, nodes);
        }
    }
}

void Importer_assimp::guess_light_nodes(std::string const&          name,
                                        std::vector<aiNode const*>& nodes) noexcept {
    aiScene const* scene = importer_.ReadFile(name, 0);

    if (!scene) {
        return;
    }

    std::set<uint32_t> emissive_materials;

    for (uint32_t i = 0, len = scene->mNumMaterials; i < len; ++i) {
        std::string const material_name = scene->mMaterials[i]->GetName().C_Str();

        if (std::string::npos != material_name.find("Emissive")) {
            emissive_materials.insert(i);
        }
    }

    gather_nodes(scene->mRootNode, scene, emissive_materials, nodes);
}

static inline float3 aiVector3_to_float3(aiVector3D const& v) noexcept {
    float const x = std::isnan(v.x) || std::isinf(v.x) ? 0.f : v.x;
    float const y = std::isnan(v.y) || std::isinf(v.y) ? 0.f : v.y;
    float const z = std::isnan(v.z) || std::isinf(v.z) ? 0.f : v.z;

    return float3(x, y, z);
}

}  // namespace model

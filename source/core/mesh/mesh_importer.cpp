#include "mesh_importer.hpp"
#include "mesh.hpp"

#include <assimp/postprocess.h>
#include <iostream>

namespace mesh {

Mesh* Importer::read(std::string const& name) noexcept {
    importer_.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS /*| aiComponent_NORMALS*/);

    aiScene const* scene = importer_.ReadFile(name,
          aiProcess_ConvertToLeftHanded
        | aiProcess_RemoveComponent
        | aiProcess_Triangulate
        | aiProcess_RemoveRedundantMaterials
        | aiProcess_PreTransformVertices
        | aiProcess_JoinIdenticalVertices
        | aiProcess_FixInfacingNormals
        | aiProcess_GenSmoothNormals
        | aiProcess_CalcTangentSpace
    //	| aiProcess_ImproveCacheLocality
        | aiProcess_OptimizeMeshes
        | aiProcess_OptimizeGraph
    );

    if (!scene) {
        std::cout << "Could not read \"" << name << "\". " << importer_.GetErrorString() << std::endl;
        return nullptr;
    }

    return new Mesh();
}

}

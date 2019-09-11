#ifndef SU_CORE_MESH_IMPORTER_HPP
#define SU_CORE_MESH_IMPORTER_HPP

#include <assimp/Importer.hpp>
#include <string>

namespace mesh {

class Mesh;

class Importer {
public:

    Mesh* read(std::string const& name) noexcept;

private:

    Assimp::Importer importer_;
};

}

#endif

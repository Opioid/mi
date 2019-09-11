#include "mesh_importer.hpp"
#include "mesh.hpp"

namespace mesh {

Mesh* Importer::import() const noexcept {
    return new Mesh();
}

}

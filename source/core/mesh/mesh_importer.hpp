#ifndef SU_CORE_MESH_IMPORTER_HPP
#define SU_CORE_MESH_IMPORTER_HPP

namespace mesh {

class Mesh;

class Importer {
public:

    Mesh* import() const noexcept;
};

}

#endif

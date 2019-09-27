#ifndef SU_CORE_MODEL_IMPORTER_HPP
#define SU_CORE_MODEL_IMPORTER_HPP

#include <assimp/Importer.hpp>
#include <string>
#include <vector>

struct aiNode;

namespace model {

class Model;

class Importer {
  public:
    Model* read(std::string const& name) noexcept;

  private:
    void guess_light_nodes(std::string const& name, std::vector<aiNode const*>& nodes) noexcept;

    Assimp::Importer importer_;
};

}  // namespace model

#endif

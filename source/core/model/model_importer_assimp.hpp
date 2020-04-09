#ifndef SU_CORE_MODEL_IMPORTER_ASSIMP_HPP
#define SU_CORE_MODEL_IMPORTER_ASSIMP_HPP

#include "assimp/Importer.hpp"
#include "model_importer.hpp"

#include <vector>

struct aiNode;

namespace model {

class Model;

class Importer_assimp : public Importer {
  public:
    Model* read(std::string const& name) noexcept final;

  private:
    void guess_light_nodes(std::string const& name, std::vector<aiNode const*>& nodes) noexcept;

    Assimp::Importer importer_;
};

}  // namespace model

#endif

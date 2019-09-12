#ifndef SU_CORE_MODEL_IMPORTER_HPP
#define SU_CORE_MODEL_IMPORTER_HPP

#include <assimp/Importer.hpp>
#include <string>

namespace model {

class Model;

class Importer {
  public:
    Model* read(std::string const& name) noexcept;

  private:
    Assimp::Importer importer_;
};

}  // namespace model

#endif

#ifndef SU_CORE_MODEL_IMPORTER_JSON_HPP
#define SU_CORE_MODEL_IMPORTER_JSON_HPP

#include "model_importer.hpp"

#include <string>

struct aiNode;

namespace model {

class Model;

class Importer_json : public Importer {
  public:
    Model* read(std::string const& name) noexcept final;
};

}  // namespace model

#endif

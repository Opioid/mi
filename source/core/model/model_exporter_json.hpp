#ifndef SU_CORE_MODEL_EXPORTER_JSON_HPP
#define SU_CORE_MODEL_EXPORTER_JSON_HPP

#include "model_exporter.hpp"

#include <string>

namespace model {

class Model;

class Exporter_json {
  public:
    bool write(std::string const& name, Model const& model) noexcept;
};

}  // namespace model

#endif

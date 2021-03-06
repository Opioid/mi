#ifndef SU_CORE_MODEL_EXPORTER_JSON_HPP
#define SU_CORE_MODEL_EXPORTER_JSON_HPP

#include <string>

namespace model {

class Model;

class Exporter_json {
  public:
    bool write(std::string const& name, Model const& model) const noexcept;

    bool write_materials(std::string const& name, std::string const& scene_name, Model const& model) const noexcept;
};

}  // namespace model

#endif

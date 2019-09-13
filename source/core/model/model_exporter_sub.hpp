#ifndef SU_CORE_MODEL_EXPORTER_SUB_HPP
#define SU_CORE_MODEL_EXPORTER_SUB_HPP

#include <string>

namespace model {

class Model;

class Exporter_sub {
  public:
    bool write(std::string const& name, Model const& model) const noexcept;
};

}  // namespace model

#endif

#ifndef SU_CORE_MODEL_IMPORTER_HPP
#define SU_CORE_MODEL_IMPORTER_HPP

#include <string>

namespace model {

class Model;

class Importer {
  public:
    virtual Model* read(std::string const& name) noexcept = 0;
};

}  // namespace model

#endif

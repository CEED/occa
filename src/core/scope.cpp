#include <occa/core/scope.hpp>

namespace occa {
  scope::scope() {}

  scope::scope(const occa::properties &props_) :
      props(props_) {}

  occa::device scope::getDevice() {
    return device;
  }

  kernelArg scope::getArg(const std::string &name) {
    const int argCount = (int) args.size();

    for (int i = 0; i < argCount; ++i) {
      scopeVariable &arg = args[i];
      if (arg.name == name) {
        return arg.value;
      }
    }

    OCCA_FORCE_ERROR("Missing argument [" << name << "]");
    return kernelArg();
  }
}
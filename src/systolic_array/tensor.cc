#include <iostream>

#include "tensor.hh"

namespace gem5 {

std::ostream& operator<<(std::ostream& os, const TensorShape& shape) {
  os << "(";
  for (int i = 0; i < shape.ndims(); i++) {
    os << shape[i];
    if (i != shape.ndims() - 1)
      os << ", ";
  }
  os << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const TensorIndexIterator& iter) {
  os << "( ";
  for (int i = 0; i < iter.dims.size(); ++i) {
    os << iter.state[i] << " ";
  }
  os << ")";
  return os;
}

}  // namespace Systolic

#ifndef PYTHONIC_NUMPY_ARGMIN_HPP
#define PYTHONIC_NUMPY_ARGMIN_HPP

#include "pythonic/include/numpy/argmin.hpp"

#include "pythonic/numpy/argminmax.hpp"

PYTHONIC_NS_BEGIN

namespace numpy
{
  template <class E>
  struct argmin_op {
    static typename E::dtype constexpr limit()
    {
      return std::numeric_limits<typename E::dtype>::max();
    }
    template <class T>
    static T elements(T first, T last)
    {
      return std::min_element(first, last);
    }
    template <class T>
    static T value(T self, T other)
    {
      return self < other;
    }
  };

  template <class E>
  long argmin(E const &expr)
  {
    return argminmax<argmin_op<E>>(expr);
  }

  DEFINE_FUNCTOR(pythonic::numpy, argmin);
}
PYTHONIC_NS_END

#endif

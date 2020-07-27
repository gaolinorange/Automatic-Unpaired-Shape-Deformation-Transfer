// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_CHECKSUMLOGVALUET_HH_INCLUDE
#define BASE_CHECKSUMLOGVALUET_HH_INCLUDE

#include <Base/Test/Checksum.hh>

#ifdef TEST_ON
#include <cmath>
#include <sstream>

namespace Test {
namespace Checksum {

/*! Default implementation of a comparison class for ValueT.
*/
template <typename ValueT>
struct DefaultCompareT
{
  bool same(const ValueT& _a, const ValueT& _b) const { return _a == _b; }
};

/*! Comparison class for checksums that MUST only be logged.
*/
template <typename ValueT>
struct NoCompareT
{
  bool same(const ValueT& _a, const ValueT& _b) const { return true; }
};


/*!
Utility class to compare double with a tolerance. Can be used to redefine the
default compare class in class ValueT.
*/
struct DoubleCompare
{
  DoubleCompare(double _tol = 1e-12) : tol_(_tol) {}
  bool same(const double& _a, const double& _b) const
  {
    return std::fabs(_a - _b) <= tol_;
  }
private:
  double tol_;
};

/*!
Generic checksum class to record and compare a value of a certain type. 
*/
template <typename ValueT, class CompareT = DefaultCompareT<ValueT>>
class NumberT : public Object
{
public:
  NumberT(
    const char* const _name,           //!<[in] Checksum name.
    const CompareT& _comp = CompareT() //!<[in] Comparison function.
    )
    : Object(_name), comp_(_comp)
  {}

protected:
  //! Generic implementation of number data comparison
  virtual Difference compare_data(const String& _old, const String& _new) const
  {
    std::istringstream strm_old(_old), strm_new(_new);
    ValueT val_old, val_new;
    strm_old >> val_old;
    strm_new >> val_new;

    // TODO: multiple comparisons, can we use just one?
    if (val_new == val_old) // bitwise comparison
      return Difference::EQUAL;
    if (comp_.same(val_old, val_new)) // tolerance comparison
      return Difference::NEGLEGIBLE;

    Base::OStringStream diff;
    diff << (val_new - val_old);
    return Difference(Difference::UNKNOWN, diff.str);
  }

private:
  CompareT comp_; // Compare class.
};

}//namespace Checksum
}//namespace Test

#endif//TEST_ON
#endif//BASE_CHECKSUMLOGVALUET_HH_INCLUDE

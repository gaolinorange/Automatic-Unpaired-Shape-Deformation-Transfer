// (C) Copyright 2016 by Autodesk, Inc.

#ifndef BASE_ICHECKSUM_HH_INCLUDE
#define BASE_ICHECKSUM_HH_INCLUDE

#ifndef TEST_ON

#define TEST(CHKSM, RCRD)

#else

#include <Base/Test/TestResult.hh>
#include <Base/Utils/OStringStream.hh>
#include <map>
#include <sstream>

namespace Test {
namespace Checksum {

//! Enumerate the checksum levels 
enum Level { L_NONE, L_STABLE, L_PRIME, L_ALL };
extern Level run_lvl; //<! The checksum run level
const char* const LEVEL_TEXT[4] = { "NONE", "STABLE", "PRIME", "ALL" };

//! typedef String, this is used a lot in this namespace
typedef std::string String;

//! The checksum record
struct Record
{
  Record() {}
  Record(const Result& _rslt, const String _data) : rslt(_rslt), data(_data) {}

  Result rslt; //! record result
  String data; //!< recorded "data" (as string, can be parsed)
};

//! The difference found by the IChecksum::compare() operation
class Difference 
{
public:
  enum Type
  {
    EQUAL, // result is bitwise identical 
    UNKNOWN, // non-negligible difference, but of unknown quality 
    IMPROVED, // result is better
    NEGLEGIBLE, // result is negligibly different
    SUSPICIOUS, // result is different, and the new result might be worse
    REGRESSED, // result is worse
    WORKED,  // result works now, but used to fail
    FAILED // result fails now, but used to work
  };

  static const char* const type_text(const Type _type)
  {
    static const char dscr[][32] = 
    {
      "EQUAL",
      "UNKNOWN",
      "IMPROVED",
      "NEGLEGIBLE", 
      "SUSPICIOUS",
      "REGRESSED",
      "WORKED",
      "FAILED"
    };
    return dscr[_type];
  }

  Difference(const Type _type = EQUAL, const String& _dscr = String())
    : type_(_type), dscr_(_dscr) 
  {}

  const Type type() const { return type_; }
  bool equal() const { return type() == EQUAL; }

  bool operator==(const Difference& _othr) const
  {
    return type_ == _othr.type_ && dscr_ == _othr.dscr_;
  }

  Difference& operator+=(const Difference& _othr)
  {
    if (type_ < _othr.type_)
      type_ = _othr.type_;
    if (dscr_.empty())
      dscr_ = _othr.dscr_;
    else if (!_othr.dscr_.empty())
      dscr_ += "; " + _othr.dscr_;
    return *this;
  }

  Difference& operator+=(const Difference::Type& _type)
  {
    if (type_ < _type)
      type_ = type_;
    return *this;
  }

  const String& description() const
  {
    return dscr_;
  }

  const char* const type_text() const { return type_text(type_); }

  friend Base::IOutputStream& operator<<(Base::IOutputStream& _os, 
    Difference& _diff)
  {
    // TODO: use string description array
    return _os << _diff.type_text() << " " << _diff.dscr_;
  }

  bool operator<(const Difference& _othr) const { return type_ < _othr.type_; }

private:
  Type type_;
  String dscr_;
};


/*!
Base class for test checksums. Whatever check we want to add in the test system,
it must be an instance of a class derived from Checksum. All derived classes
must be instantiated as global variables.
*/
class Object
{
public:
  //! Checksum name. 
  const char* const name() const { return name_; }

  //! Add a record the checksum (generic version)
  template <typename T>
  void record(const Result& _rslt, const T& _data) 
  {
    Base::OStringStream strm;
    strm << _data;
    add(_rslt, strm.str);
  }

  //! Add a record of the checksum (public version)
  void record(const Result& _rslt, const String& _data) { add(_rslt, _data); }

  /*!
  Compare two existing records (old and new).
  Returns a qualification and a description of the difference.
  The default implementation has some intelligence in comparing the record 
  results, but compares the the data simply as strings (no parsing). 
  */
  virtual Difference compare(
    const Path& _old_path, //!<[in] Path to the left record
    const Record& _old_rcrd, //!<[in] "Left" record
    const Path& _new_path, //!<[in] Path to the right record
    const Record& _new_rcrd //!<[in] "Right" record
   ) const;

  //! Get if the checksum should be run
  bool allow() const { return lvl_ <= run_lvl; }

protected:
  /*!
  Performs an automatic registration of the new checksum in a
  global list, and verifies that the name is unique.
  */
  Object(const char* const _name, const Level _lvl = L_ALL);

  //! Add a record of the checksum (protected version)
  void add(const Result& _rslt, const String& _data);

  //! Compare the data, the default implementation does a string comparison
  virtual Difference compare_data(const String& _old, const String& _new) const;

protected:
  const Level lvl_;

private:
  const char* const name_;

private:
  Object(const Object&);
  Object* operator=(const Object&);
};

/*!
Definition of the checksums registry. It is a map from a string (that is the 
checksum name to an IChecksum.
*/
typedef std::map<String, Object*> Registry; 

/*!
Function to get a static map with all the registered checksums.
*/
const Registry& registry();

}//namespace Checksum
}//namespace Test

#define TEST(CHKSM, RCRD) { if (CHKSM.allow()) { CHKSM.RCRD; } }

#endif//TEST_ON


#endif//BASE_ICHECKSUM_HH_INCLUDE

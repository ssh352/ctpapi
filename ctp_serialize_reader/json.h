#ifndef JSON_H
#define JSON_H
#include <string>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/sequence/intrinsic/value_at.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/spirit/home/support/container.hpp>
#include <boost/type_traits.hpp>
#include <boost/units/detail/utility.hpp>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "ctpapi/ThostFtdcUserApiStruct.h"

template <typename S>
struct sequence {
  // Point to the first element
  typedef boost::mpl::int_<0> begin;

  // Point to the element after the last element in the sequence
  typedef typename boost::fusion::result_of::size<S>::type end;

  // Point to the first element
  typedef boost::mpl::int_<0> first;

  // Point to the second element (for pairs)
  typedef boost::mpl::int_<1> second;

  // Point to the last element in the sequence
  typedef typename boost::mpl::prior<end>::type last;

  // Number of elements in the sequence
  typedef typename boost::fusion::result_of::size<S>::type size;
};

template <typename S, typename N>
struct element_at {
  // Type of the element at this index
  typedef typename boost::fusion::result_of::value_at<S, N>::type type;

  // Previous element
  typedef typename boost::mpl::prior<N>::type previous;

  // Next element
  typedef typename boost::mpl::next<N>::type next;

  // Member name of the element at this index
  static inline std::string name(void) {
    return boost::fusion::extension::struct_member_name<S, N::value>::call();
  }

  // Type name of the element at this index
  static inline std::string type_name(void) {
    return boost::units::detail::demangle(typeid(type).name());
  }

  // Access the element
  static inline typename boost::fusion::result_of::at<S const, N>::type get(
      S const& s) {
    return boost::fusion::at<N>(s);
  }
};

// Insert a comma into the stream
template <typename S, typename N>
struct separator {
  static inline std::string comma() { return ","; }
};

// Specialize for the last element in the sequence
template <typename S>
struct separator<S, typename sequence<S>::last> {
  static inline std::string comma() { return ""; }
};

// Forward
template <typename T>
struct json_serializer;

static inline std::string tab(int depth) {
  std::string retval;
  for (int i = 0; i < depth; ++i) {
    retval += "   ";
  }
  return retval;
}

// Occurs at every element of the sequence
template <typename S, typename N>
struct struct_serializer_recursive {
  typedef typename element_at<S, N>::type current_t;
  typedef typename element_at<S, N>::next next_t;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               S const& s,
                               int depth,
                               bool array_value,
                               bool pair) {
    std::string name = element_at<S, N>::name();
    current_t const& t = element_at<S, N>::get(s);

    os << "\n" + tab(depth) + "\"" << name << "\" : ";

    json_serializer<current_t>::serialize(os, t, depth, false, false);

    os << separator<S, N>::comma();

    struct_serializer_recursive<S, next_t>::serialize(os, s, depth, false,
                                                      false);
  }
};

template <typename S>
struct struct_serializer_recursive<S, typename sequence<S>::end> {
  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               S const& s,
                               int depth,
                               bool array_value,
                               bool pair) {
    // No output
  }
};

template <typename S>
struct struct_serializer_initiate
    : struct_serializer_recursive<S, typename sequence<S>::begin> {};

template <typename S, typename N>
struct pair_serializer_recursive {
  typedef typename element_at<S, N>::type current_t;
  typedef typename element_at<S, N>::next next_t;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               S const& s,
                               int depth,
                               bool array_value,
                               bool pair) {
    current_t const& t = element_at<S, N>::get(s);

    os << "\n" + tab(depth) + "\"" << t << "\" : ";

    pair_serializer_recursive<S, next_t>::serialize(os, s, depth, false, false);
  }
};

template <typename S>
struct pair_serializer_recursive<S, typename sequence<S>::second> {
  typedef typename element_at<S, typename sequence<S>::second>::type current_t;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               S const& s,
                               int depth,
                               bool array_value,
                               bool pair) {
    current_t const& t = element_at<S, typename sequence<S>::second>::get(s);

    os << "\"" << t << "\"";
  }
};

template <typename T>
struct pair_serializer_initiate
    : public pair_serializer_recursive<T, typename sequence<T>::begin> {};

template <typename T>
struct array_serializer {
  typedef array_serializer<T> type;

  typedef typename boost::remove_bounds<T>::type slice_t;

  static const size_t size = sizeof(T) / sizeof(slice_t);

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               T const& t,
                               int depth,
                               bool array_value,
                               bool pair) {
    os << "\n" + tab(depth) + "[";

    for (unsigned int i = 0; i < size; ++i) {
      json_serializer<slice_t>::serialize(os, t[i], depth + 1, true, false);

      if (i != size - 1) {
        os << ", ";
      }
    }

    os << "\n" + tab(depth) + "]";
  }
};

template <typename T>
struct container_serializer {
  typedef container_serializer<T> type;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               T const& t,
                               int depth,
                               bool array_value,
                               bool pair) {
    os << "\n" + tab(depth) + "[";

    std::size_t size = t.size();
    std::size_t count = 0;

    BOOST_FOREACH (typename T::value_type const& v, t) {
      json_serializer<typename T::value_type>::serialize(os, v, depth + 1, true,
                                                         false);

      if (count != size - 1) {
        os << ", ";
      }

      ++count;
    }

    os << "\n" + tab(depth) + "]";
  }
};

template <typename K, typename V, typename C, typename A>
struct container_serializer<std::map<K, V, C, A> > {
  typedef std::map<K, V, C, A> T;
  typedef container_serializer<T> type;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               T const& t,
                               int depth,
                               bool array_value,
                               bool pair) {
    os << "\n" + tab(depth) + "{";

    std::size_t size = t.size();
    std::size_t count = 0;

    BOOST_FOREACH (typename T::value_type v, t) {
      json_serializer<typename T::value_type>::serialize(os, v, depth + 1, true,
                                                         true);

      if (count != size - 1) {
        os << ", ";
      }

      ++count;
    }

    os << "\n" + tab(depth) + "}";
  }
};

template <typename T>
struct struct_serializer {
  typedef struct_serializer<T> type;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               T const& t,
                               int depth,
                               bool array_value,
                               bool pair) {
    if (pair) {
      pair_serializer_initiate<T>::serialize(os, t, depth, false, false);
    } else {
      os << "\n" + tab(depth) + "{";
      struct_serializer_initiate<T>::serialize(os, t, depth + 1, false, false);
      os << "\n" + tab(depth) + "}";
    }
  }
};

template <typename T>
struct primitive_serializer {
  typedef primitive_serializer<T> type;

  template <typename Ostream>
  static inline void serialize(Ostream& os,
                               T const& t,
                               int depth,
                               bool array_value,
                               bool pair) {
    if (array_value) {
      os << "\n" + tab(depth) + "\"" << t << "\"";
    } else {
      os << "\"" << t << "\"";
    }
  }
};

template <typename T>
struct choose_serializer {
  typedef typename boost::mpl::eval_if<
      boost::is_array<T>,
      boost::mpl::identity<primitive_serializer<T> >,
      // boost::mpl::identity<array_serializer<T> >,
      typename boost::mpl::eval_if<
          boost::spirit::traits::is_container<T>,
          boost::mpl::identity<primitive_serializer<T> >,
          // boost::mpl::identity<container_serializer<T> >,
          typename boost::mpl::eval_if<
              boost::is_class<T>,
              boost::mpl::identity<struct_serializer<T> >,
              boost::mpl::identity<primitive_serializer<T> > > > >::type type;
};

template <typename T>
struct json_serializer : public choose_serializer<T>::type {};

template <typename T>
struct json {
  std::string to_json(void) {
    std::stringstream ss;
    json_serializer<T>::serialize(ss << std::boolalpha, self(), 0, false,
                                  false);
    return ss.str();
  }

 private:
  T const& self(void) const { return *static_cast<T const*>(this); }
};

template <typename T>
void ToJson(std::ofstream& os, T& order) {
  json_serializer<T>::serialize(os, order, 0, false, false);
}

#endif  // _DEBUG


//          Copyright Dean Michael Berris 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef __NETWORK_MESSAGE_TRANSFORMERS_TO_UPPER_HPP__
#define __NETWORK_MESSAGE_TRANSFORMERS_TO_UPPER_HPP__

#include <boost/algorithm/string.hpp>

/** to_upper.hpp
 *
 * Implements the to_upper transformer. This applies
 * the to_upper string algorithm to a string, which
 * is selected by the appropriate selector.
 *
 * This defines a type, to be applied using template
 * metaprogramming on the selected string target.
 */
namespace boost {
namespace network {

namespace impl {

template <class Selector>
struct to_upper_transformer {};

template <>
struct to_upper_transformer<selectors::source_selector> {
  template <class Tag>
  void operator()(basic_message<Tag> &message_) const {
    boost::to_upper(message_.source());
  }

 protected:
  ~to_upper_transformer() {};
};

template <>
struct to_upper_transformer<selectors::destination_selector> {
  template <class Tag>
  void operator()(basic_message<Tag> &message_) const {
    boost::to_upper(message_.destination());
  }

 protected:
  ~to_upper_transformer() {};
};

}  // namespace impl

namespace detail {
struct to_upper_placeholder_helper;
}

detail::to_upper_placeholder_helper to_upper_(
    detail::to_upper_placeholder_helper);

namespace detail {

struct to_upper_placeholder_helper {
  template <class Selector>
  struct type : public impl::to_upper_transformer<Selector> {};

 private:
  to_upper_placeholder_helper() {}
  to_upper_placeholder_helper(to_upper_placeholder_helper const &) {}
  friend to_upper_placeholder_helper boost::network::to_upper_(
      to_upper_placeholder_helper);
};
}

typedef detail::to_upper_placeholder_helper (*to_upper_placeholder)(
    detail::to_upper_placeholder_helper);

inline detail::to_upper_placeholder_helper to_upper_(
    detail::to_upper_placeholder_helper) {
  return detail::to_upper_placeholder_helper();
}

}  // namespace network

}  // namespace boost

#endif  // __NETWORK_MESSAGE_TRANSFORMERS_TO_UPPER_HPP__

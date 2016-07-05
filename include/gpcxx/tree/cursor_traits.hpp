/*
  gpcxx/tree/cursor_traits.hpp

  Copyright 2013 Karsten Ahnert

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE_1_0.txt or
  copy at http://www.boost.org/LICENSE_1_0.txt)
*/


#ifndef GPCXX_TREE_CURSOR_TRAITS_HPP_DEFINED
#define GPCXX_TREE_CURSOR_TRAITS_HPP_DEFINED

#include <type_traits>

namespace gpcxx {

template< typename Cursor >
struct is_cursor : public std::false_type
{
};


template< typename Cursor >
struct cursor_value
{
    typedef typename Cursor::value_type type;
};

template< typename Cursor >
struct cursor_reference
{
    typedef typename Cursor::reference type;
};

template< typename Cursor >
struct cursor_const_reference
{
    typedef typename Cursor::const_reference type;
};

template< typename Cursor >
struct cursor_pointer
{
    typedef typename Cursor::pointer type;
};

template< typename Cursor >
struct cursor_difference
{
    typedef typename Cursor::difference_type type;
};

template< typename Cursor >
struct cursor_size
{
    typedef typename Cursor::size_type type;
};

// template< typename Cursor >
// struct cursor_category
// {
//     typedef typename Cursor::cursor_category type;
// };

} // namespace gpcxx


#endif // GPCXX_TREE_CURSOR_TRAITS_HPP_DEFINED

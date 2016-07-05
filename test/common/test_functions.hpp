/*
 * gpcxx/test/common/test_functions.hpp
 * Date: 2014-02-06
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_TEST_COMMON_TEST_FUNCTIONS_HPP_INCLUDED
#define GPCXX_TEST_COMMON_TEST_FUNCTIONS_HPP_INCLUDED

#include <gpcxx/tree/intrusive_nodes/intrusive_named_func_node.hpp>
#include <gpcxx/tree/intrusive_nodes/intrusive_nary_named_func_node.hpp>
#include <gpcxx/canonic/algebraic_node.hpp>

#include <gtest/gtest.h>

template< typename T >
void test_value( T const& t , std::string const& value )
{
    EXPECT_EQ( t , value );
}

template< typename Res , typename Context, size_t Arity >
void test_value( gpcxx::intrusive_nary_named_func_node< Res , Context , Arity > const& t , std::string const& value )
{
    EXPECT_EQ( t.name() , value );
}

template< typename Res , typename Context, typename Allocator >
void test_value( gpcxx::intrusive_named_func_node< Res , Context , Allocator > const& t , std::string const& value )
{
    EXPECT_EQ( t.name() , value );
}

template< typename Res , typename Context , typename Allocator >
void test_value( gpcxx::algebraic_node< Res , Context , Allocator > const& t , std::string const& value )
{
    EXPECT_EQ( t.name() , value );
}





template< typename Cursor >
void test_cursor( Cursor n , std::string const& value , size_t arity , size_t height , size_t level )
{
    EXPECT_EQ( n.size() , arity );
    EXPECT_EQ( n.height() , height );
    EXPECT_EQ( n.level() , level );
    test_value( *n , value );
    EXPECT_TRUE( n.valid() );
    EXPECT_FALSE( n.invalid() );
}

    






#endif // GPCXX_TEST_COMMON_TEST_FUNCTIONS_HPP_INCLUDED

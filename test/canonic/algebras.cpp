/*
 * test/canonic/algebras.cpp
 * Date: 2015-09-02
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <gpcxx/canonic/algebras.hpp>
#include <gpcxx/canonic/algebraic_node.hpp>
#include <gpcxx/tree/intrusive_functions.hpp>

#include <gtest/gtest.h>

#include <array>

#define TESTNAME algebras_tests

using namespace std;
using namespace gpcxx;

using node_type = algebraic_node< double , std::array< double , 2 > const >;
using algebras_type = algebras< node_type >;
using group_type = algebras_type::group_type;

TEST( TESTNAME , test_default_construction )
{
    EXPECT_NO_THROW( algebras_type algebras; );
}

TEST( TESTNAME , test_add_abelian_magmas )
{
    algebras_type algebras;
    auto n1 = node_type::make_binary_operation( gpcxx::plus_func {} , "+" );
    auto n2 = node_type::make_binary_operation( gpcxx::multiplies_func {} , "*" );
    algebras.add_abelian_magma( n1 );
    EXPECT_TRUE( algebras.is_commutative( n1 ) );
    EXPECT_FALSE( algebras.is_commutative( n2 ) );
}

TEST( TESTNAME , test_add_associative_operation )
{
    algebras_type algebras;
    auto n1 = node_type::make_binary_operation( gpcxx::plus_func {} , "+" );
    auto n2 = node_type::make_binary_operation( gpcxx::multiplies_func {} , "*" );
    algebras.add_associative_operation( n1 );
    EXPECT_TRUE( algebras.is_associative( n1 ) );
    EXPECT_FALSE( algebras.is_associative( n2 ) );
}

TEST( TESTNAME , test_add_group )
{
    algebras_type algebras;
    auto op = node_type::make_binary_operation( gpcxx::plus_func {} , "+" );
    auto iop = node_type::make_binary_operation( gpcxx::minus_func {} , "-" );
    auto id = node_type::make_constant_terminal( gpcxx::double_terminal<> { 0.0 } , "0" );
    auto um = node_type::make_identity_operation( gpcxx::unary_minus_func {} , "um" );
    group_type group { op , id , iop , um };
        
    algebras.add_group( group );
    
    EXPECT_TRUE( static_cast< bool >( algebras.get_group( op ) ) );
    EXPECT_FALSE( static_cast< bool >( algebras.get_group( iop ) ) );
    EXPECT_TRUE( static_cast< bool >( algebras.get_group_from_inverse_operation( iop ) ) );
    EXPECT_FALSE( static_cast< bool >( algebras.get_group_from_inverse_operation( op ) ) );
    EXPECT_TRUE( static_cast< bool >( algebras.get_group_from_inverse_function( um ) ) );
}

TEST( TESTNAME , test_add_abelian_group )
{
    algebras_type algebras;
    auto op = node_type::make_binary_operation( gpcxx::plus_func {} , "+" );
    auto iop = node_type::make_binary_operation( gpcxx::minus_func {} , "-" );
    auto id = node_type::make_constant_terminal( gpcxx::double_terminal<> { 0.0 } , "0" );
    auto um = node_type::make_identity_operation( gpcxx::unary_minus_func {} , "um" );
    group_type group { op , id , iop , um };
        
    algebras.add_abelian_group( group );
    
    EXPECT_TRUE( static_cast< bool >( algebras.get_group( op ) ) );
    EXPECT_FALSE( static_cast< bool >( algebras.get_group( iop ) ) );
    EXPECT_TRUE( static_cast< bool >( algebras.get_group_from_inverse_operation( iop ) ) );
    EXPECT_FALSE( static_cast< bool >( algebras.get_group_from_inverse_operation( op ) ) );
    EXPECT_TRUE( static_cast< bool >( algebras.get_group_from_inverse_function( um ) ) );
}

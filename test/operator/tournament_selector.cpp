/*
 test/operator/tournament_selector.cpp

 Copyright 2013 Karsten Ahnert

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <gp/operator/tournament_selector.hpp>
#include "../common/test_tree.hpp"
#include "../common/test_generator.hpp"

#include <gtest/gtest.h>

using namespace std;

TEST( operator_tests , tournament_selector_instanciation )
{
    test_tree tree;
    test_generator gen;
    std::vector< test_tree::node_type > pop( 10 , test_tree::node_type() );
    std::vector< double > fitness( 10 );
    auto selector = gp::make_tournament_selector( gen.rng , 7 );
    auto node = selector( pop , fitness );
}

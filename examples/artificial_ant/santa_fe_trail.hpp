/*
 * gpcxx/examples/artificial_ant/santa_fe_trail.hpp
 * Date: 2014-08-18
 * Author: Gerard Choinka (gerard.choinka+gpcxx@gmail.com)
 * Copyright: Gerard Choinka
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_EXAMPLES_ARTIFICIAL_ANT_SANTA_FE_TRAIL_HPP_INCLUDED
#define GPCXX_EXAMPLES_ARTIFICIAL_ANT_SANTA_FE_TRAIL_HPP_INCLUDED

#include <cstddef>

namespace santa_fe 
{

constexpr size_t const x_size = 32;
constexpr size_t const y_size = 32;
//[santa_fe_trail_definition
char const * const board1[y_size] = {
" XXX                            ",
"   X                            ",
"   X                    .XXX..  ",
"   X                    X    X  ",
"   X                    X    X  ",
"   XXXX.XXXXX       .XX..    .  ",
"            X       .        X  ",
"            X       X        .  ",
"            X       X        .  ",
"            X       X        X  ",
"            .       X        .  ",
"            X       .        .  ",
"            X       .        X  ",
"            X       X        .  ",
"            X       X  ...XXX.  ",
"            .   .X...  X        ",
"            .   .      .        ",
"            X   .      .        ",
"            X   X      .X...    ",
"            X   X          X    ",
"            X   X          .    ",
"            X   X          .    ",
"            X   .      ...X.    ",
"            X   .      X        ",
" ..XX..XXXXX.   X               ",
" X              X               ",
" X              X               ",
" X     .XXXXXXX..               ",
" X     X                        ",
" .     X                        ",
" .XXXX..                        ",
"                                "
};
//]


} // namespace santa_fe 


#endif // GPCXX_EXAMPLES_ARTIFICIAL_ANT_SANTA_FE_TRAIL_HPP_INCLUDED

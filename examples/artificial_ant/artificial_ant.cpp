/*
 * gpcxx/examples/artificial_ant/santa_fe_ant_trail_main.cpp
 * Date: 2014-07-18
 * Author: Gerard Choinka (gerard.choinka+gpcxx@gmail.com)
 * Copyright: Gerard Choinka
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "simulation.hpp"
#include "nodes.hpp"
#include "santa_fe_trail.hpp"
#include "ant_simulation_common.hpp"
#include <gpm/gpm.hpp>

#include <gpcxx/app.hpp>
#include <gpcxx/evolve.hpp>
#include <gpcxx/generate.hpp>
#include <gpcxx/io.hpp>
#include <gpcxx/operator.hpp>
#include <gpcxx/stat.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <chrono>
#include <array>
#include <string_view>
#include <fstream>


int main( int argc , char *argv[] )
{
    using namespace ant_example;
    using rng_type = std::mt19937;
    std::random_device rd;
    rng_type rng( rd() );
    char const newl { '\n' };
    
    //[world_definition 
    int const max_steps { 400 };
    auto ant_sim_santa_fe = ant_example::AntBoardSimulation<santa_fe::x_size, santa_fe::y_size>{
            max_steps,
            89,
            ant_sim::Pos2d{0,0}, 
            ant_sim::Direction::east,
            [](int x, int y){ return santa_fe::board1[x][y] == 'X';}
        };
    //]

    //[node_generate
    gpcxx::uniform_symbol< node_type > terminal_gen { std::vector< node_type >{
        node_type { ant_move_task_terminal{} ,          "m" } ,
        node_type { ant_turn_left_task_terminal{} ,     "l" } ,
        node_type { ant_turn_right_task_terminal{} ,    "r" } 
    } };
    
    gpcxx::uniform_symbol< node_type > binary_gen { std::vector< node_type > {
        node_type { prog2{} , "p2" } ,
        node_type { if_food_ahead{} , "if" }
    } };
    
    gpcxx::uniform_symbol< node_type > ternary_gen { std::vector< node_type > {
        node_type { prog3{} , "p3" }
    } };
    
    gpcxx::node_generator< node_type , rng_type , 3 > node_generator {
        { double( terminal_gen.num_symbols() ) , 0 , terminal_gen } ,
        { double( binary_gen.num_symbols() ) , 2 , binary_gen } ,
        { double( ternary_gen.num_symbols() ) , 3 , ternary_gen } };
    //]
    
    //[envolve_settings
    size_t const population_size = 5000;
    size_t const generation_max = 5000;
    double const number_elite = 4;
    double const mutation_rate = 0.3;
    double const crossover_rate = 0.8;
    double const crossover_internal_point_rate = 0.9;
    double const reproduction_rate = 0.1;
    size_t const min_tree_height = 1;
    size_t const init_max_tree_height  = 6;
    size_t const max_tree_height = 17;
    size_t const tournament_size = 4;
    //]

 
    population_type population;

    //[tree_generator
    auto tree_generator      = gpcxx::make_basic_generate_strategy( rng , node_generator , min_tree_height , max_tree_height );
    auto init_tree_generator = gpcxx::make_basic_generate_strategy( rng , node_generator , min_tree_height , init_max_tree_height );
    
    {
        std::unordered_set<population_type::value_type> unique_init_population;

        auto const unique_collision_clearance_factor = 3;
        auto iteration = 0u;
        auto const max_iteration = unsigned(population_size * unique_collision_clearance_factor);
        while( unique_init_population.size() < population_size && iteration++ < max_iteration )
        {
            population_type::value_type individum;
            init_tree_generator( individum );
            unique_init_population.insert( std::move( individum ) );
        }
        if ( unique_init_population.size() != population_size )
            throw gpcxx::gpcxx_exception( "Could not create unique population with the requested size" );
        population = population_type{ begin( unique_init_population ) , end( unique_init_population ) };
    }
    //]
    
     //[evolver_definition
    using evolver_type = gpcxx::static_pipeline< population_type , fitness_type , rng_type > ;
    evolver_type evolver( number_elite , mutation_rate , crossover_rate , reproduction_rate , rng );

    evolver.mutation_function() = gpcxx::make_mutation(
         gpcxx::make_point_mutation( rng , tree_generator , max_tree_height , 20 ) ,
         gpcxx::make_tournament_selector( rng , tournament_size ) );
    
    evolver.crossover_function() = gpcxx::make_crossover( 
        gpcxx::make_one_point_crossover_pip_strategy( rng , max_tree_height, crossover_internal_point_rate ) ,
        gpcxx::make_tournament_selector( rng , tournament_size ) );
    
    evolver.reproduction_function() = gpcxx::make_reproduce( gpcxx::make_tournament_selector( rng , tournament_size ) );
    //]
    
    //[fitness_defintion
    evaluator       fitness_f;
    fitness_type    fitness( population_size , 0.0 );
    //]
    
    //[generation_loop
    gpcxx::timer overall_timer; 
    bool    has_optimal_fitness { false };
    size_t  generation { 0 };
    do
    {
        gpcxx::timer iteration_timer;
        if( generation != 0 )
            evolver.next_generation( population , fitness );
        double evolve_time { iteration_timer.seconds() };
        
        iteration_timer.restart();
        //[fitness_calculation
        std::transform( population.begin() , population.end() , fitness.begin() , [&]( tree_type const &t ) { 
            return fitness_f( t , ant_sim_santa_fe ); 
        } );
        //]
        double eval_timeGpcxx = iteration_timer.seconds();
        
        std::cout << gpcxx::indent( 0 ) << "Generation "    << generation << newl;
        std::cout << gpcxx::indent( 1 ) << "Evolve time "   << evolve_time << newl;
        std::cout << gpcxx::indent( 1 ) << "Eval time "     << eval_timeGpcxx << newl;
        std::cout << gpcxx::indent( 1 ) << "Best individuals\n" << gpcxx::best_individuals( population , fitness , 2 , 3 , false ) << newl;
        std::cout << gpcxx::indent( 1 ) << "Statistics : "      << gpcxx::calc_population_statistics( population ) << newl << std::endl;
        
        //[breakup_conditions
        generation++;
        has_optimal_fitness = ( 0 == *std::min_element( fitness.begin(), fitness.end() ) );
    } while( !has_optimal_fitness && generation < generation_max );
    //]
    //]
    
    std::cout << "Overall time : " << overall_timer.seconds() << newl;
    
//     auto fittest_individual_position = std::distance( fitness.begin(), std::min_element( fitness.begin(), fitness.end() ) ); 
//     tree_type const & fittest_individual = population[fittest_individual_position];
    
//     // cat artificial_ant_fittest_individual.dot | dot -Tsvg | display -
//     std::ofstream("artificial_ant_fittest_individual.dot") << gpcxx::graphviz( fittest_individual , false );
    
    return 0;
}

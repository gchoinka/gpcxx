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


namespace simple_ant
{
    
struct move;
struct right;
struct left;
struct if_food_ahead;
template<int nodeCount, typename CTString> struct prog;

using prog2 = prog<2, gpm::NodeToken<'p','2'>>;
using prog3 = prog<3, gpm::NodeToken<'p','3'>>;

using ant_nodes = boost::variant<
    boost::recursive_wrapper<move>, 
    boost::recursive_wrapper<left>, 
    boost::recursive_wrapper<right>, 
    boost::recursive_wrapper<if_food_ahead>, 
    boost::recursive_wrapper<prog2>, 
    boost::recursive_wrapper<prog3>
>;


template<int NodeCount, typename CTString>
struct prog : public gpm::BaseNode<ant_nodes, NodeCount, CTString>
{
    using prog::BaseNode::BaseNode;
};

struct move : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'m'>> {};

struct right : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'r'>>{};

struct left : public gpm::BaseNode<ant_nodes, 0, gpm::NodeToken<'l'>>{};

struct if_food_ahead : public gpm::BaseNode<ant_nodes, 2, gpm::NodeToken<'i', 'f'>>
{
    using if_food_ahead::BaseNode::BaseNode;

    constexpr ant_nodes const & get(bool b) const
    {
        return b ? get<true>() : get<false>();
    }
    
    template<bool c>
    constexpr ant_nodes const & get() const
    {
        return nodes[c ? 0 : 1];
    }
};



template<typename AntBoardSimType>
class AntBoardSimulationVisitor : public boost::static_visitor<void>
{
public:
    AntBoardSimulationVisitor(AntBoardSimType & sim):sim_{sim} {} 

    void operator()(move) const
    {
        sim_.move();
    }
    
    void operator()(left) const
    {
        sim_.left(); 
    }
    
    void operator()(right) const
    {
        sim_.right();
    }
    
    void operator()(if_food_ahead const & c) const
    {
        boost::apply_visitor( *this, c.get(sim_.is_food_in_front()));
    }

    template<int NodeCount, typename CTString>
    void operator()(prog<NodeCount, CTString> const & b) const
    {
        for(auto const & n: b.nodes)
            boost::apply_visitor( *this, n );
    }
    
    
private:
    AntBoardSimType & sim_;
};


}



int main( int argc , char *argv[] )
{
    
    //if( m , p2( r , p2( if( if( m , r ) , p3( l , l , if( m , r ) ) ) , m ) ) )
    {
        using namespace simple_ant;
        auto a = ant_nodes{ 
            if_food_ahead{ 
                move{}
                , prog2{ 
                    right{}, 
                    prog2{
                        if_food_ahead{
                            if_food_ahead{
                                move{}, 
                                right{}
                            },
                            prog3{
                                left{},
                                left{},
                                if_food_ahead{
                                    move{}, 
                                    right{}
                                }
                            },
                        },
                        move{}
                    }
                }
            }
        }; 
                
        char const * optAndRPNdef = "m r m if l l p3 r m if if p2 r p2 m if";
//         char const * def2 = "p3 l p2 if l if l p2 m l p2 if l p2 r m m m";
//         
//         try
//         {
//             auto a2 = gpm::factory<ant_nodes>(gpm::RPNToken_iterator{def});
//             auto a3 = gpm::factory<ant_nodes>(gpm::PNToken_iterator{def2});
//             std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, a2) << "\n";
//             std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, a3) << "\n";
//         }
//         catch(gpm::GPMException const & exp)
//         {
//             std::cerr << exp.what() << "\n";
//         }


        
        
        
        //std::cout << boost::apply_visitor(gpm::Printer<std::string>{}, a) << "\n";
        //std::cout << boost::apply_visitor(gpm::RPNPrinter<std::string>{}, a) << "\n";

        using namespace ant_example;
        board const b{ santa_fe::x_size, santa_fe::y_size };
        int const max_steps { 400 };


        auto optAnt = gpm::factory<ant_nodes>(gpm::RPNToken_iterator{optAndRPNdef});
        
        {
//             auto antBoardSimV1 = ant_simulation{ santa_fe_trail, b.get_size_x(), b.get_size_y(), { 0, 0 }, east, max_steps };
            auto antBoardSimulation = ant_example::AntBoardSimulation<santa_fe::x_size, santa_fe::y_size>{
                    max_steps,
                    89,
                    ant_sim::Pos2d{0,0}, 
                    ant_sim::Direction::east,
                    [](int x, int y){ return santa_fe::board1[x][y] == 'X';}
                };
            
            auto antBoardSimV1Visitor = simple_ant::AntBoardSimulationVisitor{antBoardSimulation};
            
            while(!antBoardSimulation.is_finish())
            {
                boost::apply_visitor(antBoardSimV1Visitor, optAnt);
     
                
                std::vector<std::string> lines;
                antBoardSimulation.get_board_as_str([&](std::string const & s){ lines.push_back(s); } );
//                 antBoardSimV2Visitor.get_board_as_str([iter = lines.begin()](std::string const & s) mutable{ 
//                     *iter = *iter + "                " + s; 
//                     ++iter; 
//                 });
                for(auto const & l: lines)
                    std::cout << l << "\n";

            }
        }

//         return 0;
    }
    
    using namespace ant_example;
    using rng_type = std::mt19937;
    std::random_device rd;
    rng_type rng( rd() );
    char const newl { '\n' };

    
    //rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
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
        //population = population_type{ std::make_move_iterator( begin( unique_init_population ) ) , std::make_move_iterator( end( unique_init_population ) ) };
        //std::make_move_iterator( begin( ... ) ) clang 3.6 does have an issue with this https://llvm.org/bugs/show_bug.cgi?id=19917
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
        
        std::vector<simple_ant::ant_nodes> populationGPM;
        std::transform( population.begin() , population.end() , std::back_inserter(populationGPM) , [&]( tree_type const &t ) { 
            return gpm::factory<simple_ant::ant_nodes>(gpm::PNToken_iterator{gpcxx::polish_string( t, " ")}); 
        } );
        
        std::vector<int> scoresGPM(populationGPM.size(), 0);
        iteration_timer.restart();

        std::transform( populationGPM.begin() , populationGPM.end() ,  scoresGPM.begin() , [&]( simple_ant::ant_nodes const &t ) mutable{ 
            auto antBoardSim = ant_example::AntBoardSimulation<santa_fe::x_size, santa_fe::y_size>{
                max_steps,
                89,
                ant_sim::Pos2d{0,0}, 
                ant_sim::Direction::east,
                [](int x, int y){ return santa_fe::board1[x][y] == 'X';}
            };
            auto antBoardSimulationVisitor = simple_ant::AntBoardSimulationVisitor{antBoardSim};
            while(!antBoardSim.is_finish())
            {
                boost::apply_visitor(antBoardSimulationVisitor, t);
            }
            return antBoardSim.score(); 
        } );
        
        double eval_timeGPM = iteration_timer.seconds();
        
//         std::vector<int> scoresGPM2(populationGPM.size(), 0);
//         iteration_timer.restart();
// 
//         std::transform( populationGPM.begin() , populationGPM.end() ,  scoresGPM2.begin() , []( simple_ant::ant_nodes const &t ){ 
//             auto calc = ant_example::AntBoardSimulation<santa_fe::x_size, santa_fe::y_size>{
//                 max_steps,
//                 89,
//                 ant_sim::Pos2d{0,0}, 
//                 ant_sim::Direction::east,
//                 [](int x, int y){ return santa_fe::board1[x][y] == 'X';}
//             };
//             while(!calc.is_finish())
//             {
//                 boost::apply_visitor(calc, t);
//             }
//             return calc.score(); 
//         } );
//         
//         double eval_timeGPM2 = iteration_timer.seconds();
//         {
//             std::ofstream fout( std::string(  "pop_" ) + std::to_string( generation ) );
//             std::vector< size_t > indices( population_size );
//             std::iota( indices.begin() , indices.end() , 0 );
//             std::sort( indices.begin() , indices.end() , [&]( size_t i , size_t j ) -> bool {
//                 return fitness[i] < fitness[j]; } );
//             
//             for( size_t j=0 ; j<population_size ; ++j )
//             {
//                 size_t i = indices[j];
//                 fout << j << " " << i << " "
//                      << fitness[i] << " " << population[i].root().height() << " "
//                      << gpcxx::simple( population[i] , false )
//                      << newl;
// //                 fout << j << " " << i << " "
// //                      << fitness[i] << " " << population[i].root().height() << " "
// //                      << gpcxx::polish_string( population[i], " ")
// //                      << newl;     
// //                 auto a42 = gpm::factory<simple_ant::ant_nodes>(PNToken_iterator{gpcxx::polish_string( population[i], " ")});
// //                 std::cout << "gpm " << boost::apply_visitor(gpm::Printer<std::string>{}, a42) << newl;
// //                 std::cout << "gpx " << gpcxx::simple( population[i] , false ) << newl << newl;
// //                 std::cout << "gpm " << boost::apply_visitor(gpm::PNPrinter<std::string>{}, a42) << newl;
// //                 std::cout << "gpx " << gpcxx::polish_string( population[i], " ") << newl;
// //                 
//             }
//         }
            
        
        
        
        std::cout << gpcxx::indent( 0 ) << "Generation "    << generation << newl;
        std::cout << gpcxx::indent( 1 ) << "Evolve time "   << evolve_time << newl;
        std::cout << gpcxx::indent( 1 ) << "Eval time "     << eval_timeGpcxx << newl;
        std::cout << gpcxx::indent( 1 ) << "Eval time "     << eval_timeGPM << newl;

        std::cout << gpcxx::indent( 1 ) << "Best individuals\n" << gpcxx::best_individuals( population , fitness , 2 , 3 , false ) << newl;
        std::cout << gpcxx::indent( 1 ) << "Best individuals\n" << gpcxx::best_individuals( population , scoresGPM , 2 , 3 , false ) << newl;
        std::cout << gpcxx::indent( 1 ) << "Statistics : "      << gpcxx::calc_population_statistics( population ) << newl << std::endl;
        
        //[breakup_conditions
        generation++;
        has_optimal_fitness = ( 0 == *std::min_element( fitness.begin(), fitness.end() ) );
    } while( !has_optimal_fitness && generation < generation_max );
    //]
    //]
    
    std::cout << "Overall time : " << overall_timer.seconds() << newl;
    
    auto fittest_individual_position = std::distance( fitness.begin(), std::min_element( fitness.begin(), fitness.end() ) ); 
    tree_type const & fittest_individual = population[fittest_individual_position];
    
    // cat artificial_ant_fittest_individual.dot | dot -Tsvg | display -
    std::ofstream("artificial_ant_fittest_individual.dot") << gpcxx::graphviz( fittest_individual , false );
    
    return 0;
}

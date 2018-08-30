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
#include <boost/variant.hpp>
#include <boost/mpl/for_each.hpp>
#include <array>
#include <string_view>

#include <fstream>


namespace genp
{
    class GenPException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    
    namespace detail
    {
        template<typename VariantType, typename Iter>
        VariantType factory_imp(Iter &);

        template<typename VariantType, typename Iter>
        using FactoryMap = std::unordered_map<std::string, std::function<VariantType(Iter &)>>;

        
        template<typename VariantType, typename Iter>
        struct FactoryMapInsertHelper 
        {
            FactoryMap<VariantType, Iter> & factoryMap; 
            
            template<class T> 
            void operator()(T) 
            {
                factoryMap[T::name] = [](Iter & tokenIter) 
                {
                    T ret;
                    for(auto & n: ret.nodes)
                        n = factory_imp<VariantType>(++tokenIter);
                    return ret; 
                };
            }
        };

        template<typename VariantType, typename Iter>
        FactoryMap<VariantType, Iter> makeFactoryMap()
        {
            FactoryMap<VariantType, Iter> factoryMap;
            auto insertHelper = FactoryMapInsertHelper<VariantType, Iter>{factoryMap};
            boost::mpl::for_each<typename VariantType::types>(insertHelper);
            return factoryMap;
        }

        template<typename VariantType, typename Iter>
        VariantType factory_imp(Iter & tokenIter)
        {
            static auto nodeCreateFunMap = makeFactoryMap<VariantType, Iter>();
            auto token = *tokenIter;
            if(!nodeCreateFunMap.count(token))
            {
                throw GenPException{std::string{"cant find factory function for token >>"} + token + "<<"};
            }
                
            return nodeCreateFunMap[token](tokenIter);
        }
    }


    template<typename VariantType, typename Iter>
    VariantType factory(Iter tokenIter)
    {
        return detail::factory_imp<VariantType>(tokenIter);
    }


    template<typename StringT>
    struct Printer : public boost::static_visitor<StringT>
    {    
        template<typename T>
        StringT operator()(T const & b) const
        {
            char const * delimiter = "";
            char const * begin_delimiter = "";
            char const * end_delimiter = "";
            StringT children;
            for(auto const & n: b.nodes)
            {
                children += delimiter + boost::apply_visitor( *this, n );
                delimiter = ", ";
                begin_delimiter = "(";
                end_delimiter = ")";
            }
            return StringT{T::name} + begin_delimiter + children + end_delimiter;
        }
    };

    template<typename StringT>
    struct RPNPrinter : public boost::static_visitor<StringT>
    {
        template<typename T>
        StringT operator()(T const & b) const
        {
            StringT children;
            for(auto const & n: b.nodes)
            {
                children = boost::apply_visitor( *this, n ) + " " + children;
            }
            return children + T::name;
        }
    };
    
    template<char ... ch> 
    struct NodeToken
    {
        constexpr static char name[] = {ch..., '\0'};
    };
    
    template<typename VariantType, int NodeCount, typename CTString>
    struct BaseNode : public CTString
    {
        template<typename ... Args>
        BaseNode(Args && ... args):nodes{std::forward<Args>(args)...}{}
            
        std::array<VariantType, NodeCount> nodes;
    };
}

namespace simple_ant
{
    
struct move;
struct right;
struct left;
struct if_food_ahead;
template<int nodeCount, typename CTString> struct prog;

using prog2 = prog<2, genp::NodeToken<'p','2'>>;
using prog3 = prog<3, genp::NodeToken<'p','3'>>;

using ant_nodes = boost::variant<
    boost::recursive_wrapper<move>, 
    boost::recursive_wrapper<left>, 
    boost::recursive_wrapper<right>, 
    boost::recursive_wrapper<if_food_ahead>, 
    boost::recursive_wrapper<prog2>, 
    boost::recursive_wrapper<prog3>
>;


template<int NodeCount, typename CTString>
struct prog : public genp::BaseNode<ant_nodes, NodeCount, CTString>
{
    using prog::BaseNode::BaseNode;
};

struct move : public genp::BaseNode<ant_nodes, 0, genp::NodeToken<'m'>> {};

struct right : public genp::BaseNode<ant_nodes, 0, genp::NodeToken<'r'>>{};

struct left : public genp::BaseNode<ant_nodes, 0, genp::NodeToken<'l'>>{};

struct if_food_ahead : public genp::BaseNode<ant_nodes, 2, genp::NodeToken<'?'>>
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



class calculator : public boost::static_visitor<void>
{
public:
    calculator(ant_example::ant_simulation & sim):sim_{sim} {} 

    void operator()(move) const
    {
        sim_.move();
    }
    
    void operator()(left) const
    {
        sim_.turn_left();
    }
    
    void operator()(right) const
    {
        sim_.turn_right();
    }
    
    void operator()(if_food_ahead const & c) const
    {
        boost::apply_visitor( *this, c.get(sim_.food_in_front()));
    }

    template<int NodeCount, typename CTString>
    void operator()(prog<NodeCount, CTString> const & b) const
    {
        for(auto const & n: b.nodes)
            boost::apply_visitor( *this, n );
    }
    
private:
    ant_example::ant_simulation & sim_;
        
};

}
//[ant_move_test
bool ant_move_test()
{
    size_t const board_size_x { 32 };
    size_t const board_size_y { 32 };
    ant_example::board  const the_board { board_size_x , board_size_y };
    ant_example::ant an_ant { the_board.pos_2d_to_1d( { 0, 0 } ), ant_example::east };
    
    
    an_ant.move(the_board);
    
    bool position_is_valid      = an_ant.pos() == the_board.pos_2d_to_1d( { 1, 0 } );
    bool step_count_is_valid    = an_ant.steps_done() == 1;
    
    
    return position_is_valid && step_count_is_valid ;
} 
//]


        class token_iterator
        {
            std::string_view sv_;
            std::string_view::const_iterator currentPos_;
            
        public:
            token_iterator(std::string_view sv):sv_{sv}
            {
                currentPos_ = sv_.end();
                --currentPos_;
                for(; currentPos_ > sv_.begin(); --currentPos_)
                {
                    if(*currentPos_ == ' ')
                    {
                        currentPos_++;
                        break;
                    }
                }
            }
            std::string operator*()
            {
                auto endIter = currentPos_ + 1;
                for(; endIter != sv_.end(); ++endIter)
                {
                    if(*endIter == ' ')
                    {
                        break;
                    }
                }
                return std::string{sv_.substr(currentPos_ - sv_.begin(), endIter - currentPos_)};
            }
            
            token_iterator& operator++()
            {
                
                --currentPos_ ;
                if(currentPos_ > sv_.begin())
                {
                    --currentPos_ ;
                    for(; currentPos_ > sv_.begin(); --currentPos_)
                    {
                        if(*currentPos_ == ' ')
                        {
                            ++currentPos_;
                            break;
                        }
                    }
                }
                return *this;
            }
            
        };


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
                
        char const * def = "m r m ? l l p3 r m ? ? p2 r p2 m ?";
        
        try
        {
            auto a2 = genp::factory<ant_nodes>(token_iterator{def});
            std::cout << boost::apply_visitor(genp::RPNPrinter<std::string>{}, a2) << "\n";
        }
        catch(genp::GenPException const & exp)
        {
            std::cerr << exp.what() << "\n";
        }


        
        
        
        std::cout << boost::apply_visitor(genp::Printer<std::string>{}, a) << "\n";
        std::cout << boost::apply_visitor(genp::RPNPrinter<std::string>{}, a) << "\n";

        using namespace ant_example;
        board const b{ santa_fe::x_size, santa_fe::y_size };
        int const max_steps { 400 };
        ant_simulation::food_trail_type santa_fe_trail { santa_fe::make_santa_fe_trail( b ) };
        ant_simulation                  ant_sim_santa_fe{ santa_fe_trail, b.get_size_x(), b.get_size_y(), { 0, 0 }, east, max_steps };
        std::cout << ant_sim_santa_fe.score() << "\n";
        
        while(!ant_sim_santa_fe.is_finsh())
        {
            boost::apply_visitor(calculator{ant_sim_santa_fe}, a);
        }
        std::cout << ant_sim_santa_fe.score() << "\n";
        return 0;
    }
    
    using namespace ant_example;
    using rng_type = std::mt19937;
    std::random_device rd;
    rng_type rng( rd() );
    char const newl { '\n' };

    
    rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
    //[world_definition 
    board const b{ santa_fe::x_size, santa_fe::y_size };
    int const max_steps { 400 };
    ant_simulation::food_trail_type santa_fe_trail { santa_fe::make_santa_fe_trail( b ) };
    ant_simulation                  ant_sim_santa_fe{ santa_fe_trail, b.get_size_x(), b.get_size_y(), { 0, 0 }, east, max_steps };
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
        
        
        {
            std::ofstream fout( std::string(  "pop_" ) + std::to_string( generation ) );
            std::vector< size_t > indices( population_size );
            std::iota( indices.begin() , indices.end() , 0 );
            std::sort( indices.begin() , indices.end() , [&]( size_t i , size_t j ) -> bool {
                return fitness[i] < fitness[j]; } );
            
            for( size_t j=0 ; j<population_size ; ++j )
            {
                size_t i = indices[j];
                fout << j << " " << i << " "
                     << fitness[i] << " " << population[i].root().height() << " "
                     << gpcxx::simple( population[i] , false )
                     << std::endl;
            }
        }
/*        
        {
            using namespace std;
            std::vector< size_t > idx;
            auto iter = gpcxx::sort_indices( fitness , idx );
            auto best = population[ idx[0] ];
            ant_simulation sim { santa_fe_trail, b.get_size_x(), b.get_size_y(), { 0, 0 }, east, max_steps };
            for( size_t i=0 ; i<100 ; ++i )
            {
                cout << simple( best , false ) << endl << endl;
                best.root()->eval( sim );
                cout << sim.get_board_as_str() << endl;
                usleep( 50 * 1000 );
            }
        }*/
            
        
        double eval_time = iteration_timer.seconds();
        
        std::cout << gpcxx::indent( 0 ) << "Generation "    << generation << newl;
        std::cout << gpcxx::indent( 1 ) << "Evolve time "   << evolve_time << newl;
        std::cout << gpcxx::indent( 1 ) << "Eval time "     << eval_time << newl;
        std::cout << gpcxx::indent( 1 ) << "Best individuals\n" << gpcxx::best_individuals( population , fitness , 2 , 3 , false ) << newl;
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

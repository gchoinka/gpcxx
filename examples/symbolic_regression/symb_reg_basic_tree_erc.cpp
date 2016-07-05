/*
 * main_optimize.cpp
 * Date: 2013-01-28
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 */

#define FUSION_MAX_VECTOR_SIZE 20

#include <gpcxx/tree.hpp>
#include <gpcxx/generate.hpp>
#include <gpcxx/operator.hpp>
#include <gpcxx/eval.hpp>
#include <gpcxx/evolve.hpp>
#include <gpcxx/io.hpp>
#include <gpcxx/stat.hpp>
#include <gpcxx/app.hpp>

#include <boost/fusion/include/make_vector.hpp>

#include <iostream>
#include <random>
#include <vector>
#include <functional>


const std::string tab = "\t";


namespace pl = std::placeholders;
namespace fusion = boost::fusion;

int main( int argc , char *argv[] )
{
    typedef std::mt19937 rng_type ;
    typedef double value_type;
    typedef char symbol_type;
    typedef gpcxx::regression_training_data< value_type , 3 > training_data_type;
    typedef gpcxx::regression_context< value_type , 3 > context_type;
    typedef std::vector< value_type > fitness_type;

    auto eval = gpcxx::make_static_eval_erc< value_type , symbol_type , context_type >(
        fusion::make_vector( 1.0 , std::normal_distribution<>( 0.0 , 1.0 ) ) ,
        fusion::make_vector(
            fusion::make_vector( 'x' , []( context_type const& t ) { return t[0]; } )
          , fusion::make_vector( 'y' , []( context_type const& t ) { return t[1]; } )
          , fusion::make_vector( 'z' , []( context_type const& t ) { return t[2]; } )          
          ) ,
        fusion::make_vector(
            fusion::make_vector( 's' , []( double v ) -> double { return std::sin( v ); } )
          , fusion::make_vector( 'c' , []( double v ) -> double { return std::cos( v ); } ) 
          ) ,
        fusion::make_vector(
            fusion::make_vector( '+' , std::plus< double >() )
          , fusion::make_vector( '-' , std::minus< double >() )
          , fusion::make_vector( '*' , std::multiplies< double >() ) 
          , fusion::make_vector( '/' , std::divides< double >() ) 
          ) );
    typedef decltype( eval ) eval_type;
    typedef eval_type::node_attribute_type node_attribute_type;
    
    typedef gpcxx::basic_tree< node_attribute_type > tree_type;
    typedef std::vector< tree_type > population_type;
    typedef gpcxx::static_pipeline< population_type , fitness_type , rng_type > evolver_type;
    
    rng_type rng;

    training_data_type c;
    gpcxx::generate_regression_test_data( c , 1024 , rng , []( double x1 , double x2 , double x3 )
                    { return  x1 * x1 * x1 + 1.0 / 10.0 * x2 * x2 - 3.0 / 4.0 * ( x3 - 4.0 ) + 1.0 ; } );


    
    size_t population_size = 512;
    size_t number_elite = 1;
    double mutation_rate = 0.2;
    double crossover_rate = 0.6;
    double reproduction_rate = 0.3;
    size_t min_tree_height = 8 , max_tree_height = 8;

    auto terminal_gen = eval.get_terminal_symbol_distribution();
    auto unary_gen = eval.get_unary_symbol_distribution();
    auto binary_gen = eval.get_binary_symbol_distribution();
    std::array< double , 3 > weights = {{ 2.0 * double( terminal_gen.num_symbols() ) ,
                                       double( unary_gen.num_symbols() ) ,
                                       double( binary_gen.num_symbols() ) }};
    auto tree_generator = gpcxx::make_ramp( rng , terminal_gen , unary_gen , binary_gen , min_tree_height , max_tree_height , 0.5 , weights );
    

    evolver_type evolver( number_elite , mutation_rate , crossover_rate , reproduction_rate , rng );
    fitness_type fitness( population_size , 0.0 );
    population_type population( population_size );


    auto fitness_f = gpcxx::regression_fitness< eval_type >( eval );
    evolver.mutation_function() = gpcxx::make_mutation(
        gpcxx::make_simple_mutation_strategy( rng , terminal_gen , unary_gen , binary_gen ) ,
        gpcxx::make_random_selector( rng ) );
    evolver.crossover_function() = gpcxx::make_crossover( 
        gpcxx::make_one_point_crossover_strategy( rng , 10 ) ,
        gpcxx::make_random_selector( rng ) );
    evolver.reproduction_function() = gpcxx::make_reproduce( gpcxx::make_random_selector( rng ) );


    // initialize population with random trees and evaluate fitness
    for( size_t i=0 ; i<population.size() ; ++i )
    {
        tree_generator( population[i] );
        fitness[i] = fitness_f( population[i] , c );
    }
    std::cout << "Best individuals" << std::endl << gpcxx::best_individuals( population , fitness ) << std::endl;
    std::cout << "Statistics : " << gpcxx::calc_population_statistics( population ) << std::endl;
    std::cout << std::endl << std::endl;

    for( size_t i=0 ; i<100 ; ++i )
    {
        evolver.next_generation( population , fitness );
        for( size_t i=0 ; i<population.size() ; ++i )
            fitness[i] = fitness_f( population[i] , c );
        
        std::cout << "Iteration " << i << std::endl;
        std::cout << "Best individuals" << std::endl << gpcxx::best_individuals( population , fitness , 1 ) << std::endl;
        std::cout << "Statistics : " << gpcxx::calc_population_statistics( population ) << std::endl << std::endl;
    }

    return 0;
}

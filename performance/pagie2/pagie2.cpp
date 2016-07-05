/*
 * main_optimize.cpp
 * Date: 2013-01-28
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 */

#define FUSION_MAX_VECTOR_SIZE 20

#include <gpcxx/tree/basic_tree.hpp>
#include <gpcxx/generate/uniform_symbol.hpp>
#include <gpcxx/generate/node_generator.hpp>
#include <gpcxx/generate/ramp.hpp>
#include <gpcxx/operator/mutation.hpp>
#include <gpcxx/operator/simple_mutation_strategy.hpp>
#include <gpcxx/operator/random_selector.hpp>
#include <gpcxx/operator/tournament_selector.hpp>
#include <gpcxx/operator/crossover.hpp>
#include <gpcxx/operator/one_point_crossover_strategy.hpp>
#include <gpcxx/operator/reproduce.hpp>
#include <gpcxx/eval/static_eval.hpp>
#include <gpcxx/eval/regression_fitness.hpp>
#include <gpcxx/evolve/static_pipeline.hpp>
#include <gpcxx/io/best_individuals.hpp>
#include <gpcxx/stat/population_statistics.hpp>
#include <gpcxx/app/timer.hpp>
#include <gpcxx/app/normalize.hpp>
#include <gpcxx/app/generate_evenly_spaced_test_data.hpp>

#include <boost/fusion/include/make_vector.hpp>

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <functional>

const std::string tab = "\t";

namespace fusion = boost::fusion;

typedef double value_type;
typedef gpcxx::regression_training_data< value_type , 3 > trainings_data_type;
typedef std::mt19937 rng_type ;
typedef char symbol_type;
typedef std::array< value_type , 3 > eval_context_type;
typedef std::vector< value_type > fitness_type;






namespace pl = std::placeholders;


int main( int argc , char *argv[] )
{
    rng_type rng;

    trainings_data_type c = gpcxx::generate_evenly_spaced_test_data< 3 >( -5.0 , 5.0 + 0.1 , 0.4 , []( double x1 , double x2 , double x3 ) {
                        return  1.0 / ( 1.0 + pow( x1 , -4.0 ) ) + 1.0 / ( 1.0 + pow( x2 , -4.0 ) ) + 1.0 / ( 1.0 + pow( x3 , -4.0 ) ); } );
    gpcxx::normalize( c.y );
    

    std::ofstream fout1( "testdata.dat" );
    for( size_t i=0 ; i<c.x[0].size() ; ++i )
        fout1 << c.y[i] << " " << c.x[0][i] << " " << c.x[1][i] << " " << c.x[2][i] << "\n";
    fout1.close();
    
    auto eval = gpcxx::make_static_eval< value_type , symbol_type , eval_context_type >(
        fusion::make_vector(
            fusion::make_vector( 'x' , []( eval_context_type const& t ) { return t[0]; } )
          , fusion::make_vector( 'y' , []( eval_context_type const& t ) { return t[1]; } )
          , fusion::make_vector( 'z' , []( eval_context_type const& t ) { return t[2]; } )          
          ) ,
        fusion::make_vector(
            fusion::make_vector( 's' , []( double v ) -> double { return std::sin( v ); } )
          , fusion::make_vector( 'c' , []( double v ) -> double { return std::cos( v ); } ) 
          , fusion::make_vector( 'e' , []( double v ) -> double { return std::exp( v ); } ) 
          , fusion::make_vector( 'l' , []( double v ) -> double { return ( std::abs( v ) < 1.0e-20 ) ? log( 1.0e-20 ) : std::log( std::abs( v ) ); } ) 
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

    
    size_t population_size = 1000;
    size_t generation_size = 20;
    double number_elite = 1;
    double mutation_rate = 0.0;
    double crossover_rate = 0.6;
    double reproduction_rate = 0.3;
    size_t min_tree_height = 8 , max_tree_height = 8;
    size_t tournament_size = 15;


    // generators< rng_type > gen( rng );
    auto terminal_gen = eval.get_terminal_symbol_distribution();
    auto unary_gen = eval.get_unary_symbol_distribution();
    auto binary_gen = eval.get_binary_symbol_distribution();
    gpcxx::node_generator< node_attribute_type , rng_type , 3 > node_generator {
        { 2.0 * double( terminal_gen.num_symbols() ) , 0 , terminal_gen } ,
        { double( unary_gen.num_symbols() ) , 1 , unary_gen } ,
        { double( binary_gen.num_symbols() ) , 2 , binary_gen } };

    auto tree_generator = gpcxx::make_ramp( rng , node_generator , min_tree_height , max_tree_height , 0.5 );
    

    evolver_type evolver( number_elite , mutation_rate , crossover_rate , reproduction_rate , rng );
    std::vector< double > fitness( population_size , 0.0 );
    std::vector< tree_type > population( population_size );


    auto fitness_f = gpcxx::regression_fitness< eval_type >( eval );
    evolver.mutation_function() = gpcxx::make_mutation(
        gpcxx::make_simple_mutation_strategy( rng , node_generator ) ,
        gpcxx::make_tournament_selector( rng , tournament_size ) );
    evolver.crossover_function() = gpcxx::make_crossover( 
        gpcxx::make_one_point_crossover_strategy( rng , max_tree_height ) ,
        gpcxx::make_tournament_selector( rng , tournament_size ) );
    evolver.reproduction_function() = gpcxx::make_reproduce( gpcxx::make_tournament_selector( rng , tournament_size ) );
    
    gpcxx::timer timer;



    // initialize population with random trees and evaluate fitness
    timer.restart();
    for( size_t i=0 ; i<population.size() ; ++i )
    {
        tree_generator( population[i] );
        fitness[i] = fitness_f( population[i] , c );
    }
    std::cout << gpcxx::indent( 0 ) << "Generation time " << timer.seconds() << std::endl;
    std::cout << gpcxx::indent( 1 ) << "Best individuals" << std::endl << gpcxx::best_individuals( population , fitness , 1 , 10 ) << std::endl;
    std::cout << gpcxx::indent( 1 ) << "Statistics : " << gpcxx::calc_population_statistics( population ) << std::endl;
    std::cout << gpcxx::indent( 1 ) << std::endl << std::endl;

    timer.restart();
    for( size_t generation=1 ; generation<=generation_size ; ++generation )
    {
        gpcxx::timer iteration_timer;
        iteration_timer.restart();
        evolver.next_generation( population , fitness );
        double evolve_time = iteration_timer.seconds();
        iteration_timer.restart();
        std::transform( population.begin() , population.end() , fitness.begin() , [&]( tree_type const &t ) { return fitness_f( t , c ); } );
        double eval_time = iteration_timer.seconds();
        
        std::cout << gpcxx::indent( 0 ) << "Generation " << generation << std::endl;
        std::cout << gpcxx::indent( 1 ) << "Evolve time " << evolve_time << std::endl;
        std::cout << gpcxx::indent( 1 ) << "Eval time " << eval_time << std::endl;
        std::cout << gpcxx::indent( 1 ) << "Best individuals" << std::endl << gpcxx::best_individuals( population , fitness , 2 , 10 ) << std::endl;
        std::cout << gpcxx::indent( 1 ) << "Statistics : " << gpcxx::calc_population_statistics( population ) << std::endl << std::endl;
    }
    std::cout << "Overall time : " << timer.seconds() << std::endl;

    return 0;
}

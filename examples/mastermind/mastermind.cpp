/*
 * mastermind.cpp
 * Date: 2014-05-23
 * Author: Gerard Choinka (gerard.choinka@gmail.com)
 */

#define FUSION_MAX_VECTOR_SIZE 20

#include <gpcxx/tree/intrusive_tree.hpp>
#include <gpcxx/tree/basic_intrusive_node.hpp>
#include <gpcxx/tree/basic_named_intrusive_node.hpp>
#include <gpcxx/tree/intrusive_func.hpp>

#include <gpcxx/generate/uniform_symbol.hpp>
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


#include <boost/random/linear_congruential.hpp>

#include <map>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <functional>
#include <fstream>
#include <bitset>
#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <initializer_list>
char const tab = '\t';
char const newl = '\n';


using boost::format;
using boost::str;


namespace gpcxx {
    template<size_t ELEMENT>
    struct element_terminal
    {
        template< typename Context , typename Node >
        typename Node::result_type operator()( Context const& c , Node const& node ) const
        {
            typedef typename Node::result_type result_type;
            result_type bs; 
            bs.set(ELEMENT);
            return bs;
        }
    }; 
    
    template<class T>
    T ror(T x, unsigned int moves)
    {
        return (x >> moves) | (x << sizeof(T)*8 - moves);
    }
    
    template<size_t SHIFT>
    struct unary_rrot_func
    {
        template< typename Context , typename Node >
        typename Node::result_type operator()( Context const& c , Node const& node ) const
        {
            return ror( node.children( 0 )->eval( c ) , SHIFT );
        }
    }; 
        
    struct binary_or_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return node.children( 0 )->eval( c ) | node.children( 1 )->eval( c );                        
        }                                                                                                 
    };
    
    
    struct binary_and_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return node.children( 0 )->eval( c ) & node.children( 1 )->eval( c );                        
        }                                                                                                 
    };
    
    struct binary_xor_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return node.children( 0 )->eval( c ) ^ node.children( 1 )->eval( c );                        
        }                                                                                                 
    };
    
    struct binary_nand_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return ~( node.children( 0 )->eval( c ) & node.children( 1 )->eval( c ) );                        
        }                                                                                                 
    };
    
    struct binary_nor_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return ~( node.children( 0 )->eval( c ) | node.children( 1 )->eval( c ) );                        
        }                                                                                                 
    };
    
    struct binary_nxor_func                                                                               
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return ~( node.children( 0 )->eval( c ) ^ node.children( 1 )->eval( c ) );                        
        }                                                                                                 
    };
    
    struct unary_inv_func                                                                     
    {                                                                                                     
        template< typename Context , typename Node >                                                      
        inline typename Node::result_type operator()( Context const& c , Node const& node ) const         
        {                                                                                                 
            return ~( node.children( 0 )->eval( c ) );                                                 
        }                                                                                                 
    };
    
    template< typename Value = double >
    struct terminal
    {     
        template< typename Context , typename Node >
        typename Node::result_type operator()( Context const& c , Node const& node ) const
        {
            typedef typename Node::result_type result_type;        
            return static_cast< result_type >( c[0] );
        }
    };
    
    template< typename Value >
    terminal<Value> make_terminal(const Value & v)
    {
        return terminal<Value> ();
    }
    
}



template<size_t nslots_, size_t ncolours_>
class MasterMindRow 
{
public:
    typedef size_t color_t;

    static size_t const nslots   = nslots_;
    static size_t const ncolours = ncolours_;
    static size_t const data_size = nslots_ * 2 + nslots_ * ncolours_;
    static color_t const defaultColor = 0;
    
    typedef std::bitset<data_size> data_type;

    MasterMindRow()
    {
        for(size_t i = 0; i < nslots; ++i)
            color(i, defaultColor);
    }
    
    MasterMindRow(std::initializer_list<color_t> entries)
    {
        BOOST_VERIFY( nslots_ == entries.size() );
        size_t index = 0;
        for(color_t c : entries )
            color(index++, c);            
    }
    
    
    void color(size_t pos, color_t colorId)
    {
        BOOST_VERIFY( pos < nslots );
        BOOST_VERIFY( colorId < ncolours );
        
        for(int i = 0; i < ncolours_; ++i)
            data_.reset( nslots_ * 2 + (pos * ncolours_) + i );
        
        size_t const posOfColor = (nslots_ * 2) + (pos * ncolours_) +  colorId;
        data_.set( posOfColor );
    }
   
    color_t color(size_t pos) const 
    {
        for(int i = 0; i < ncolours_; ++i)
            if(data_.test( nslots_ * 2 + (pos * ncolours_) + i ) )
                return i;
        BOOST_VERIFY( false );
        return ncolours; //no color ist set
    }
        
    std::string toStringFormated() const 
    {
        std::string s;
        for(int i = 0; i < nslots; ++i)
            s.append(data_.test( i ) ? "1" : "0");     
        s.append("'");
        for(int i = nslots; i < (nslots*2); ++i)
            s.append(data_.test( i ) ? "1" : "0");  
        
        for(int i = (nslots*2); i < (nslots_ * 2 + nslots_ * ncolours_ ); ++i)
        {
            if(((i-(nslots_ * 2))%ncolours_) == 0)
                s.append("'");
            s.append(data_.test( i ) ? "1" : "0");  

        }
        
        std::reverse( s.begin(), s.end() );  
        return s;
    }
    

    void compareTo(const MasterMindRow<nslots_, ncolours_> & other)
    {
        size_t colorPosHits = 0;
        for(size_t i = 0; i < nslots_; ++i)
        {
            if(color(i) == other.color(i))
                colorPosHits++;
        }
        
        size_t countColors[ncolours] = {};
        size_t countColorsOther[ncolours] = {};
        for(size_t i = 0; i < nslots; ++i)
        {
            countColors[color(i)]++;
            countColorsOther[other.color(i)]++;
        }
              
        size_t colorHits = 0;
        for(size_t i = 0; i < ncolours; ++i)
        {
            colorHits += std::min(countColors[i], countColorsOther[i]);
        }
            
       
        colorHits -= colorPosHits;
        
        setHits(colorPosHits, colorHits);
    }
    
    size_t posAndColorHits() const
    {
        size_t c = 0;
        for(size_t i = (nslots*0); i < (nslots*1); ++i)
            c += data_.test( i ) ? 1 : 0;     
        return c;
    }
    
    size_t colorHits() const
    {
        size_t c = 0;
        for(size_t i = (nslots*1); i < (nslots*2); ++i)
            c += data_.test( i ) ? 1 : 0;    
        return c;
    }
    
    data_type data()
    {
        return data_;
    }
    
    int score() const
    {
        int pos_color_hit_factor = 10;
        int max_score = nslots * pos_color_hit_factor;
        return max_score - (colorHits() + posAndColorHits() * pos_color_hit_factor);
    }
    
private:
    
    void setHits(size_t posAndColorHit, size_t colorHit)
    {
        BOOST_VERIFY( posAndColorHit <= nslots );
        BOOST_VERIFY( colorHit <= nslots );
        BOOST_VERIFY( ( colorHit + posAndColorHit ) <= nslots );
        
        for(size_t i = 0; i < (nslots*2); ++i)
            data_.reset( i );
        
        for(size_t i = 0; i < posAndColorHit; ++i)
            data_.set( nslots_ * 0 + i );
        
        for(size_t i = 0; i < colorHit; ++i)
            data_.set( nslots_ * 1 + i );
    }
    
    size_t countByColor(color_t colorId) const
    {
        size_t colorCount = 0;
        for(size_t i = 0; i < nslots; ++i)
        {
            if( data_.test( (nslots_ * 2) + (i * ncolours_) +  colorId ) )
                colorCount++;
        }
        return colorCount;
    }
    
    size_t searchColor(color_t colorId, size_t startPos = 0) const
    {
        BOOST_VERIFY( startPos < nslots );
        for(size_t i = startPos; i < nslots; ++i)
        {
            if( data_.test( (nslots_ * 2) + (i * ncolours_) +  colorId ) )
                return i;
        }
        return nslots;
    }
    
    bool hasColor(color_t colorId) const 
    {
        return countByColor(colorId) != 0;
    }
    

    
private:
    data_type data_;
};
    
    

template<typename MMRowT, typename RandGenT>
void fillMMRowWithColor(MMRowT & mMRow, RandGenT & randomGen)
{
    for(size_t i = 0; i < MMRowT::nslots; ++i)
        mMRow.color(i, randomGen() % MMRowT::ncolours);
} 


bool test();

template<typename trainings_data_type, typename rnd_type, typename mmrow_type>
void generate_test_data( trainings_data_type &data, size_t makeN, mmrow_type secred_mmrow, rnd_type random_generator)
{
    data.x[0].clear();
    data.y.clear();
    
    for(size_t i = 0; i < makeN; ++i)
    {
        mmrow_type rnd_mmrow;
        fillMMRowWithColor(rnd_mmrow, random_generator);
        rnd_mmrow.compareTo( secred_mmrow );
        data.x[0].push_back( rnd_mmrow.data() );
        data.y.push_back( rnd_mmrow.score() );
    }
}


int main( int argc , char *argv[] )
{
    test();
       
    size_t const nslots = 4;
    size_t const ncolours = 6;
    typedef MasterMindRow< nslots, ncolours > default_mastermind_row_t;
   // size_t const terminals_needed = MasterMindRow< nslots, ncolours >::data_size ;
    
    size_t const nterminals = 1;
    
    typedef typename  default_mastermind_row_t::data_type value_type;
    typedef gpcxx::regression_training_data< value_type , nterminals > trainings_data_type;
    typedef std::mt19937 rng_type ;
    typedef std::array< value_type , nterminals > eval_context_type;
    typedef std::vector< double > fitness_type;
    typedef gpcxx::basic_named_intrusive_node< value_type , eval_context_type > node_type;
    typedef gpcxx::intrusive_tree< node_type > tree_type;
    typedef std::vector< tree_type > population_type;
//     
    
    size_t const initPopulationSize = 128;

    int const seed = 100;
    rng_type rnd (seed);

    default_mastermind_row_t tosearch{0,1,2,3};   
    
     trainings_data_type c;
     generate_test_data( c , initPopulationSize,  tosearch, rnd);
 
     typedef gpcxx::static_pipeline< population_type , fitness_type , rng_type > evolver_type;
     
     std::vector< node_type > terminals {
         node_type { gpcxx::terminal< value_type >{  } ,  "r" } 
     };


      
    gpcxx::uniform_symbol< node_type > terminal_gen { terminals };
    
    gpcxx::uniform_symbol< node_type > binary_gen 
    { 
        std::vector< node_type >{
            node_type { gpcxx::binary_or_func{} ,   "|"  } ,
            node_type { gpcxx::binary_and_func{} ,  "&"  } ,
            node_type { gpcxx::binary_xor_func{} ,  "^"  } ,
            node_type { gpcxx::binary_nor_func{} ,  "!|" } ,
            node_type { gpcxx::binary_nand_func{} , "!&" } ,
            node_type { gpcxx::binary_nxor_func{} , "!^" }            
        }       
    };
    gpcxx::uniform_symbol< node_type > unary_gen 
    { 
        std::vector< node_type >{
            node_type { gpcxx::unary_inv_func{} , "~" } ,
            node_type { gpcxx::unary_rrot_func<  0 >{} , ">0>" } ,
            node_type { gpcxx::unary_rrot_func<  1 >{} , ">1>" } , 
            node_type { gpcxx::unary_rrot_func<  2 >{} , ">2>" } ,
            node_type { gpcxx::unary_rrot_func<  3 >{} , ">3>" } ,
            node_type { gpcxx::unary_rrot_func<  4 >{} , ">4>" } ,
            node_type { gpcxx::unary_rrot_func<  5 >{} , ">5>" } ,
            node_type { gpcxx::unary_rrot_func<  6 >{} , ">6>" } ,
            node_type { gpcxx::unary_rrot_func<  7 >{} , ">7>" } ,
            node_type { gpcxx::unary_rrot_func<  8 >{} , ">8>" } ,
            node_type { gpcxx::unary_rrot_func<  9 >{} , ">9>" } ,
            node_type { gpcxx::unary_rrot_func< 10 >{} , ">10>" } ,
            node_type { gpcxx::unary_rrot_func< 11 >{} , ">11>" } ,
            node_type { gpcxx::unary_rrot_func< 12 >{} , ">12>" } ,
            node_type { gpcxx::unary_rrot_func< 13 >{} , ">13>" } ,
            node_type { gpcxx::unary_rrot_func< 14 >{} , ">14>" } ,
            node_type { gpcxx::unary_rrot_func< 15 >{} , ">15>" } ,
            node_type { gpcxx::unary_rrot_func< 16 >{} , ">16>" } ,
            node_type { gpcxx::unary_rrot_func< 17 >{} , ">17>" } ,
            node_type { gpcxx::unary_rrot_func< 18 >{} , ">18>" } ,
            node_type { gpcxx::unary_rrot_func< 19 >{} , ">19>" } ,
            node_type { gpcxx::unary_rrot_func< 20 >{} , ">20>" } ,
            node_type { gpcxx::unary_rrot_func< 21 >{} , ">21>" } ,
            node_type { gpcxx::unary_rrot_func< 22 >{} , ">22>" } ,
            node_type { gpcxx::unary_rrot_func< 23 >{} , ">23>" } ,
            node_type { gpcxx::unary_rrot_func< 24 >{} , ">24>" } ,
            node_type { gpcxx::unary_rrot_func< 25 >{} , ">25>" } ,
            node_type { gpcxx::unary_rrot_func< 26 >{} , ">26>" } ,
            node_type { gpcxx::unary_rrot_func< 27 >{} , ">27>" } ,
            node_type { gpcxx::unary_rrot_func< 28 >{} , ">28>" } ,
            node_type { gpcxx::unary_rrot_func< 29 >{} , ">29>" } ,
            node_type { gpcxx::unary_rrot_func< 30 >{} , ">30>" } ,
            node_type { gpcxx::unary_rrot_func< 31 >{} , ">31>" } ,
            node_type { gpcxx::unary_rrot_func< 32 >{} , ">32>" } 
            
            
        }
    };  
    
    size_t population_size = 128;
    size_t generation_size = 20;
    double number_elite = 1;
    double mutation_rate = 0.0;
    double crossover_rate = 0.6;
    double reproduction_rate = 0.3;
    size_t min_tree_height = 1; 
    size_t max_tree_height = 64;
    size_t tournament_size = 15;
    
    std::array< double , 3 > weights = {{ double( terminal_gen.num_symbols() ) ,
                                          double( unary_gen.num_symbols() ) ,
                                          double( binary_gen.num_symbols() ) }};
    
    auto tree_generator = gpcxx::make_basic_generate_strategy( rnd , terminal_gen , unary_gen , binary_gen , max_tree_height , max_tree_height , weights );
    evolver_type evolver( number_elite , mutation_rate , crossover_rate , reproduction_rate , rnd );
    std::vector< double > fitness( population_size , 0.0 );
    std::vector< tree_type > population( population_size );
    
    evolver.mutation_function() = gpcxx::make_mutation(
        gpcxx::make_simple_mutation_strategy( rnd , terminal_gen , unary_gen , binary_gen ) ,
        gpcxx::make_tournament_selector( rnd , tournament_size ) 
    );
    
    evolver.crossover_function() = gpcxx::make_crossover( 
        gpcxx::make_one_point_crossover_strategy( rnd , max_tree_height ) ,
        gpcxx::make_tournament_selector( rnd , tournament_size ) 
    );
    evolver.reproduction_function() = gpcxx::make_reproduce( 
        gpcxx::make_tournament_selector( rnd , tournament_size ) 
    );
//     
//     gpcxx::timer timer;
//     auto fitness_f = gpcxx::make_regression_fitness( evaluator() );
//     
//     // initialize population with random trees and evaluate fitness
//     timer.restart();
//     for( size_t i=0 ; i<population.size() ; ++i )
//     {
//         tree_generator( population[i] );
//         fitness[i] = fitness_f( population[i] , c );
//     }
//     std::cout << gpcxx::indent( 0 ) << "Generation time " << timer.seconds() << std::endl;
//     std::cout << gpcxx::indent( 1 ) << "Best individuals" << std::endl << gpcxx::best_individuals( population , fitness , 1 , 10 ) << std::endl;
//     std::cout << gpcxx::indent( 1 ) << "Statistics : " << gpcxx::calc_population_statistics( population ) << std::endl;
//     std::cout << gpcxx::indent( 1 ) << std::endl << std::endl;
}


bool test()
{
    size_t const nslots = 4;
    size_t const ncolours = 6;
    int const seed = 100;
    boost::rand48 rd (seed);
    MasterMindRow< nslots, ncolours > colorSecret;
    MasterMindRow< nslots, ncolours > colorTest;   
    MasterMindRow< nslots, ncolours > colorTest2{0,1,2,3};   

    fillMMRowWithColor(colorSecret, rd);
    fillMMRowWithColor(colorTest, rd);
    
    colorTest.compareTo(colorSecret);
    std::cout << "test " << colorTest.toStringFormated()   << 
        tab << colorTest.posAndColorHits() << 
        tab << colorTest.colorHits() << newl;   
    std::cout << "secr " << colorSecret.toStringFormated() << 
        tab << colorSecret.posAndColorHits() << 
        tab << colorSecret.colorHits() << newl; 
    std::cout << "secr " << colorTest2.toStringFormated() << 
        tab << colorTest2.posAndColorHits() << 
        tab << colorTest2.colorHits() << newl;
        
    {
        MasterMindRow< nslots, ncolours > colorSecret{0,0,0,0};
        MasterMindRow< nslots, ncolours > colorTest{0,0,0,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 0);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 4);
    }
    {
        MasterMindRow< nslots, ncolours > colorSecret{0,0,0,0};
        MasterMindRow< nslots, ncolours > colorTest  {1,1,0,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 0);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 2);
    }
    {
        MasterMindRow< nslots, ncolours > colorSecret{0,0,0,4};
        MasterMindRow< nslots, ncolours > colorTest  {1,1,3,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 1);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 0);
    }

}
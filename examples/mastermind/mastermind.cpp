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
#include <initializer_list>
char const tab = '\t';
char const newl = '\n';


size_t const nSlots = 4;
size_t const nColours = 6;

size_t const terminalsNeeded = 2 * nSlots + nSlots * nColours;
typedef bool value_type;
typedef gpcxx::regression_training_data< value_type , terminalsNeeded > trainings_data_type;

typedef std::mt19937 rng_type ;
typedef std::bitset< terminalsNeeded > eval_context_type;
typedef std::bitset< terminalsNeeded > fitness_type;

typedef gpcxx::basic_named_intrusive_node< value_type , eval_context_type > node_type;
typedef gpcxx::intrusive_tree< node_type > tree_type;
typedef std::vector< tree_type > population_type;





template<size_t nSlots_, size_t nColours_>
class MasterMindRow 
{
public:
    typedef size_t color_t;
    static size_t const nSlots         = nSlots_;
    static size_t const nColours       = nColours_;
    static color_t const defaultColor   = 0;
    
    MasterMindRow()
    {
        for(size_t i = 0; i < nSlots; ++i)
            color(i, defaultColor);
        
    }
    
    MasterMindRow(std::initializer_list<color_t> entries)
    {
        BOOST_VERIFY( nSlots_ == entries.size() );
        size_t index = 0;
        for(color_t c : entries )
            color(index++, c);
            
    }
    
    
    void color(size_t pos, color_t colorId)
    {
        BOOST_VERIFY( pos < nSlots );
        BOOST_VERIFY( colorId < nColours );
        
        for(int i = 0; i < nColours_; ++i)
            bitrep.reset(nSlots_ * 2 + (pos * nColours_) + i);
        
        size_t const posOfColor = (nSlots_ * 2) + (pos * nColours_) +  colorId;
        bitrep.set( posOfColor );
    }
   
    color_t color(size_t pos) const 
    {
        for(int i = 0; i < nColours_; ++i)
            if(bitrep.test(nSlots_ * 2 + (pos * nColours_) + i))
                return i;
        BOOST_VERIFY( false );
        return nColours; //no color ist set
    }
    
    std::string toString() const 
    {
        return bitrep.to_string();
    }
    
        
    std::string toStringFormated() const 
    {
        std::string s;
        for(int i = 0; i < nSlots; ++i)
            s.append(bitrep.test(i) ? "1" : "0");     
        s.append("'");
        for(int i = nSlots; i < (nSlots*2); ++i)
            s.append(bitrep.test(i) ? "1" : "0");  
        
        for(int i = (nSlots*2); i < (nSlots_ * 2 + nSlots_ * nColours_ ); ++i)
        {
            if(((i-(nSlots_ * 2))%nColours_) == 0)
                s.append("'");
            s.append(bitrep.test(i) ? "1" : "0");  

        }
        
        std::reverse( s.begin(), s.end() );  
        return s;
    }
    

    void compareTo(const MasterMindRow<nSlots_, nColours_> & other)
    {
        size_t colorPosHits = 0;
        for(size_t i = 0; i < nSlots_; ++i)
        {
            if(color(i) == other.color(i))
                colorPosHits++;
        }
        
        size_t countColors[nColours] = {};
        size_t countColorsOther[nColours] = {};
        for(size_t i = 0; i < nSlots; ++i)
        {
            countColors[color(i)]++;
            countColorsOther[other.color(i)]++;
        }
              
        size_t colorHits = 0;
        for(size_t i = 0; i < nColours; ++i)
        {
            colorHits += std::min(countColors[i], countColorsOther[i]);
        }
            
       
        colorHits -= colorPosHits;
        
        setHits(colorPosHits, colorHits);
    }
    
    size_t posAndColorHits() const
    {
        size_t c = 0;
        for(size_t i = (nSlots*0); i < (nSlots*1); ++i)
            c += bitrep.test(i) ? 1 : 0;     
        return c;
    }
    
    size_t colorHits() const
    {
        size_t c = 0;
        for(size_t i = (nSlots*1); i < (nSlots*2); ++i)
            c += bitrep.test(i) ? 1 : 0;    
        return c;
    }
private:
    
    void setHits(size_t posAndColorHit, size_t colorHit)
    {
        BOOST_VERIFY( posAndColorHit <= nSlots );
        BOOST_VERIFY( colorHit <= nSlots );
        BOOST_VERIFY( ( colorHit + posAndColorHit ) <= nSlots );
        
        for(size_t i = 0; i < (nSlots*2); ++i)
            bitrep.reset(i);
        
        for(size_t i = 0; i < posAndColorHit; ++i)
            bitrep.set(nSlots_ * 0 + i);
        
        for(size_t i = 0; i < colorHit; ++i)
            bitrep.set(nSlots_ * 1 + i);
    }
    
    size_t countByColor(color_t colorId) const
    {
        size_t colorCount = 0;
        for(size_t i = 0; i < nSlots; ++i)
        {
            if(bitrep.test((nSlots_ * 2) + (i * nColours_) +  colorId))
                colorCount++;
        }
        return colorCount;
    }
    
    size_t searchColor(color_t colorId, size_t startPos = 0) const
    {
        BOOST_VERIFY( startPos < nSlots );
        for(size_t i = startPos; i < nSlots; ++i)
        {
            if(bitrep.test((nSlots_ * 2) + (i * nColours_) +  colorId))
                return i;
        }
        return nSlots;
    }
    
    bool hasColor(color_t colorId) const 
    {
        return countByColor(colorId) != 0;
    }
    
private:
    std::bitset< nSlots_ * 2 + nSlots_ * nColours_ > bitrep;
};
    
    

template<typename MMRowT, typename RandGenT>
void fillMMRowWithColor(MMRowT & mMRow, RandGenT & randomGen)
{
    for(size_t i = 0; i < MMRowT::nSlots; ++i)
    {
        mMRow.color(i, randomGen() % MMRowT::nColours);
    }
}


bool test();

template< typename F >
void generate_test_data( trainings_data_type &data, double rmin , double rmax , double stepsize , F f )
{
    data.x[0].clear(); data.x[1].clear(); data.x[2].clear(); data.y.clear();
    
    for( double xx = rmin ; xx <= rmax ; xx += stepsize )
    {
        for( double yy = rmin ; yy <= rmax ; yy += stepsize )
        {
            for( double zz = rmin ; zz <= rmax ; zz += stepsize )
            {
                data.x[0].push_back( xx );
                data.x[1].push_back( yy );
                data.x[2].push_back( zz );
                data.y.push_back( f( xx , yy , zz ) );
            }
        }
    }
}


int main( int argc , char *argv[] )
{
    test();
    rng_type rng;
    
    trainings_data_type c;
    generate_test_data( c , -5.0 , 5.0 + 0.1 , 0.4 , []( double x1 , double x2 , double x3 ) {
                        return  1.0 / ( 1.0 + pow( x1 , -4.0 ) ) + 1.0 / ( 1.0 + pow( x2 , -4.0 ) ) + 1.0 / ( 1.0 + pow( x3 , -4.0 ) ); } );
    gpcxx::normalize( c.y );

  
}


bool test()
{
    size_t const nSlots = 4;
    size_t const nColours = 6;
    int const seed = 100;
    boost::rand48 rd (seed);
    MasterMindRow< nSlots, nColours > colorSecret;
    MasterMindRow< nSlots, nColours > colorTest;   
    MasterMindRow< nSlots, nColours > colorTest2{0,1,2,3};   

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
        MasterMindRow< nSlots, nColours > colorSecret{0,0,0,0};
        MasterMindRow< nSlots, nColours > colorTest{0,0,0,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 0);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 4);
    }
    {
        MasterMindRow< nSlots, nColours > colorSecret{0,0,0,0};
        MasterMindRow< nSlots, nColours > colorTest  {1,1,0,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 0);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 2);
    }
    {
        MasterMindRow< nSlots, nColours > colorSecret{0,0,0,4};
        MasterMindRow< nSlots, nColours > colorTest  {1,1,3,0};   
        colorSecret.compareTo(colorTest);
        BOOST_VERIFY(colorSecret.colorHits() == 1);
        BOOST_VERIFY(colorSecret.posAndColorHits() == 0);
    }

}
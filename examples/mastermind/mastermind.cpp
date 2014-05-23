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

char const tab = '\t';
char const newl = '\n';



typedef double value_type;
typedef gpcxx::regression_training_data< value_type , 3 > trainings_data_type;
typedef std::mt19937 rng_type ;
typedef std::array< value_type , 3 > eval_context_type;
typedef std::vector< value_type > fitness_type;
typedef gpcxx::basic_named_intrusive_node< double , eval_context_type > node_type;
typedef gpcxx::intrusive_tree< node_type > tree_type;
typedef std::vector< tree_type > population_type;





template<size_t nSlots_, size_t nColours_>
class MasterMindRow 
{
public:
    static size_t const nSlots         = nSlots_;
    static size_t const nColours       = nColours_;
    
    void color(size_t pos, size_t colorId)
    {
        BOOST_VERIFY( pos < nSlots );
        BOOST_VERIFY( colorId < nColours );
        
        for(int i = 0; i < nColours_; ++i)
            bitrep.reset(nSlots_ * 2 + (pos * nColours_) + i);
        
        size_t const posOfColor = (nSlots_ * 2) + (pos * nColours_) +  colorId;
        bitrep.set( posOfColor );
    }
   
    size_t color(size_t pos) const 
    {
        for(int i = 0; i < nColours_; ++i)
            if(bitrep.test(nSlots_ * 2 + (pos * nColours_) + i))
                return i;
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
        for(int i = 0; i < nSlots_; ++i)
        {
            if(color(i) == other.color(i))
                colorPosHits++;
        }
        

        std::bitset<nSlots> colorHitsSet;
        for(int i = 0; i < nSlots_; ++i)
        {
            size_t colorId = other.color(i);
            if(hasColor(other.color(i)))
                colorHitsSet.set(i);
        }
        size_t colorHits = colorHitsSet.count() - colorPosHits;
        
        setHits(colorPosHits, colorHits);
    }
    
    size_t posAndColorHits() const
    {
        size_t c = 0;
        for(int i = (nSlots*0); i < (nSlots*1); ++i)
            c += bitrep.test(i) ? 1 : 0;     
        return c;
    }
    
    size_t colorHits() const
    {
        size_t c = 0;
        for(int i = (nSlots*1); i < (nSlots*2); ++i)
            c += bitrep.test(i) ? 1 : 0;    
        return c;
    }
private:
    
    void setHits(size_t posAndColorHit, size_t colorHit)
    {
        BOOST_VERIFY( posAndColorHit <= nSlots );
        BOOST_VERIFY( colorHit <= nSlots );
        BOOST_VERIFY( ( colorHit + posAndColorHit ) <= nSlots );
        
        for(int i = 0; i < (nSlots*2); ++i)
            bitrep.reset(i);
        
        for(int i = 0; i < posAndColorHit; ++i)
            bitrep.set(nSlots_ * 0 + i);
        
        for(int i = 0; i < colorHit; ++i)
            bitrep.set(nSlots_ * 1 + i);
    }
    
    size_t countByColor(size_t colorId) const
    {
        size_t colorCount = 0;
        for(size_t i = 0; i < nSlots; ++i)
        {
            if(bitrep.test((nSlots_ * 2) + (i * nColours_) +  colorId))
                colorCount++;
        }
        return colorCount;
    }
    
    bool hasColor(size_t colorId) const 
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



int main( int argc , char *argv[] )
{
    int const seed = 100;
    size_t const nSlots = 4;
    size_t const nColours = 6;
    boost::rand48 rd (seed);
    MasterMindRow< nSlots, nColours > colorSecret;
    MasterMindRow< nSlots, nColours > colorTest;   



    fillMMRowWithColor(colorSecret, rd);
    fillMMRowWithColor(colorTest, rd);
    
    colorTest.compareTo(colorSecret);
    std::cout << "test " << colorTest.toStringFormated()   << 
        tab << colorTest.posAndColorHits() << 
        tab << colorTest.colorHits() << newl;   
    std::cout << "secr " << colorSecret.toStringFormated() << 
        tab << colorSecret.posAndColorHits() << 
        tab << colorSecret.colorHits() << newl;  

}

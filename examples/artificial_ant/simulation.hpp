/*
 * gpcxx/examples/artificial_ant/ant_simulation.hpp
 * Date: 2014-08-18
 * Author: Gerard Choinka (gerard.choinka+gpcxx@gmail.com)
 * Copyright: Gerard Choinka
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_EXAMPLES_ARTIFICIAL_ANT_ANT_SIMULATION_HPP_INCLUDED
#define GPCXX_EXAMPLES_ARTIFICIAL_ANT_ANT_SIMULATION_HPP_INCLUDED

#include "ant_simulation_common.hpp" 

#include <array>
#include <string>

namespace ant_example {

//[ant_wold

enum class BoardState { empty, food, hadFood };
constexpr static std::array<char, 3> boardStateToChar{' ', 'O', '*'};

template<typename FieldT>
class AntBoardSimulation
{
public:
    using FieldType = FieldT;
    
    template<typename FieldInitFunction>
    AntBoardSimulation(int steps, int max_food, ant_sim::Pos2d antPos, ant_sim::Direction direction, FieldInitFunction fieldInitFunction)
        :steps_{steps}, max_food_{max_food}, antPos_{antPos}, direction_{direction}
    {
        fieldInitFunction(field_);
    } 

    void move()
    {
        --steps_;
        auto toadd = ant_sim::toPos[static_cast<size_t>(direction_)];
        antPos_ = (antPos_ + toadd);
        antPos_.x() = (antPos_.x() + xSize()) % xSize();
        antPos_.y() = (antPos_.y() + ySize()) % ySize();
        
        if(field_[antPos_.x()][antPos_.y()] == BoardState::food)
        {  
            ++foodConsumed_;
            field_[antPos_.x()][antPos_.y()] = BoardState::hadFood;
        }
    }
    
    void left()
    {
        --steps_;
        direction_ = rotateCCW(direction_);
    }
    
    void right()
    {
        --steps_;
        direction_ = rotateCW(direction_);
    }
    
    bool is_food_in_front() const
    {
        auto toadd = ant_sim::toPos[static_cast<size_t>(direction_)];
        auto newPos = antPos_ + toadd;
        newPos.x() = (newPos.x() + xSize()) % xSize();
        newPos.y() = (newPos.y() + ySize()) % ySize();
        
        return field_[newPos.x()][newPos.y()] == BoardState::food;
    }
    
    bool is_finish() const
    {
        return steps_ <= 0 || score() == 0;
    }
    
    int score() const
    {
        return max_food_ - foodConsumed_;
    }
    
    std::string get_status_line() const 
    {
        std::string res;
        res.reserve(ySize());
        (((res += "steps:") += std::to_string(steps_) += " score:") += std::to_string(score()) += " fif:") += std::to_string(is_food_in_front());
        res.insert(res.size(), ySize() - res.size(), ' ');
        return res;
    }
    
    template<typename LineSinkF>
    void get_board_as_str(LineSinkF lineSink) const
    {
        lineSink(get_status_line());
        for(size_t x = 0; x < xSize(); ++x)
        {
            std::string res; res.reserve(ySize());
            for(size_t y = 0; y < ySize(); ++y)
            {
                if(antPos_ == ant_sim::Pos2d{int(x),int(y)})
                    res += ant_sim::directionToChar[static_cast<size_t>(direction_)];
                else 
                    res += boardStateToChar[static_cast<size_t>(field_[x][y])];
            }
            lineSink(res);
        }
    }
    
    auto xSize() const
    {
        return std::size(field_[0]);
    }
    
    auto ySize() const
    {
        return std::size(field_);
    }
    
private:
    FieldT field_;
    int steps_;
    int max_food_;
    int foodConsumed_ = 0;
    ant_sim::Pos2d antPos_;
    ant_sim::Direction direction_;
};


template<int XSize, int YSize>
using AntBoardSimulationStaticSize = AntBoardSimulation<std::array<std::array<BoardState, YSize>, XSize>>;

//]

} // namespace ant_example

#endif // GPCXX_EXAMPLES_ARTIFICIAL_ANT_ANT_SIMULATION_HPP_INCLUDED

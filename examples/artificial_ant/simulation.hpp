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
template<int XSize, int YSize>
class AntBoardSimulation
{
private:
    enum class BoardState { empty, food, hadFood };
    constexpr static std::array<char, 3> boardStateToChar{' ', 'O', '*'};
    
public:
    template<typename FieldInitFunction>
    AntBoardSimulation(int steps, int max_food, ant_sim::Pos2d antPos, ant_sim::Direction direction, FieldInitFunction fieldInitFunction)
        :steps_{steps}, max_food_{max_food}, antPos_{antPos}, direction_{direction}
    {
        for(size_t x = 0; x < field_.size(); ++x)
        {
            for(size_t y = 0; y < field_[x].size(); ++y)
            {
                field_[x][y] = fieldInitFunction(x,y) ? BoardState::food : BoardState::empty;
            }
        }
    } 

    void move()
    {
        --steps_;
        auto toadd = ant_sim::toPos[static_cast<size_t>(direction_)];
        antPos_ = (antPos_ + toadd);
        antPos_.x() = (antPos_.x() + XSize) % XSize;
        antPos_.y() = (antPos_.y() + YSize) % YSize;
        
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
        newPos.x() = (newPos.x() + XSize) % XSize;
        newPos.y() = (newPos.y() + YSize) % YSize;
        
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
        res.reserve(YSize);
        (((res += "steps:") += std::to_string(steps_) += " score:") += std::to_string(score()) += " fif:") += std::to_string(is_food_in_front());
        res.insert(res.size(), YSize - res.size(), ' ');
        return res;
    }
    
    template<typename LineSinkF>
    void get_board_as_str(LineSinkF lineSink) const
    {
        lineSink(get_status_line());
        for(size_t x = 0; x < field_.size() ; ++x)
        {
            std::string res; res.reserve(YSize);
            for(size_t y = 0; y < field_.size(); ++y)
            {
                if(antPos_ == ant_sim::Pos2d{int(x),int(y)})
                    res += ant_sim::directionToChar[static_cast<size_t>(direction_)];
                else 
                    res += boardStateToChar[static_cast<size_t>(field_[x][y])];
            }
            lineSink(res);
        }
    }
    
private:
    
    std::array<std::array<BoardState, YSize>, XSize> field_;
    int steps_;
    int max_food_;
    int foodConsumed_ = 0;
    ant_sim::Pos2d antPos_;
    ant_sim::Direction direction_;
};
//]

} // namespace ant_example

#endif // GPCXX_EXAMPLES_ARTIFICIAL_ANT_ANT_SIMULATION_HPP_INCLUDED

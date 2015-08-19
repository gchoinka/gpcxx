/*
 * gpcxx/io/bracket.hpp
 * Date: 2015-08-18
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_IO_BRACKET_HPP_INCLUDED
#define GPCXX_IO_BRACKET_HPP_INCLUDED

#include <gpcxx/util/identity.hpp>
#include <gpcxx/io/detail/read_bracket.hpp>

#include <boost/algorithm/string/find.hpp>
#include <boost/range/algorithm/find.hpp>

#include <ostream>
#include <string>
#include <sstream>


namespace gpcxx {

template< typename Cursor , typename SymbolMapper >
void write_bracket_cursor( std::ostream &out , Cursor t , std::string const &opening , std::string const& closing , SymbolMapper const& mapper )
{
    out << opening;
    out << mapper( *t );
    if( t.size() != 0 )
    {
        write_bracket_cursor( out , t.children( 0 ) , opening , closing , mapper );
    }
    for( size_t i=1 ; i<t.size(); ++i )
    {
        write_bracket_cursor( out , t.children( i ) , opening , closing , mapper );
    }
    out << closing;
}


template< typename Tree , typename SymbolMapper >
void write_bracket( std::ostream &out , Tree const& t , std::string const &opening , std::string const& closing , SymbolMapper const& mapper )
{
    if( !t.empty() )
        write_bracket_cursor( out , t.root() , opening , closing , mapper );
}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
std::string bracket_string( Tree const& t , std::string const &opening = "{" , std::string const& closing = "}" , SymbolMapper const &mapper = SymbolMapper() )
{
    std::ostringstream str;
    write_bracket( str , t , opening , closing , mapper );
    return str.str();
}



namespace detail {
    
template< typename Tree , typename SymbolMapper >
struct bracket_writer
{
    Tree const& m_t;
    std::string const &m_opening;
    std::string const &m_closing;
    SymbolMapper const& m_mapper;
    bracket_writer( Tree const& t , std::string const &opening , std::string const& closing , SymbolMapper const& mapper )
    : m_t( t ) , m_opening( opening ) , m_closing( closing ) , m_mapper( mapper ) { }

    std::ostream& operator()( std::ostream& out ) const
    {
        write_bracket( out , m_t , m_opening , m_closing , m_mapper );
        return out;
    }
};


template< typename T , typename SymbolMapper >
std::ostream& operator<<( std::ostream& out , bracket_writer< T , SymbolMapper > const& p )
{
    return p( out );
}

} // namespace detail




template< typename T , typename SymbolMapper = gpcxx::identity >
detail::bracket_writer< T , SymbolMapper > bracket( T const& t , std::string const &opening = "{" , std::string const& closing = "}" , SymbolMapper const &mapper = SymbolMapper() )
{
    return detail::bracket_writer< T , SymbolMapper >( t , opening , closing , mapper );
}





template< typename Tree , typename NodeMapper >
void read_bracket( std::string str , Tree &tree , NodeMapper const& mapper , std::string const &opening = "{" , std::string const& closing = "}" )
{
    auto iter = boost::range::find( str , opening );
    detail::read_bracket_impl( std::make_pair( iter , str.end() ) , tree , tree.root() , opening , closing , mapper() );
    
//     using iterator_t = boost::split_iterator< std::string::const_iterator >;
//     
//     iterator_t first = iterator_t( str , boost::first_finder( opening , boost::is_iequal() ) );
    
    // auto rng = boost::ifind_first( str , opening );
    
    
    
//     
//     if( opening != "" ) boost::algorithm::erase_all( str , opening );
//     if( closing != "" ) boost::algorithm::erase_all( str , closing );
//     
//     
//     
//     detail::read_polish_impl( first , tree , tree.root() , mapper );
}



} // namespace gpcxx


#endif // GPCXX_IO_BRACKET_HPP_INCLUDED

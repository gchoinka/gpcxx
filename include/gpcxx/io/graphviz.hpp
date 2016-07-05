/*
 * graphviz.hpp
 * Date: 2013-02-03
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 */

#ifndef GPCXX_IO_GRAPHVIZ_HPP_INCLUDED
#define GPCXX_IO_GRAPHVIZ_HPP_INCLUDED

#include <gpcxx/util/identity.hpp>
#include <gpcxx/util/exception.hpp>

#include <fstream>
#include <cstdio>


namespace gpcxx {


namespace detail {
    
template< typename Cursor , typename SymbolMapper >
void write_graphviz_cursor( std::ostream &out , Cursor const &n , size_t &count , bool print_node_info , SymbolMapper const& mapper )
{
    size_t cc = count;
    out << "NODE" << cc << " [ label = \"" << mapper( *n );
    if( print_node_info )
    {
        out << "(" << n.height() << " " << n.level() << ")";
    }
    out << "\" ]\n";
        
    for( size_t i=0 ; i<n.size() ; ++i )
    {
        ++count;
        out << "NODE" << cc << " -> " << "NODE" << count << "\n";
        write_graphviz_cursor( out , n.children( i ) , count , print_node_info , mapper );
    }
}

} // namespace detail


template< typename Cursor , typename SymbolMapper >
void write_graphviz_cursor( std::ostream &out , Cursor const& n , bool print_node_info , SymbolMapper const& mapper )
{
    out << "digraph G\n";
    out << "{\n";
    size_t count = 0;
    detail::write_graphviz_cursor( out , n , count , print_node_info , mapper );
    out << "}\n";
}




template< typename Tree , typename SymbolMapper >
void write_graphviz( std::ostream &out , Tree const& t , bool print_node_info , SymbolMapper const& mapper )
{
    if( !t.empty() )
        write_graphviz_cursor( out , t.root() , print_node_info , mapper );
}



namespace detail {
    
    
template< typename Tree , typename SymbolMapper >
struct graphviz_writer
{
    Tree const& m_t;
    SymbolMapper const& m_mapper;
    bool m_print_node_info;
    
    graphviz_writer( Tree const& t , bool print_node_info , SymbolMapper const& mapper )
    : m_t( t ) , m_mapper( mapper ) , m_print_node_info( print_node_info ) { }
    
    std::ostream& operator()( std::ostream& out ) const
    {
        write_graphviz( out , m_t , m_print_node_info , m_mapper );
        return out;
    }
};

template< typename T , typename SymbolMapper >
std::ostream& operator<<( std::ostream& out , graphviz_writer< T , SymbolMapper > const& p )
{
    return p( out );
}


} // namespace detail



template< typename T , typename SymbolMapper = gpcxx::identity >
detail::graphviz_writer< T , SymbolMapper > graphviz( T const& t , bool print_node_info = false , SymbolMapper const &mapper = SymbolMapper() )
{
    return detail::graphviz_writer< T , SymbolMapper >( t , print_node_info , mapper );
}




// template< typename Tree , typename SymbolMapper = gpcxx::identity >
// void generate_graphviz_pdf( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
// {
//     std::ofstream fout( "__tmp__.dot" );
//     write_graphviz( fout , t , false , mapper );
//     fout.close();
// 
//     system ( "dot -Tps2 __tmp__.dot > __tmp__.ps" );
//     system ( ( std::string( "ps2pdf __tmp__.ps " ) + filename ).c_str() );
//     system ( "rm __tmp__.dot" );
//     system ( "rm __tmp__.ps" );
// }

namespace detail {
    
template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_output( const std::string& filename , const Tree& t , std::string const& format_string , SymbolMapper const& mapper = SymbolMapper() )
{
    using namespace std::string_literals;
    
    std::ofstream fout( "__tmp__.dot" );
    write_graphviz( fout , t , false , mapper );
    fout.close();

    std::string cmd = "dot -T"s + format_string + " __tmp__.dot -o"s + filename;
    if( std::system ( cmd.c_str() ) != 0 )
        throw gpcxx_exception( "Could not create graphviz output." );
    if( std::system ( "rm __tmp__.dot" ) != 0 )
        throw gpcxx_exception( "Could not remove temporary dot file." );
}

}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_svg( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
{
    detail::generate_graphviz_output( filename , t , "svg" , mapper );
}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_png( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
{
    detail::generate_graphviz_output( filename , t , "png" , mapper );
}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_jpg( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
{
    detail::generate_graphviz_output( filename , t , "jpg" , mapper );
}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_pdf( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
{
    detail::generate_graphviz_output( filename , t , "pdf" , mapper );
}

template< typename Tree , typename SymbolMapper = gpcxx::identity >
void generate_graphviz_ps( const std::string &filename , const Tree &t , SymbolMapper const& mapper = SymbolMapper() )
{
    detail::generate_graphviz_output( filename , t , "ps" , mapper );
}





} // namespace gpcxx

#endif // GPCXX_IO_GRAPHVIZ_HPP_INCLUDED

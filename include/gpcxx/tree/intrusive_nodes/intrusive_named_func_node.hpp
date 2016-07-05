/*
 * gpcxx/tree/intrusive_nodes/intrusive_named_func_node.hpp
 * Date: 2013-10-15
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_NAMED_FUNC_NODE_HPP_INCLUDED
#define GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_NAMED_FUNC_NODE_HPP_INCLUDED

#include <gpcxx/tree/intrusive_nodes/intrusive_node.hpp>

#include <functional>
#include <string>



namespace gpcxx {




template< typename Res , typename Context, typename Allocator = std::allocator< void* > >
class intrusive_named_func_node : public gpcxx::intrusive_node< intrusive_named_func_node< Res , Context , Allocator > , Allocator >
{
public:
    
    using result_type = Res;
    using context_type = Context;
    using node_type = intrusive_named_func_node< result_type , context_type , Allocator >;
    using func_type = std::function< result_type( context_type& , node_type const& ) >;
    
    intrusive_named_func_node( func_type f , std::string name )
    : m_func( std::move( f ) ) , m_name( std::move( name ) ) { }
    
    result_type eval( context_type & context ) const
    {
        return m_func( context , *this );
    }
    
    std::string const& name( void ) const
    {
        return m_name;
    }
    
    std::string& name( void )
    {
        return m_name;
    }
    
    bool operator==( intrusive_named_func_node const &other ) const
    {
        return m_name == other.m_name;
    }
    
    bool operator!=( intrusive_named_func_node const& other ) const
    {
        return ! ( *this == other );
    }
    
private:
    
    func_type m_func;
    std::string m_name;
};




template< typename Res , typename Context , typename Allocator >
std::ostream& operator<<( std::ostream &out , intrusive_named_func_node< Res , Context , Allocator > const& node )
{
    out << node.name();
    return out;
}


} // namespace gpcxx


#endif // GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_NAMED_FUNC_NODE_HPP_INCLUDED

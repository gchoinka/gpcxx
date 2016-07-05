/*
 * gpcxx/tree/intrusive_nodes/intrusive_func_node.hpp
 * Date: 2013-10-15
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_FUNC_NODE_HPP_INCLUDED
#define GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_FUNC_NODE_HPP_INCLUDED

#include <gpcxx/tree/intrusive_nodes/intrusive_node.hpp>

#include <functional>

namespace gpcxx {
    
    
// Attention this class is not intended to be copied around
template< typename Res , typename Context , typename Allocator = std::allocator< void* > >
class intrusive_func_node : public gpcxx::intrusive_node< intrusive_func_node< Res , Context , Allocator > , Allocator >
{
public:
    
    using result_type = Res;
    using context_type = Context;
    using node_type = intrusive_func_node< result_type , context_type, Allocator >;
    
    
    typedef std::function< result_type( context_type const& , node_type const& ) > func_type;
    
    intrusive_func_node( func_type f )
    : m_func( std::move( f ) ) { }
    
    result_type eval( context_type const & context ) const
    {
        return m_func( context , *this );
    }
    
private:
    
    func_type m_func;
};




} // namespace gpcxx


#endif // GPCXX_TREE_INTRUSIVE_NODES_INTRUSIVE_FUNC_NODE_HPP_INCLUDED

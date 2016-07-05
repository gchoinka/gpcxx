/*
 * gpcxx/tree/intrusive_node.hpp
 * Date: 2013-10-14
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_TREE_INTRUSIVE_NODE_HPP_INCLUDED
#define GPCXX_TREE_INTRUSIVE_NODE_HPP_INCLUDED

#include <gpcxx/tree/detail/node_base.hpp>

#include <array>
#include <algorithm>
#include <cassert>
#include <memory>

    
namespace gpcxx {
    
namespace detail {

template< typename Node > class tree_base_cursor;

}



template< typename Node , typename Allocator = std::allocator< void* > >
class intrusive_node : public detail::node_base< detail::descending_vector_node< Allocator > > 
{
    template< typename N > friend class detail::tree_base_cursor;
   
public:
    
    using node_type = Node;
    using node_base_type = detail::node_base< detail::descending_vector_node< Allocator > >;
    using node_pointer = node_type*;
    using const_node_pointer = node_type const*;
    using value_type = node_type;
    using reference = value_type&;
    using const_reference = value_type const&;
    
    intrusive_node( node_type *parent = nullptr ) noexcept
    : node_base_type( parent )
    {
    }
    
    intrusive_node( intrusive_node const& )
    : node_base_type()
    {
    }
    
    intrusive_node( intrusive_node && )
    : node_base_type()
    {
    }
    
    reference operator*( void ) noexcept
    {
        return get();
    }
    
    const_reference operator*( void ) const noexcept 
    {
        return get();
    }
    
    intrusive_node& operator=( intrusive_node const& )
    {
        return *this;
    }
    
    intrusive_node& operator=( intrusive_node && )
    {
        return *this;
    }

    node_type& child( size_t i ) noexcept
    {
        return static_cast< node_type& >( *( this->child_node(i) ) );
    }
    
    node_type const& child( size_t i ) const noexcept
    {
        return static_cast< node_type const& >( *( this->child_node(i) ) );
    }
    
    void clear_children( void )
    {
        std::fill( this->m_children.begin() , this->m_children.end() , nullptr );
    }
    
private:
    
    reference get( void ) noexcept { return *static_cast< Node* >( this ); }
    
    const_reference get( void ) const noexcept { return *static_cast< Node const* >( this ); }
    
    
};


} // namespace gpcxx





#endif // GPCXX_TREE_INTRUSIVE_NODE_HPP_INCLUDED

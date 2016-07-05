/*
 * gpcxx/tree/detail/tree_base_cursor.hpp
 * Date: 2015-02-09
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 * Copyright: Karsten Ahnert
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GPCXX_TREE_DETAIL_TREE_BASE_CURSOR_HPP_INCLUDED
#define GPCXX_TREE_DETAIL_TREE_BASE_CURSOR_HPP_INCLUDED

#include <gpcxx/tree/detail/node_helpers.hpp>
#include <gpcxx/tree/cursor_traits.hpp>
#include <gpcxx/util/assert.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include <type_traits>




namespace gpcxx {
namespace detail {

    

/**
 * cursor consist of node pointer of parent and index for the current node, which is the child index in the parent
 * 
 * cursor validity:
 * # complete invalid cursor: parent node is nullptr, exist if default constructed
 * # invalid cursor - current node is not present, but parent is ok
 *   * the root of an empty tree is such kind of invalid node
 *   * the end cursor of a node is such kind of invalid node
 * 
 * TODO: Find a better name for complete invalid cursors and implement a check in cursor
 * 
 * special cursors:
 * # root: invalid cursor can be used to insert
 * # shoot: invalid cursor symbolizing the very last element
 */
template< typename Node >
class tree_base_cursor : public boost::iterator_facade<
    tree_base_cursor< Node > ,                           // Derived-Iterator
    typename node_value_getter< Node >::type ,            // Value
    boost::random_access_traversal_tag // ,               // Category
    // boost::use_default ,                               // Reference
    // boost::use_default 
    >
{
    
    friend class boost::iterator_core_access;
    
    //
    // private types:
    // 
    using node_type = Node;
    using node_pointer = node_type* ;
    using real_node_type = typename std::remove_const< Node >::type;
    using node_base = typename node_base_getter< Node >::type;
    using node_base_pointer = node_base*;
    using real_node_base = typename std::remove_const< node_base >::type;
    using real_node_base_pointer = real_node_base*;
    using const_real_node_base_pointer = real_node_base const* ;
    
    using base_type = boost::iterator_facade<
        tree_base_cursor< Node > ,                        // Derived-Iterator
        typename node_value_getter< Node >::type ,         // Value
        boost::random_access_traversal_tag >;
    
    template< typename OtherNode >
    using other_node_enabler = std::enable_if< std::is_convertible< OtherNode* , node_pointer >::value >;
        
    

    
public:


    //
    // types:
    //
    using size_type = size_t; 
    using cursor = tree_base_cursor< node_type >;
    using const_cursor = tree_base_cursor< real_node_type const >;
    

    
    //
    // construct:
    //
    tree_base_cursor( node_base_pointer node = nullptr , size_type pos = 0 )
    : m_node( node ) , m_pos( pos ) { }
    
    template< typename OtherNode , typename Enabler = typename other_node_enabler< OtherNode >::type >
    tree_base_cursor( tree_base_cursor< OtherNode > const& other )
    : m_node( other.parent_node() ) , m_pos( other.pos() ) { }
    
    tree_base_cursor( tree_base_cursor const& ) = default;
    tree_base_cursor( tree_base_cursor&& ) = default;
    tree_base_cursor& operator=( tree_base_cursor const& ) = default;
    tree_base_cursor& operator=( tree_base_cursor&& ) = default;
    
    
    
    //
    // capacity:
    //
    size_type size( void ) const noexcept
    {
        return m_node->child_node( m_pos )->size();
    }
    
    size_type max_size( void ) const noexcept
    {
        return m_node->max_size();
    }
    
    bool empty( void ) const noexcept
    {
        return ( size() == 0 );
    }
    
    // operator==
    // operator!=
    
    
    
    
    // a.parity() size_type   (std::distance(b.begin(), a) if b is a's parent.)
    
    //
    // cursors:
    //
    cursor begin( void )
    {
        return cursor( m_node->child_node( m_pos ) , 0 );
    }
    
    const_cursor begin( void ) const
    {
        return cbegin();
    }
    
    const_cursor cbegin( void ) const
    {
        return const_cursor( m_node->child_node( m_pos ) , 0 );
    }
    
    cursor end( void )
    {
        return cursor( m_node->child_node( m_pos ) , this->size() );
    }
    
    const_cursor end( void ) const
    {
        return cend();
    }
    
    const_cursor cend( void ) const
    {
        return const_cursor( m_node->child_node( m_pos ) , this->size() );
    }
    
    cursor parent( void )
    {
        GPCXX_ASSERT( ! is_root() );
        auto parent_node = m_node->parent_node();
        return cursor( parent_node , parent_node->child_index( m_node ) );
    }
    
    const_cursor parent( void ) const
    {
        return cparent();
    }
    
    const_cursor cparent( void ) const
    {
        GPCXX_ASSERT( ! is_root() );
        auto parent_node = m_node->parent_node();
        return const_cursor( parent_node , parent_node->child_index( m_node ) );
    }
    
    cursor children( size_type i )
    {
        return cursor( m_node->child_node( m_pos ) , i );
    }
    
    const_cursor children( size_type i ) const
    {
        return const_cursor( m_node->child_node( m_pos ) , i );
    }
    
    

    //
    // structure queries:
    // 
    size_type height( void ) const noexcept
    {
        return node()->height();
    }
    
    size_type level( void ) const noexcept
    {
        return parent_node()->level();   // level is shifted, because the root is not the very first node.
    }
    
    size_t num_nodes( void ) const noexcept
    {
        return node()->count_nodes();
    }
    
    bool is_root( void ) const noexcept
    {
        return ( ( m_node->parent_node() == nullptr ) && ( m_pos == 0 ) );
    }
    
    bool is_shoot( void ) const noexcept
    {
        return ( ( m_node->parent_node() == nullptr ) && ( m_pos == 1 ) );
    }
    
    bool valid( void ) const
    {
        return ( m_node != nullptr ) && ( m_node->valid_child( m_pos ) );
    }
    
    bool invalid( void ) const
    {
        return ( m_node == nullptr ) || ( m_node->invalid_child( m_pos ) );
    }
    
    
    
//protected:
public:
    
    //
    // node accessors:
    //
    node_base_pointer parent_node( void ) noexcept
    {
        return m_node;
    }
    
    const node_base_pointer parent_node( void ) const noexcept
    {
        return m_node;
    }
    
    node_base_pointer node( void ) noexcept
    {
        return m_node->child_node( m_pos );
    }
    
    const node_base_pointer node( void ) const noexcept
    {
        return m_node->child_node( m_pos );
    }
    
    size_type pos( void ) const noexcept
    {
        return m_pos;
    }
    
    
private:

    //
    // iterator interface:
    //
    void increment( void )
    {
        ++m_pos;
    }
    
    void decrement( void )
    {
        --m_pos;
    }
    
    void advance( typename base_type::difference_type n )
    {
        m_pos += n;
    }
    
    typename base_type::difference_type distance_to( tree_base_cursor const& other ) const
    {
        using diff_type = typename base_type::difference_type;
        return static_cast< diff_type >( other.m_pos ) - static_cast< diff_type >( m_pos );
    }
    
    bool equal( tree_base_cursor const& other) const
    {
        return ( other.m_node == m_node ) && ( other.m_pos == m_pos );
    }
    
    typename base_type::reference dereference() const
    {
        return static_cast< node_pointer >( m_node->child_node( m_pos ) )->get();
    }
    
    
    node_base_pointer m_node;
    size_type m_pos;
};


} // namespace detail







template< typename Node >
struct is_cursor< detail::tree_base_cursor< Node > > : public std::true_type { };




} // namespace gpcxx


#endif // GPCXX_TREE_DETAIL_TREE_BASE_CURSOR_HPP_INCLUDED

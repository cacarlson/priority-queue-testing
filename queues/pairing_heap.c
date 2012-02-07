#include "pairing_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

pairing_heap* create_heap( uint32_t capacity )
{
    map = create_mem_map( capacity );
    pairing_heap *heap = (pairing_heap*) calloc( 1, sizeof( pairing_heap ) );
    return heap;
}

void destroy_heap( pairing_heap *heap )
{
    clear_heap( heap );
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( pairing_heap *heap )
{
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( pairing_heap *heap, pairing_node *node )
{
    return node->key;
}

item_type* get_item( pairing_heap *heap, pairing_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t get_size( pairing_heap *heap )
{
    return heap->size;
}

pairing_node* insert( pairing_heap *heap, item_type item, key_type key )
{
    pairing_node *wrapper = heap_node_alloc( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    heap->size++;

    heap->root = merge( heap, heap->root, wrapper );

    return wrapper;
}

pairing_node* find_min( pairing_heap *heap )
{
    if ( empty( heap ) )
        return NULL;
    return heap->root;
}

key_type delete_min( pairing_heap *heap )
{
    return delete( heap, heap->root );
}

key_type delete( pairing_heap *heap, pairing_node *node )
{
    key_type key = node->key;

    if ( node == heap->root )
        heap->root = collapse( heap, node->child );
    else
    {
        if ( node->prev->child == node )
            node->prev->child = node->next;
        else
            node->prev->next = node->next;

        if ( node->next != NULL )
            node->next->prev = node->prev;

        heap->root = merge( heap, heap->root, collapse( heap, node->child ) );
    }

    heap_node_free( map, node );
    heap->size--;

    return key;
}

void decrease_key( pairing_heap *heap, pairing_node *node, key_type new_key )
{
    node->key = new_key;
    if ( node == heap->root )
        return;

    if ( node->prev->child == node )
        node->prev->child = node->next;
    else
        node->prev->next = node->next;

    if ( node->next != NULL )
        node->next->prev = node->prev;

    heap->root = merge( heap, heap->root, node );
}

bool empty( pairing_heap *heap )
{
    return ( heap->size == 0 );
}

pairing_node* merge( pairing_heap *heap, pairing_node *a, pairing_node *b )
{
    pairing_node *parent, *child;

    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;

    if ( b->key < a->key )
    {
        parent = b;
        child = a;
    }
    else
    {
        parent = a;
        child = b;
    }

    child->next = parent->child;
    if ( parent->child != NULL )
        parent->child->prev = child;
    child->prev = parent;
    parent->child = child;

    parent->next = NULL;
    parent->prev = NULL;

    return parent;
}

pairing_node* collapse( pairing_heap *heap, pairing_node *node )
{
    pairing_node *tail, *a, *b, *next, *result;

    if ( node == NULL )
        return NULL;

    next = node;
    tail = NULL;
    while ( next != NULL )
    {
        a = next;
        b = a->next;
        if ( b != NULL )
        {
            next = b->next;
            result = merge( heap, a, b );
            // tack the result onto the end of the temporary list
            result->prev = tail;
            tail = result;                    
        }
        else
        {
            a->prev = tail;
            tail = a;
            break;
        }
    }
    
    result = NULL;
    while ( tail != NULL )
    {
        // trace back through to merge the list
        next = tail->prev;
        result = merge( heap, result, tail );
        tail = next;
    }

    return result;
}

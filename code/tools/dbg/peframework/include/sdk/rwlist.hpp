/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        eirrepo/sdk/rwlist.hpp
*  PURPOSE:     RenderWare list export for multiple usage
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_LIST_DEFINITIONS_
#define _RENDERWARE_LIST_DEFINITIONS_

#include <assert.h>

// Macros used by RW, optimized for usage by the engine (inspired by S2 Games' macros)
#ifdef USE_MACRO_LIST

#define LIST_ISVALID(item) ( (item).next->prev == &(item) && (item).prev->next == &(item) )
#define LIST_VALIDATE(item) ( assert( LIST_ISVALID( (item) ) ) )
#define LIST_APPEND(link, item) ( (item).next = &(link), (item).prev = (link).prev, (item).prev->next = &(item), (item).next->prev = &(item) )
#define LIST_INSERT(link, item) ( (item).next = (link).next, (item).prev = &(link), (item).prev->next = &(item), (item).next->prev = &(item) )
#define LIST_REMOVE(link) ( (link).prev->next = (link).next, (link).next->prev = (link).prev )
#define LIST_CLEAR(link) ( (link).prev = &(link), (link).next = &(link) )
#define LIST_INITNODE(link) ( (link).prev = NULL, (link).next = NULL )
#ifdef _DEBUG
#define LIST_EMPTY(link) ( (link).prev == &(link) && (link).next == &(link) )
#else
#define LIST_EMPTY(link) ( (link).next == &(link) )
#endif //_DEBUG

#else //USE_MACRO_LIST

// Optimized versions.
// Not prone to bugs anymore.
template <typename listType>    inline bool LIST_ISVALID( listType& item )                      { return item.next->prev == &item && item.prev->next == &item; }
template <typename listType>    inline void LIST_VALIDATE( listType& item )                     { return assert( LIST_ISVALID( item ) ); }
template <typename listType>    inline void LIST_APPEND( listType& link, listType& item )       { item.next = &link; item.prev = link.prev; item.prev->next = &item; item.next->prev = &item; }
template <typename listType>    inline void LIST_INSERT( listType& link, listType& item )       { item.next = link.next; item.prev = &link; item.prev->next = &item; item.next->prev = &item; }
template <typename listType>    inline void LIST_REMOVE( listType& link )                       { link.prev->next = link.next; link.next->prev = link.prev; }
template <typename listType>    inline void LIST_CLEAR( listType& link )                        { link.prev = &link; link.next = &link; }
template <typename listType>    inline void LIST_INITNODE( listType& link )                     { link.prev = NULL; link.next = NULL; }
template <typename listType>    inline bool LIST_EMPTY( listType& link )                        { return link.prev == &link && link.next == &link; }

#endif //USE_MACRO_LIST

// These have to be macros unfortunately.
// Special macros used by RenderWare only.
#define LIST_GETITEM(type, item, node) ( (type*)( (char*)(item) - offsetof(type, node) ) )
#define LIST_FOREACH_BEGIN(type, root, node) for ( RwListEntry <type> *iter = (root).next, *niter; iter != &(root); iter = niter ) { type *item = LIST_GETITEM(type, iter, node); niter = iter->next;
#define LIST_FOREACH_END }

template < class type >
struct RwListEntry
{
    inline RwListEntry( void )  {}

    // We do not support default move assignment ever.
    // If you want to do that you must mean it, using moveFrom method.
    inline RwListEntry( RwListEntry&& right ) = delete;
    inline RwListEntry( const RwListEntry& right ) = delete;

    // Since assignment operators could be expected we make this clear
    // that they are not.
    inline RwListEntry& operator =( RwListEntry&& right ) = delete;
    inline RwListEntry& operator =( const RwListEntry& right ) = delete;

    inline void moveFrom( RwListEntry&& right )
    {
        // NOTE: members of this are assumed invalid.
        //       hence this is more like a moveFromConstructor.

        RwListEntry *realNext = right.next;
        RwListEntry *realPrev = right.prev;
        
        realNext->prev = this;
        realPrev->next = this;

        this->next = realNext;
        this->prev = realPrev;

        // The beautiful thing is that we implicitly "know" about
        // validity of list nodes. In other words there is no value
        // we set the node pointers to when moving away.
    }

    RwListEntry <type> *next, *prev;
};
template < class type >
struct RwList
{
    inline RwList( void )
    {
        LIST_CLEAR( this->root );
    }

    inline RwList( RwList&& right )
    {
        if ( !LIST_EMPTY( right.root ) )
        {
            this->root.moveFrom( std::move( right.root ) );

            LIST_CLEAR( right.root );
        }
        else
        {
            LIST_CLEAR( this->root );
        }
    }

    inline void operator =( RwList&& right )
    {
        if ( LIST_EMPTY( right.root ) == false )
        {
            RwListEntry <type> *insNext = right.root.next;
            RwListEntry <type> *insPrev = right.root.prev;

            RwListEntry <type> *ourNext = root.next;
            RwListEntry <type> *ourPrev = root.prev;

            insNext->prev = ourPrev;
            ourPrev->next = insNext;

            insPrev->next = &this->root;
            this->root.prev = insPrev;

            LIST_CLEAR( right.root );
        }
    }

    RwListEntry <type> root;
};

// List helpers.
template <typename structType, size_t nodeOffset, bool isReverse = false>
struct rwListIterator
{
    inline rwListIterator( RwList <structType>& rootNode )
    {
        this->rootNode = &rootNode.root;

        if ( isReverse == false )
        {
            this->list_iterator = this->rootNode->next;
        }
        else
        {
            this->list_iterator = this->rootNode->prev;
        }
    }

    inline rwListIterator( RwList <structType>& rootNode, RwListEntry <structType> *node )
    {
        this->rootNode = &rootNode.root;

        this->list_iterator = node;
    }

    inline rwListIterator( const rwListIterator& right )
    {
        this->rootNode = right.rootNode;

        this->list_iterator = right.list_iterator;
    }

    inline rwListIterator( rwListIterator&& right )
    {
        this->rootNode = right.rootNode;

        this->list_iterator = right.list_iterator;

        right.rootNode = NULL;
        right.list_iterator = NULL;
    }

    inline void operator = ( const rwListIterator& right )
    {
        this->rootNode = right.rootNode;

        this->list_iterator = right.list_iterator;
    }

    inline bool IsEnd( void ) const
    {
        return ( this->rootNode == this->list_iterator );
    }

    inline void Increment( void )
    {
        if ( isReverse == false )
        {
            this->list_iterator = this->list_iterator->next;
        }
        else
        {
            this->list_iterator = this->list_iterator->prev;
        }
    }

    inline structType* Resolve( void ) const
    {
        return (structType*)( (char*)this->list_iterator - nodeOffset );
    }

private:
    RwListEntry <structType> *rootNode;
    RwListEntry <structType> *list_iterator;
};

#define DEF_LIST_ITER( newName, structType, nodeName ) \
    typedef rwListIterator <structType, offsetof(structType, nodeName)> newName;

#endif //_RENDERWARE_LIST_DEFINITIONS_
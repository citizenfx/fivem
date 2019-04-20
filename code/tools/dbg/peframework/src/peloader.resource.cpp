// Resource helper API.
#include "peloader.h"

#include <string>

#include <sdk/UniChar.h>

// Generic finder routine.
PEFile::PEResourceItem* PEFile::PEResourceDir::FindItem( bool isIdentifierName, const std::u16string& name, std::uint16_t identifier )
{
    if ( isIdentifierName )
    {
        auto findIter = this->idChildren.find( identifier );

        if ( findIter != this->idChildren.end() )
        {
            return *findIter;
        }
    }
    else
    {
        auto findIter = this->namedChildren.find( name );

        if ( findIter != this->namedChildren.end() )
        {
            return *findIter;
        }
    }

    return NULL;
}

std::wstring PEFile::PEResourceItem::GetName( void ) const
{
    if ( !this->hasIdentifierName )
    {
        return CharacterUtil::ConvertStrings <char16_t, wchar_t> ( this->name );
    }
    else
    {
        std::wstring charBuild( L"(ident:" );

        charBuild += std::to_wstring( this->identifier );

        charBuild += L")";

        return charBuild;
    }
}

bool PEFile::PEResourceDir::AddItem( PEFile::PEResourceItem *theItem )
{
    if ( theItem->hasIdentifierName )
    {
        this->idChildren.insert( theItem );
    }
    else
    {
        this->namedChildren.insert( theItem );
    }

    return true;
}

bool PEFile::PEResourceDir::RemoveItem( const PEFile::PEResourceItem *theItem )
{
    if ( theItem->hasIdentifierName )
    {
        auto findIter = this->idChildren.find( theItem );

        if ( findIter != this->idChildren.end() )
        {
            this->idChildren.erase( findIter );
            
            return true;
        }
    }
    else
    {
        auto findIter = this->namedChildren.find( theItem );

        if ( findIter != this->namedChildren.end() )
        {
            this->namedChildren.erase( findIter );

            return true;
        }
    }

    return false;
}

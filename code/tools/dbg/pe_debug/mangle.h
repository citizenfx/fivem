// Header file for compiler name mangling tools.

#ifndef _MANGLING_TOOLS_HEADER_
#define _MANGLING_TOOLS_HEADER_

#include <sdk/MacroUtils.h>
#include <sdk/rwlist.hpp>

#include <string>
#include <vector>

// Built-in types.
enum class eSymbolValueType
{
    UNKNOWN,
    VOID,
    WCHAR_T,
    BOOL,
    CHAR,
    UNSIGNED_CHAR,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    LONG,
    UNSIGNED_LONG,
    LONG_LONG,
    UNSIGNED_LONG_LONG,
    INT128,
    UNSIGNED_INT128,
    FLOAT,
    DOUBLE,
    LONG_DOUBLE,
    FLOAT128,
    VARARG,
    CUSTOM
};

enum class eSymbolTypeQualifier
{
    VALUE,      // with built-in type specifically
    POINTER,
    REFERENCE,
    RVAL_REFERENCE,
    CUSTOM      // with type namespace given
};

struct mangle_parse_error
{
    inline mangle_parse_error( void )
    {
        return;
    }
};

struct symbolType_t;

struct symbolParsePoint_t
{
    virtual symbolType_t ResolveParsePoint( void ) const = 0;
    virtual bool isConstant( void ) const = 0;

    inline symbolParsePoint_t( void ) = default;
    inline symbolParsePoint_t( const symbolParsePoint_t& right ) = delete;
    inline symbolParsePoint_t( symbolParsePoint_t&& right )
    {
        this->moveOverNode( std::move( right ) );
    }

private:
    AINLINE void moveOverNode( symbolParsePoint_t&& right )
    {
        if ( right.isParsePointRegistered )
        {
            right.isParsePointRegistered = false;

            this->parseNode.moveFrom( std::move( right.parseNode ) );

            this->isParsePointRegistered = true;
        }
        else
        {
            this->isParsePointRegistered = false;
        }
    }

    AINLINE void unregisterParsePoint( void )
    {
        if ( this->isParsePointRegistered )
        {
            LIST_REMOVE( this->parseNode );

            this->isParsePointRegistered = false;
        }
    }

public:
    inline ~symbolParsePoint_t( void )
    {
        this->unregisterParsePoint();
    }

    inline symbolParsePoint_t& operator = ( const symbolParsePoint_t& right ) = delete;
    inline symbolParsePoint_t& operator = ( symbolParsePoint_t&& right )
    {
        this->unregisterParsePoint();

        this->moveOverNode( std::move( right ) );

        return *this;
    }

    bool isParsePointRegistered = false;
    RwListEntry <symbolParsePoint_t> parseNode;
};

struct SymbolCollection
{
    inline SymbolCollection( void )
    {
        return;
    }

    inline SymbolCollection( SymbolCollection&& right ) = default;
    inline SymbolCollection( const SymbolCollection& right ) = delete;

    inline ~SymbolCollection( void )
    {
        LIST_FOREACH_BEGIN( symbolParsePoint_t, this->parsePoints.root, parseNode )

            item->isParsePointRegistered = false;

        LIST_FOREACH_END

        LIST_CLEAR( this->parsePoints.root );
    }

    inline SymbolCollection& operator = ( SymbolCollection&& right ) = default;
    inline SymbolCollection& operator = ( const SymbolCollection& right ) = delete;

    inline void RegisterParsePoint( symbolParsePoint_t& point )
    {
        assert( point.isParsePointRegistered == false );

        LIST_APPEND( this->parsePoints.root, point.parseNode );

        point.isParsePointRegistered = true;
    }

    RwList <symbolParsePoint_t> parsePoints;
};

struct symbolicTemplateSpec_t abstract
{
    virtual ~symbolicTemplateSpec_t( void )
    {
        return;
    }

    virtual symbolicTemplateSpec_t* Clone( void ) const = 0;
};

struct symbolicLiteral_t : public symbolicTemplateSpec_t
{
    eSymbolValueType literalType;
    unsigned long literalValue;

    symbolicTemplateSpec_t* Clone( void ) const override
    {
        return new symbolicLiteral_t( *this );
    }
};

struct symbolicTemplateArg_t
{
    inline symbolicTemplateArg_t( void ) = default;
    inline symbolicTemplateArg_t( const symbolicTemplateArg_t& right )
        : type( right.type )
    {
        if ( symbolicTemplateSpec_t *rightPtr = right.ptr )
        {
            this->ptr = rightPtr->Clone();
        }
        else
        {
            this->ptr = NULL;
        }
    }
    inline symbolicTemplateArg_t( symbolicTemplateArg_t&& right )
    {
        this->type = right.type;
        this->ptr = right.ptr;

        right.ptr = NULL;
    }

private:
    inline void releasePointer( void )
    {
        if ( symbolicTemplateSpec_t *ptr = this->ptr )
        {
            delete ptr;

            this->ptr = NULL;
        }
    }

public:
    inline ~symbolicTemplateArg_t( void )
    {
        this->releasePointer();
    }

    inline symbolicTemplateArg_t& operator = ( const symbolicTemplateArg_t& right )
    {
        this->releasePointer();

        this->type = right.type;
        
        if ( symbolicTemplateSpec_t *rightPtr = right.ptr )
        {
            this->ptr = rightPtr->Clone();
        }
        else
        {
            this->ptr = NULL;
        }

        return *this;
    }
    inline symbolicTemplateArg_t& operator = ( symbolicTemplateArg_t&& right )
    {
        this->releasePointer();

        this->type = right.type;
        this->ptr = right.ptr;

        right.ptr = NULL;

        return *this;
    }

    enum class eType    
    {
        TYPE,
        LITERAL
    };

    eType type = eType::TYPE;
    symbolicTemplateSpec_t *ptr = NULL;
};

typedef std::vector <symbolicTemplateArg_t> symbolicTemplateParams_t;

// symbolType_t is a holder of several type suits.
struct symbolTypeSuit_t abstract
{
    virtual ~symbolTypeSuit_t( void )
    {
        return;
    }

    virtual symbolTypeSuit_t* CloneTypeSuit( void ) const = 0;

    virtual void makeConstant( void ) = 0;
    virtual bool isConstant( void ) const = 0;
    virtual bool isComplicated( void ) const = 0;
};

struct symbolType_t : public symbolParsePoint_t, public symbolicTemplateSpec_t
{
    inline symbolType_t( void )
    {
        this->typeSuit = NULL;
    }
    inline symbolType_t( symbolTypeSuit_t *typeSuit )
    {
        // Be sure that typeSuit is allocated on the heap!
        this->typeSuit = typeSuit;
    }

    template <typename typeSuitSpecial, typename = typename std::enable_if <std::is_base_of <symbolTypeSuit_t, typeSuitSpecial>::value>::type>
    inline symbolType_t( typeSuitSpecial&& value )
    {
        this->typeSuit = new typeSuitSpecial( std::move( value ) );
    }

    inline symbolType_t( const symbolType_t& right ) = delete;
    inline symbolType_t( symbolType_t&& right )
        : symbolParsePoint_t( std::move( right ) ),
          symbolicTemplateSpec_t( std::move( right ) )
    {
        this->typeSuit = right.typeSuit;
        right.typeSuit = NULL;
    }

    inline symbolType_t makeAttributeClone( void ) const
    {
        symbolTypeSuit_t *typeSuitOut = NULL;
        
        if ( symbolTypeSuit_t *suit = this->typeSuit )
        {
            typeSuitOut = suit->CloneTypeSuit();
        }
        
        return typeSuitOut;
    }

    symbolicTemplateSpec_t* Clone( void ) const override
    {
        return new symbolType_t( this->makeAttributeClone() );
    }

    symbolType_t ResolveParsePoint( void ) const override
    {
        return this->makeAttributeClone();
    }

    bool isConstant( void ) const override
    {
        if ( symbolTypeSuit_t *typeSuit = this->typeSuit )
        {
            return typeSuit->isConstant();
        }

        return false;
    }

    void makeConstant( void )
    {
        if ( symbolTypeSuit_t *typeSuit = this->typeSuit )
        {
            typeSuit->makeConstant();
        }

        throw mangle_parse_error();
    }

    typedef std::vector <symbolType_t> symbolTypes_t;

private:
    inline void clearSuit( void )
    {
        if ( symbolTypeSuit_t *suit = this->typeSuit )
        {
            delete suit;

            this->typeSuit = NULL;
        }
    }

public:
    inline ~symbolType_t( void )
    {
        clearSuit();
    }

    inline symbolType_t& operator = ( const symbolType_t& right ) = delete;
    inline symbolType_t& operator = ( symbolType_t&& right )
    {
        clearSuit();

        symbolParsePoint_t::operator = ( std::move( right ) );
        symbolicTemplateSpec_t::operator = ( std::move( right ) );

        this->typeSuit = right.typeSuit;
        right.typeSuit = NULL;
        
        return *this;
    }

    // Must be assigned by type runtime.
    symbolTypeSuit_t *typeSuit = NULL;
};

// Calling conventions.
enum class eSymbolCallConv
{
    UNKNOWN,
    CDECL,
    STDCALL,
    FASTCALL,
    THISCALL
};

struct symbolTypeSuit_function_t : public symbolTypeSuit_t
{
    inline symbolTypeSuit_function_t( void ) = default;
    inline symbolTypeSuit_function_t( const symbolTypeSuit_function_t& right ) = delete;
    inline symbolTypeSuit_function_t( symbolTypeSuit_function_t&& right ) = default;

    inline symbolTypeSuit_function_t& operator = ( const symbolTypeSuit_function_t& right ) = delete;
    inline symbolTypeSuit_function_t& operator = ( symbolTypeSuit_function_t&& right ) = default;

    symbolTypeSuit_t* CloneTypeSuit( void ) const override
    {
        symbolTypeSuit_function_t suitOut;

        suitOut.returnType = this->returnType.makeAttributeClone();

        for ( const symbolType_t& type : this->parameters )
        {
            suitOut.parameters.push_back( type.makeAttributeClone() );
        }
        
        return new symbolTypeSuit_function_t( std::move( suitOut ) );
    }

    bool isConstant( void ) const override
    {
        return false;
    }

    void makeConstant( void ) override
    {
        throw mangle_parse_error();
    }

    bool isComplicated( void ) const override
    {
        return true;
    }

    symbolType_t returnType;
    symbolType_t::symbolTypes_t parameters;
    eSymbolCallConv callConv = eSymbolCallConv::UNKNOWN;
};

struct symbolTypeSuit_array_t : public symbolTypeSuit_t
{
    inline symbolTypeSuit_array_t( void ) = default;
    inline symbolTypeSuit_array_t( const symbolTypeSuit_array_t& right ) = delete;
    inline symbolTypeSuit_array_t( symbolTypeSuit_array_t&& right ) = default;

    inline symbolTypeSuit_array_t& operator = ( const symbolTypeSuit_array_t& right ) = delete;
    inline symbolTypeSuit_array_t& operator = ( symbolTypeSuit_array_t&& right ) = default;

    symbolTypeSuit_t* CloneTypeSuit( void ) const override
    {
        symbolTypeSuit_array_t suitOut;

        suitOut.typeOfItem = this->typeOfItem.makeAttributeClone();
        suitOut.hasIndex = this->hasIndex;
        suitOut.sizeOfArray = this->sizeOfArray;

        return new symbolTypeSuit_array_t( std::move( suitOut ) );
    }

    bool isConstant( void ) const override
    {
        return this->typeOfItem.isConstant();
    }

    void makeConstant( void ) override
    {
        this->typeOfItem.makeConstant();
    }

    bool isComplicated( void ) const override
    {
        return true;
    }

    // Wraps a type into an array declaration.
    symbolType_t typeOfItem;
    bool hasIndex;
    unsigned long sizeOfArray;
};

// Built-in operator types for convenience.
enum class eOperatorType
{
    CONSTRUCTOR,
    DESTRUCTOR,
    NEW,
    NEW_ARRAY,
    DELETE,
    DELETE_ARRAY,
    OR,
    AND,
    NEG,
    XOR,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,         
    REMAINDER,      // %
    ASSIGN,
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    MULTIPLY_ASSIGN,
    DIVIDE_ASSIGN,
    REMAINDER_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_SHIFT_ASSIGN,
    RIGHT_SHIFT_ASSIGN,
    EQUALITY,
    INEQUALITY,
    LESS_THAN,
    GREATER_THAN,
    LESSEQ_THAN,
    GREATEREQ_THAN,
    NOT,
    LOGICAL_AND,
    LOGICAL_OR,
    INCREMENT,
    DECREMENT,
    COMMA,
    POINTER_RESOLUTION,     // ->*
    POINTER,                // ->
    ROUND_BRACKETS,
    SQUARE_BRACKETS,
    QUESTIONMARK,
    SIZEOF,
    CAST_TO,
    MAKE_POINTER,           // * (unary)
    MAKE_REFERENCE          // & (unary)
};

struct symbolicNamespace_t : public symbolParsePoint_t
{
    inline symbolicNamespace_t( void ) = default;

    inline symbolicNamespace_t( symbolicNamespace_t&& right )
        : symbolParsePoint_t( std::move( right ) ),
          nsType( std::move( right.nsType ) ),
          name( std::move( right.name ) ),
          opType( std::move( right.opType ) ), opCastToType( std::move( opCastToType ) ),
          templateArgs( std::move( right.templateArgs ) )
    {
        this->moveParentResolveFrom( std::move( right ) );
    }
    inline symbolicNamespace_t( const symbolicNamespace_t& right ) = delete;

private:
    inline void unlinkParentResolve( void )
    {
        if ( symbolicNamespace_t *parentResolve = this->parentResolve )
        {
            parentResolve->resolvedBy = NULL;

            this->parentResolve = NULL;
        }
    }

    inline void moveParentResolveFrom( symbolicNamespace_t&& right )
    {
        symbolicNamespace_t *resolvedBy = right.resolvedBy;

        right.resolvedBy = NULL;

        if ( resolvedBy )
        {
            resolvedBy->parentResolve = this;
        }

        this->resolvedBy = resolvedBy;

        symbolicNamespace_t *parentResolve = right.parentResolve;

        right.parentResolve = NULL;

        if ( parentResolve )
        {
            parentResolve->resolvedBy = this;
        }

        this->parentResolve = parentResolve;
    }

public:
    inline ~symbolicNamespace_t( void )
    {
        this->unlinkParentResolve();
    }

    inline symbolicNamespace_t& operator = ( symbolicNamespace_t&& right )
    {
        this->unlinkParentResolve();

        symbolParsePoint_t::operator = ( std::move( right ) );

        this->nsType = std::move( right.nsType );
        this->name = std::move( right.name );
        this->opType = std::move( right.opType );
        this->opCastToType = std::move( right.opCastToType );
        this->templateArgs = std::move( right.templateArgs );

        this->moveParentResolveFrom( std::move( right ) );

        return *this;
    }
    inline symbolicNamespace_t& operator = ( const symbolicNamespace_t& right ) = delete;

    inline void setParentResolve( symbolicNamespace_t& ns )
    {
        this->unlinkParentResolve();

        this->parentResolve = &ns;
        
        assert( ns.resolvedBy == NULL );

        ns.resolvedBy = this;
    }

    inline symbolicNamespace_t* getResolvedBy( void ) const
    {
        return this->resolvedBy;
    }

    inline symbolicNamespace_t makeAttributeClone( void ) const
    {
        symbolicNamespace_t ns;
        ns.nsType = this->nsType;
        ns.name = this->name;
        ns.opType = this->opType;
        ns.opCastToType = this->opCastToType.makeAttributeClone();
        ns.templateArgs = this->templateArgs;
        return ns;
    }

    enum class eType
    {
        NAME,
        OPERATOR
    };

    eType nsType = eType::NAME;

    std::string name;
    eOperatorType opType;
    symbolType_t opCastToType;      // valid if opType == CAST_TO

    // Each namespace entry can have a template parameter list attached to it.
    symbolicTemplateParams_t templateArgs;

private:
    // Namespace resolution path.
    symbolicNamespace_t *parentResolve = NULL;
    symbolicNamespace_t *resolvedBy = NULL;

public:
    symbolType_t ResolveParsePoint( void ) const override;

    bool isConstant( void ) const override
    {
        return false;
    }

    typedef std::vector <symbolicNamespace_t> symbolicNamespaces_t;
};

struct symbolTypeSuit_regular_t : public symbolTypeSuit_t
{
    inline symbolTypeSuit_regular_t( void ) = default;

    inline symbolTypeSuit_regular_t( symbolTypeSuit_regular_t&& right )
        : isConst( right.isConst ),
          valueType( right.valueType ), extTypeName( std::move( right.extTypeName ) ),
          valueQual( right.valueQual ), extQualName( std::move( right.extQualName ) )
    {
        this->subtype = right.subtype;
        right.subtype = NULL;
    }
    inline symbolTypeSuit_regular_t( const symbolTypeSuit_regular_t& right )
        : isConst( right.isConst ),
          valueType( right.valueType ),
          valueQual( right.valueQual ), extQualName( right.extQualName )
    {
        // We are kind of cheating here, but whatever.
        for ( const symbolicNamespace_t& ns : right.extTypeName )
        {
            this->extTypeName.push_back( ns.makeAttributeClone() );
        }

        // Clone the subtype.
        if ( symbolType_t *subtype = right.subtype )
        {
            this->subtype = new symbolType_t( subtype->makeAttributeClone() );
        }
        else
        {
            this->subtype = NULL;
        }
    }

private:
    inline void clearSubtype( void )
    {
        if ( symbolType_t *subtype = this->subtype )
        {
            delete subtype;

            this->subtype = NULL;
        }
    }

public:
    inline ~symbolTypeSuit_regular_t( void )
    {
        clearSubtype();
    }

    symbolTypeSuit_t* CloneTypeSuit( void ) const override
    {
        return new symbolTypeSuit_regular_t( *this );
    }

    void makeConstant( void )
    {
        this->isConst = true;
    }

    bool isConstant( void ) const override
    {
        return this->isConst;
    }

    bool isComplicated( void ) const override
    {
        return
            this->valueQual != eSymbolTypeQualifier::VALUE ||
            this->valueType == eSymbolValueType::CUSTOM ||
            this->isConst;
    }

    inline symbolTypeSuit_regular_t& operator = ( symbolTypeSuit_regular_t&& right )
    {
        this->~symbolTypeSuit_regular_t();

        return *new (this) symbolTypeSuit_regular_t( std::move( right ) );
    }
    inline symbolTypeSuit_regular_t& operator = ( const symbolTypeSuit_regular_t& right ) = delete;

    bool isConst = false;
    eSymbolValueType valueType = eSymbolValueType::VOID;
    symbolicNamespace_t::symbolicNamespaces_t extTypeName;      // valid if CUSTOM.
    eSymbolTypeQualifier valueQual = eSymbolTypeQualifier::VALUE;
    std::string extQualName;        // valid if CUSTOM.

    // We could have a subtype.
    symbolType_t *subtype = NULL;
};

// Model for a C++ symbol name.
struct ProgFunctionSymbol
{
    inline ProgFunctionSymbol( void ) = default;

    inline ProgFunctionSymbol( ProgFunctionSymbol&& right ) = default;
    inline ProgFunctionSymbol( const ProgFunctionSymbol& right ) = default;

    inline ProgFunctionSymbol& operator = ( ProgFunctionSymbol&& right ) = default;
    inline ProgFunctionSymbol& operator = ( const ProgFunctionSymbol& right ) = default;

    eSymbolValueType returnType = eSymbolValueType::UNKNOWN;
    symbolicNamespace_t::symbolicNamespaces_t returnTypeCustomName;

    eSymbolCallConv callingConv = eSymbolCallConv::UNKNOWN;

    symbolicNamespace_t::symbolicNamespaces_t namespaces;
    symbolType_t::symbolTypes_t arguments;

    // Method qualifiers.
    bool hasConstQualifier = false;

    // Supported mangling schemes.
    enum class eManglingType
    {
        GCC,
        VISC
    };

    // Mangling transcoding API.
    bool ParseMangled( const char *codecString );
    bool OutputMangled( eManglingType type, std::string& mangledOut );
};

#endif //_MANGLING_TOOLS_HEADER_
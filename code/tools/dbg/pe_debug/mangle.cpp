// Compiler name mangling transformation tools.

#include "mangle.h"

#include <sdk/MacroUtils.h>

#include <cctype>
#include <sstream>

// Helpful docs:
//  https://github.com/gchatelet/gcc_cpp_mangling_documentation
//  http://www.agner.org/optimize/calling_conventions.pdf
//  http://demangler.com/
//  http://www.int0x80.gr/papers/name_mangling.pdf

// GCC mangling operator symbols.
#define GCC_OPSYMB_NEW                  "nw"
#define GCC_OPSYMB_NEW_ARRAY            "na"
#define GCC_OPSYMB_DELETE               "dl"
#define GCC_OPSYMB_DELETE_ARRAY         "da"
#define GCC_OPSYMB_PLUS                 "pl"
#define GCC_OPSYMB_MINUS                "mi"
#define GCC_OPSYMB_NEG                  "co"
#define GCC_OPSYMB_MULTIPLY             "ml"
#define GCC_OPSYMB_DIVIDE               "dv"
#define GCC_OPSYMB_REMAINDER            "rm"
#define GCC_OPSYMB_AND                  "an"
#define GCC_OPSYMB_OR                   "or"
#define GCC_OPSYMB_XOR                  "eo"
#define GCC_OPSYMB_ASSIGN               "aS"
#define GCC_OPSYMB_PLUS_ASSIGN          "pL"
#define GCC_OPSYMB_MINUS_ASSIGN         "mI"
#define GCC_OPSYMB_MULTIPLY_ASSIGN      "mL"
#define GCC_OPSYMB_DIVIDE_ASSIGN        "dV"
#define GCC_OPSYMB_REMAINDER_ASSIGN     "rM"
#define GCC_OPSYMB_AND_ASSIGN           "aN"
#define GCC_OPSYMB_OR_ASSIGN            "oR"
#define GCC_OPSYMB_XOR_ASSIGN           "eO"
#define GCC_OPSYMB_LEFT_SHIFT           "ls"
#define GCC_OPSYMB_RIGHT_SHIFT          "rs"
#define GCC_OPSYMB_LEFT_SHIFT_ASSIGN    "lS"
#define GCC_OPSYMB_RIGHT_SHIFT_ASSIGN   "rS"
#define GCC_OPSYMB_EQUALITY             "eq"
#define GCC_OPSYMB_INEQUALITY           "ne"
#define GCC_OPSYMB_LESS_THAN            "lt"
#define GCC_OPSYMB_GREATER_THAN         "gt"
#define GCC_OPSYMB_LESSEQ_THAN          "le"
#define GCC_OPSYMB_GREATEREQ_THAN       "ge"
#define GCC_OPSYMB_NOT                  "nt"
#define GCC_OPSYMB_LOGICAL_AND          "aa"
#define GCC_OPSYMB_LOGICAL_OR           "oo"
#define GCC_OPSYMB_INCREMENT            "pp"
#define GCC_OPSYMB_DECREMENT            "mm"
#define GCC_OPSYMB_COMMA                "cm"
#define GCC_OPSYMB_POINTER_RESOLUTION   "pm"
#define GCC_OPSYMB_POINTER              "pt"
#define GCC_OPSYMB_ROUND_BRACKETS       "cl"
#define GCC_OPSYMB_SQUARE_BRACKETS      "ix"
#define GCC_OPSYMB_QUESTIONMARK         "qu"
#define GCC_OPSYMB_SIZEOF               "st"
#define GCC_OPSYMB_SIZEOF2              "sz"
#define GCC_OPSYMB_CAST                 "cv"
#define GCC_OPSYMB_MAKE_POINTER         "de"
#define GCC_OPSYMB_MAKE_REFERENCE       "ad"

// GCC mangling type sequences.
#define GCC_TYPESYMB_VOID               'v'
#define GCC_TYPESYMB_WCHAR_T            'w'
#define GCC_TYPESYMB_BOOL               'b'
#define GCC_TYPESYMB_CHAR               'c'
#define GCC_TYPESYMB_SIGNED_CHAR        'a'
#define GCC_TYPESYMB_UNSIGNED_CHAR      'h'
#define GCC_TYPESYMB_SHORT              's'
#define GCC_TYPESYMB_UNSIGNED_SHORT     't'
#define GCC_TYPESYMB_INT                'i'
#define GCC_TYPESYMB_UNSIGNED_INT       'j'
#define GCC_TYPESYMB_LONG               'l'
#define GCC_TYPESYMB_UNSIGNED_LONG      'm'
#define GCC_TYPESYMB_LONG_LONG          'x'
#define GCC_TYPESYMB_UNSIGNED_LONG_LONG 'y'
#define GCC_TYPESYMB_INT128             'n'
#define GCC_TYPESYMB_UNSIGNED_INT128    'o'
#define GCC_TYPESYMB_FLOAT              'f'
#define GCC_TYPESYMB_DOUBLE             'd'
#define GCC_TYPESYMB_LONG_DOUBLE        'e'
#define GCC_TYPESYMB_FLOAT128           'g'
#define GCC_TYPESYMB_VARARG             'z'
#define GCC_TYPESYMB_VENDOR             'u'

// GCC mangling type qualifiers.
#define GCC_TYPEQUALSYMB_POINTER        "P"
#define GCC_TYPEQUALSYMB_REFERENCE      "R"
#define GCC_TYPEQUALSYMB_RVAL_REFERENCE "O"
#define GCC_TYPEQUALSYMB_VENDOR         "U"

// MSVC member modifier codes.
#define MSVC_MEMBMOD_PRIV_DEFAULT       'A'
#define MSVC_MEMBMOD_PRIV_FAR           'B'
#define MSVC_MEMBMOD_PRIV_STATIC        'C'
#define MSVC_MEMBMOD_PRIV_STATIC_FAR    'D'
#define MSVC_MEMBMOD_PRIV_VIRTUAL       'E'
#define MSVC_MEMBMOD_PRIV_VIRTUAL_FAR   'F'

#define MSVC_MEMBMOD_PROT_DEFAULT       'I'
#define MSVC_MEMBMOD_PROT_FAR           'J'
#define MSVC_MEMBMOD_PROT_STATIC        'K'
#define MSVC_MEMBMOD_PROT_STATIC_FAR    'L'
#define MSVC_MEMBMOD_PROT_VIRTUAL       'M'
#define MSVC_MEMBMOD_PROT_VIRTUAL_FAR   'N'

#define MSVC_MEMBMOD_PUB_DEFAULT        'Q'
#define MSVC_MEMBMOD_PUB_FAR            'R'
#define MSVC_MEMBMOD_PUB_STATIC         'S'
#define MSVC_MEMBMOD_PUB_STATIC_FAR     'T'
#define MSVC_MEMBMOD_PUB_VIRTUAL        'U'
#define MSVC_MEMBMOD_PUB_VIRTUAL_FAR    'V'

// MSVC storage class codes
#define MSVC_STORAGE_NEAR               'A'
#define MSVC_STORAGE_CONST              'B'
#define MSVC_STORAGE_VOLATILE           'C'
#define MSVC_STORAGE_CONST_VOLATILE     'D'
#define MSVC_STORAGE_FAR                'E'
#define MSVC_STORAGE_CONST_FAR          'F'
#define MSVC_STORAGE_VOLATILE_FAR       'G'
#define MSVC_STORAGE_CONST_VOLATILE_FAR 'H'
#define MSVC_STORAGE_HUGE               'I'

// MSVC call convention codes
#define MSVC_CALLCONV_CDECL             'A'
#define MSVC_CALLCONV_PASCAL            'C'
#define MSVC_CALLCONV_THISCALL          'E'
#define MSVC_CALLCONV_STDCALL           'G'
#define MSVC_CALLCONV_FASTCALL          'I'

// MSVC type identifiers
#define MSVC_TYPESYMB_VOID                  "X"
#define MSVC_TYPESYMB_BOOL                  "_N"
#define MSVC_TYPESYMB_CHAR                  "D"
#define MSVC_TYPESYMB_UNSIGNED_CHAR         "E"
#define MSVC_TYPESYMB_SHORT                 "F"
#define MSVC_TYPESYMB_UNSIGNED_SHORT        "G"
#define MSVC_TYPESYMB_INT                   "H"
#define MSVC_TYPESYMB_UNSIGNED_INT          "I"
#define MSVC_TYPESYMB_LONG                  "J"
#define MSVC_TYPESYMB_UNSIGNED_LONG         "K"
#define MSVC_TYPESYMB_LONG_LONG             "_J"
#define MSVC_TYPESYMB_UNSIGNED_LONG_LONG    "_K"
#define MSVC_TYPESYMB_WCHAR_T               "_W"
#define MSVC_TYPESYMB_FLOAT                 "M"
#define MSVC_TYPESYMB_DOUBLE                "N"
#define MSVC_TYPESYMB_LONG_DOUBLE           "O"
#define MSVC_TYPESYMB_VARARG                "Z"

#define MSVC_TYPESYMB_UNKNOWN               "_P"

// MSVC operator sequences.
#define MSVC_OPSYMB_CONSTRUCTOR             "0"
#define MSVC_OPSYMB_DESTRUCTOR              "1"
#define MSVC_OPSYMB_SQUARE_BRACKETS         "A"
#define MSVC_OPSYMB_ROUND_BRACKETS          "R"
#define MSVC_OPSYMB_POINTER                 "C"
#define MSVC_OPSYMB_INCREMENT               "E"
#define MSVC_OPSYMB_DECREMENT               "F"
#define MSVC_OPSYMB_NEW                     "2"
#define MSVC_OPSYMB_NEW_ARRAY               "_U"
#define MSVC_OPSYMB_DELETE                  "3"
#define MSVC_OPSYMB_DELETE_ARRAY            "_V"
#define MSVC_OPSYMB_MAKE_POINTER            "D"
#define MSVC_OPSYMB_MAKE_REFERENCE          "I"
#define MSVC_OPSYMB_NOT                     "7"
#define MSVC_OPSYMB_NEG                     "S"
#define MSVC_OPSYMB_POINTER_RESOLUTION      "J"
#define MSVC_OPSYMB_MULTIPLY                "J"
#define MSVC_OPSYMB_DIVIDE                  "K"
#define MSVC_OPSYMB_REMAINDER               "L"
#define MSVC_OPSYMB_PLUS                    "H"
#define MSVC_OPSYMB_MINUS                   "G"
#define MSVC_OPSYMB_LEFT_SHIFT              "6"
#define MSVC_OPSYMB_RIGHT_SHIFT             "5"
#define MSVC_OPSYMB_LESS_THAN               "M"
#define MSVC_OPSYMB_GREATER_THAN            "O"
#define MSVC_OPSYMB_LESSEQ_THAN             "N"
#define MSVC_OPSYMB_GREATEREQ_THAN          "P"
#define MSVC_OPSYMB_EQUALITY                "8"
#define MSVC_OPSYMB_INEQUALITY              "9"
#define MSVC_OPSYMB_AND                     "I"
#define MSVC_OPSYMB_OR                      "U"
#define MSVC_OPSYMB_XOR                     "T"
#define MSVC_OPSYMB_LOGICAL_AND             "V"
#define MSVC_OPSYMB_LOGICAL_OR              "W"
#define MSVC_OPSYMB_ASSIGN                  "4"
#define MSVC_OPSYMB_MULTIPLY_ASSIGN         "X"
#define MSVC_OPSYMB_DIVIDE_ASSIGN           "_0"
#define MSVC_OPSYMB_REMAINDER_ASSIGN        "_1"
#define MSVC_OPSYMB_PLUS_ASSIGN             "Y"
#define MSVC_OPSYMB_MINUS_ASSIGN            "Z"
#define MSVC_OPSYMB_LEFT_SHIFT_ASSIGN       "_3"
#define MSVC_OPSYMB_RIGHT_SHIFT_ASSIGN      "_2"
#define MSVC_OPSYMB_AND_ASSIGN              "_4"
#define MSVC_OPSYMB_OR_ASSIGN               "_5"
#define MSVC_OPSYMB_XOR_ASSIGN              "_6"
#define MSVC_OPSYMB_COMMA                   "Q"
#define MSVC_OPSYMB_CAST_TO                 "B"

symbolType_t symbolicNamespace_t::ResolveParsePoint( void ) const
{
    // TODO: maybe make this more complicated.
    if ( this->nsType != eType::NAME )
    {
        throw mangle_parse_error();
    }

    // Get the deepest parse node.
    const symbolicNamespace_t *deepestNode = this;

    while ( const symbolicNamespace_t *prevSpace = deepestNode->parentResolve )
    {
        deepestNode = prevSpace;
    }

    // We return a basic type declaring us.
    symbolTypeSuit_regular_t valueBasic;
    valueBasic.valueType = eSymbolValueType::CUSTOM;
    
    // Go up the deepest node again.
    while ( true )
    {
        bool breakNow = ( deepestNode == this );

        assert( deepestNode != NULL );

        valueBasic.extTypeName.push_back( deepestNode->makeAttributeClone() );

        if ( breakNow )
        {
            break;
        }

        deepestNode = deepestNode->resolvedBy;
    }

    return valueBasic;
}

static AINLINE bool stracquire( const char*& strIn, const char *compWith )
{
    const char *str = strIn;

    while ( true )
    {
        char left = *str;
        char right = *compWith++;

        if ( right == 0 )
        {
            break;
        }

        str++;
        
        if ( left == 0 )
        {
            return false;
        }

        if ( left != right )
            return false;
    }

    strIn = str;
    return true;
}

static AINLINE bool _ParseMangleNumeric( const char*& streamIn, unsigned long& numOut )
{
    char c = *streamIn;

    if ( std::isdigit( c ) )
    {
        std::string numstr; 

        // Named definition.
        do
        {
            numstr += c;

            c = *++streamIn;

        } while ( std::isdigit( c ) );

        // Parse the count.
        numOut = std::stoul( numstr );
        return true;
    }
    
    return false;
}

static AINLINE bool _ParseMangleBoundString( const char*& streamIn, std::string& nameOut )
{
    unsigned long countName;
                
    if ( _ParseMangleNumeric( streamIn, countName ) )
    {
        // Read the name encoded string.
        std::string namestr;

        while ( countName-- )
        {
            char c = *streamIn;

            if ( c == 0 )
                throw mangle_parse_error();

            streamIn++;

            namestr += c;
        }

        nameOut = std::move( namestr );
        return true;
    }

    return false;
}

static inline symbolType_t BrowseSubstitution(
    unsigned long substIndex, 
    const SymbolCollection& collection
)
{
    LIST_FOREACH_BEGIN( symbolParsePoint_t, collection.parsePoints.root, parseNode )

        // If we are a constant value, we first try the non-constant portion.
        if ( item->isConstant() )
        {
            if ( substIndex == 0 )
            {
                symbolType_t symbOut = item->ResolveParsePoint();

                assert( symbOut.typeSuit != NULL );

                symbOut.typeSuit->makeConstant();
                return symbOut;
            }

            substIndex--;
        }

        if ( substIndex == 0 )
        {
            return item->ResolveParsePoint();
        }

        substIndex--;               

    LIST_FOREACH_END
    
    // We fail.
    throw mangle_parse_error();
}

// Special browsing routine used in namespace resolution.
static inline symbolicNamespace_t BrowseSubstitutionNamespace(
    unsigned long substIndex, 
    const SymbolCollection& collection
)
{
    LIST_FOREACH_BEGIN( symbolParsePoint_t, collection.parsePoints.root, parseNode )

        // We only count namespaces.
        if ( symbolicNamespace_t *nsEntry = dynamic_cast <symbolicNamespace_t*> ( item ) )
        {
            if ( substIndex == 0 )
            {
                return nsEntry->makeAttributeClone();
            }

            substIndex--;
        }

    LIST_FOREACH_END
    
    // We fail.
    throw mangle_parse_error();
}

// Forward declaration.
static inline symbolType_t _GCCParseMangledSymbolType(
    const char*& gccStream,
    SymbolCollection& collection
);

static AINLINE bool _GCCParseTypeInstancing(
    const char*& gccStream,
    SymbolCollection& collection,
    symbolicTemplateParams_t& argsOut
)
{
    if ( stracquire( gccStream, "I" ) )
    {
        symbolicTemplateParams_t templateArgs;

        bool isInLiteralMode = false;

        while ( true )
        {
            char c = *gccStream;

            if ( c == 0 )
                throw mangle_parse_error();

            if ( c == 'E' )
            {
                gccStream++;

                if ( isInLiteralMode )
                {
                    isInLiteralMode = false;
                }
                else
                {
                    break;
                }
            }
            else if ( stracquire( gccStream, "L" ) )
            {
                if ( isInLiteralMode )
                    throw mangle_parse_error();

                isInLiteralMode = true;
            }
            else
            {
                symbolicTemplateArg_t argPut;

                if ( isInLiteralMode )
                {
                    argPut.type = symbolicTemplateArg_t::eType::LITERAL;

                    symbolicLiteral_t litOut;

                    char typechar = *gccStream;

#define GCC_LITSYMB_HELPER( name_id ) \
    if ( typechar == GCC_TYPESYMB_##name_id ) \
    { \
        litOut.literalType = eSymbolValueType::##name_id##; \
        gccStream++; \
    }

                    // We can scan a lot of literal types.
                         GCC_LITSYMB_HELPER( VOID )
                    else GCC_LITSYMB_HELPER( WCHAR_T )
                    else GCC_LITSYMB_HELPER( BOOL )
                    else GCC_LITSYMB_HELPER( CHAR )
                    else GCC_LITSYMB_HELPER( UNSIGNED_CHAR )
                    else GCC_LITSYMB_HELPER( SHORT )
                    else GCC_LITSYMB_HELPER( UNSIGNED_SHORT )
                    else GCC_LITSYMB_HELPER( INT )
                    else GCC_LITSYMB_HELPER( UNSIGNED_INT )
                    else GCC_LITSYMB_HELPER( LONG )
                    else GCC_LITSYMB_HELPER( UNSIGNED_LONG )
                    else GCC_LITSYMB_HELPER( LONG_LONG )
                    else GCC_LITSYMB_HELPER( UNSIGNED_LONG_LONG )
                    else GCC_LITSYMB_HELPER( INT128 )
                    else GCC_LITSYMB_HELPER( UNSIGNED_INT128 )
                    else GCC_LITSYMB_HELPER( FLOAT )
                    else GCC_LITSYMB_HELPER( DOUBLE )
                    else GCC_LITSYMB_HELPER( LONG_DOUBLE )
                    else GCC_LITSYMB_HELPER( FLOAT128 )
                    else GCC_LITSYMB_HELPER( VARARG )
                    else
                    {
                        // Unknown literal symbol.
                        throw mangle_parse_error();
                    }

                    // Next we parse the literal value.
                    unsigned long litVal;

                    bool gotValue = _ParseMangleNumeric( gccStream, litVal );

                    if ( !gotValue )
                    {
                        throw mangle_parse_error();
                    }

                    litOut.literalValue = litVal;

                    // Store it.
                    argPut.ptr = new symbolicLiteral_t( std::move( litOut ) );
                }
                else
                {
                    argPut.type = symbolicTemplateArg_t::eType::TYPE;

                    symbolType_t typeOut = _GCCParseMangledSymbolType( gccStream, collection );

                    argPut.ptr = new symbolType_t( std::move( typeOut ) );
                }

                // Register it.
                templateArgs.push_back( std::move( argPut ) );
            }
        }

        argsOut = std::move( templateArgs );
        return true;
    }

    return false;
}

static AINLINE bool _GCCParseOneOperator(
    const char*& gccStream,
    eOperatorType& opTypeOut, symbolType_t& castToOut,
    SymbolCollection& collection
)
{
#define GCC_OPSYMB_HELPER( symb_id ) \
    if ( stracquire( gccStream, GCC_OPSYMB_##symb_id ) ) \
    { \
        opTypeOut = eOperatorType::##symb_id##; \
        return true; \
    }

         GCC_OPSYMB_HELPER( NEW )
    else GCC_OPSYMB_HELPER( NEW_ARRAY )
    else GCC_OPSYMB_HELPER( DELETE )
    else GCC_OPSYMB_HELPER( DELETE_ARRAY )
    else GCC_OPSYMB_HELPER( OR )
    else GCC_OPSYMB_HELPER( AND )
    else GCC_OPSYMB_HELPER( NEG )
    else GCC_OPSYMB_HELPER( XOR )
    else GCC_OPSYMB_HELPER( PLUS )
    else GCC_OPSYMB_HELPER( MINUS )
    else GCC_OPSYMB_HELPER( MULTIPLY )
    else GCC_OPSYMB_HELPER( DIVIDE )
    else GCC_OPSYMB_HELPER( REMAINDER )
    else GCC_OPSYMB_HELPER( ASSIGN )
    else GCC_OPSYMB_HELPER( PLUS_ASSIGN )
    else GCC_OPSYMB_HELPER( MINUS_ASSIGN )
    else GCC_OPSYMB_HELPER( MULTIPLY_ASSIGN )
    else GCC_OPSYMB_HELPER( DIVIDE_ASSIGN )
    else GCC_OPSYMB_HELPER( REMAINDER_ASSIGN )
    else GCC_OPSYMB_HELPER( AND_ASSIGN )
    else GCC_OPSYMB_HELPER( OR_ASSIGN )
    else GCC_OPSYMB_HELPER( XOR_ASSIGN )
    else GCC_OPSYMB_HELPER( LEFT_SHIFT )
    else GCC_OPSYMB_HELPER( RIGHT_SHIFT )
    else GCC_OPSYMB_HELPER( LEFT_SHIFT_ASSIGN )
    else GCC_OPSYMB_HELPER( RIGHT_SHIFT_ASSIGN )
    else GCC_OPSYMB_HELPER( EQUALITY )
    else GCC_OPSYMB_HELPER( INEQUALITY )
    else GCC_OPSYMB_HELPER( LESS_THAN )
    else GCC_OPSYMB_HELPER( GREATER_THAN )
    else GCC_OPSYMB_HELPER( LESSEQ_THAN )
    else GCC_OPSYMB_HELPER( GREATEREQ_THAN )
    else GCC_OPSYMB_HELPER( NOT )
    else GCC_OPSYMB_HELPER( LOGICAL_AND )
    else GCC_OPSYMB_HELPER( LOGICAL_OR )
    else GCC_OPSYMB_HELPER( INCREMENT )
    else GCC_OPSYMB_HELPER( DECREMENT )
    else GCC_OPSYMB_HELPER( COMMA )
    else GCC_OPSYMB_HELPER( POINTER_RESOLUTION )
    else GCC_OPSYMB_HELPER( POINTER )
    else GCC_OPSYMB_HELPER( ROUND_BRACKETS )
    else GCC_OPSYMB_HELPER( SQUARE_BRACKETS )
    else GCC_OPSYMB_HELPER( SIZEOF )
    else GCC_OPSYMB_HELPER( MAKE_POINTER )
    else GCC_OPSYMB_HELPER( MAKE_REFERENCE )
    else if ( stracquire( gccStream, GCC_OPSYMB_CAST ) )
    {
        opTypeOut = eOperatorType::CAST_TO;

        // We need to read the type aswell.
        castToOut = _GCCParseMangledSymbolType( gccStream, collection );

        return true;
    }
    
    return false;
}

static AINLINE void _GCCTryNameTemplateInstancing(
    const char*& gccStream, SymbolCollection& symbCollect,
    symbolicNamespace_t& ns
)
{
    symbolicTemplateParams_t templateArgs;

    if ( _GCCParseTypeInstancing( gccStream, symbCollect, templateArgs ) )
    {
        ns.templateArgs = std::move( templateArgs );
    }
}

static AINLINE bool _GCCParseShortcutToken(
    const char*& gccStream, SymbolCollection& collection,
    symbolicNamespace_t::symbolicNamespaces_t& nsOut
)
{
    if ( stracquire( gccStream, "t" ) )
    {
        // Actually an entry of the standard namespace.
        std::string nameIntoStdNamespace;

        bool gotName = _ParseMangleBoundString( gccStream, nameIntoStdNamespace );

        if ( !gotName )
        {
            throw mangle_parse_error();
        }

        symbolicNamespace_t::symbolicNamespaces_t stdNS;

        symbolicNamespace_t stdEntry;
        stdEntry.name = "std";

        symbolicNamespace_t memberEntry;
        memberEntry.name = std::move( nameIntoStdNamespace );

        stdEntry.setParentResolve( memberEntry );

        stdNS.push_back( std::move( stdEntry ) );

        // We register as substitution.
        collection.RegisterParsePoint( memberEntry );

        _GCCTryNameTemplateInstancing( gccStream, collection, memberEntry );

        stdNS.push_back( std::move( memberEntry ) );

        nsOut = std::move( stdNS );
        return true;
    }
    else if ( stracquire( gccStream, "a" ) )
    {
        // Shortcut for allocator.
        
        symbolicNamespace_t::symbolicNamespaces_t stdNS;

        symbolicNamespace_t stdEntry;
        stdEntry.name = "std";

        symbolicNamespace_t memberEntry;
        memberEntry.name = "allocator";

        stdEntry.setParentResolve( memberEntry );

        stdNS.push_back( std::move( stdEntry ) );

        // Also register here.
        collection.RegisterParsePoint( memberEntry );

        _GCCTryNameTemplateInstancing( gccStream, collection, memberEntry );

        stdNS.push_back( std::move( memberEntry ) );

        nsOut = std::move( stdNS );
        return true;
    }

    return false;
}

static AINLINE bool _GCCParseNamespacePath(
    const char*& gccStream, SymbolCollection& symbCollect,
    symbolicNamespace_t::symbolicNamespaces_t& nsOut,
    bool& isConstNamespaceOut
)
{
    // Check for special qualifiers.
    if ( stracquire( gccStream, "N" ) == false )
        return false;

    // Check for further qualifier depending on class-things.
    bool isConstNamespace = false;

    if ( stracquire( gccStream, "K" ) )
    {
        isConstNamespace = true;
    }

    // Read the namespace names.
    symbolicNamespace_t::symbolicNamespaces_t namespaces;

    bool shouldLinkLastNamespace = false;
    bool shouldRegisterLastNamespace = false;
    bool requiresTemplateInstancingCheck = false;

    // Check for STD namespace begin.
    if ( stracquire( gccStream, "S" ) )
    {
        if ( _GCCParseShortcutToken( gccStream, symbCollect, namespaces ) )
        {
            // We started off with a STD namespace, which is fine.
            shouldLinkLastNamespace = true;
        }
        else
        {
            // Reset stream.
            gccStream--;
        }
    }

    while ( true )
    {
        char c = *gccStream;

        if ( c == 0 )
            throw mangle_parse_error();

        if ( requiresTemplateInstancingCheck )
        {
            symbolicNamespace_t& lastNS = namespaces.back();

            // Maybe we have template parameters to attach...?
            _GCCTryNameTemplateInstancing( gccStream, symbCollect, lastNS );

            requiresTemplateInstancingCheck = false;

            // Remember to update the char!
            c = *gccStream;

            if ( c == 0 )
                throw mangle_parse_error();
        }

        // If we are a class namespace thing, we get terminated by a special symbol.
        if ( c == 'E' )
        {
            gccStream++;
            break;
        }

        bool gotNamespace = false;
        symbolicNamespace_t ns;

        // If we previously registered a namespace, we should remember it for resolution.
        // This will register all such namespaces other than the last.
        if ( shouldRegisterLastNamespace )
        {
            symbolicNamespace_t& lastNS = namespaces.back();

            shouldRegisterLastNamespace = false;

            // Register it.
            symbCollect.RegisterParsePoint( lastNS );
        }

        if ( shouldLinkLastNamespace )
        {
            symbolicNamespace_t& lastNS = namespaces.back();

            shouldLinkLastNamespace = false;

            // Register the resolution link.
            lastNS.setParentResolve( ns );
        }

        std::string namestr;
                
        if ( _ParseMangleBoundString( gccStream, namestr ) )
        {
            // Add this namespace entry.
            ns.nsType = symbolicNamespace_t::eType::NAME;
            ns.name = std::move( namestr );

            gotNamespace = true;
        }
        else
        {
            // Some other kind of entry, could be an operator descriptor.
            eOperatorType opType;
            symbolType_t castToType;

            if ( _GCCParseOneOperator( gccStream, opType, castToType, symbCollect ) )
            {
                ns.nsType = symbolicNamespace_t::eType::OPERATOR;
                ns.opType = opType;
                ns.opCastToType = std::move( castToType );

                gotNamespace = true;
            }
            else
            {
                // Special operators.
                if ( stracquire( gccStream, "C" ) )
                {
                    // Make sure we have namespace entries.
                    if ( namespaces.empty() )
                        throw mangle_parse_error();

                    ns.nsType = symbolicNamespace_t::eType::OPERATOR;
                    ns.opType = eOperatorType::CONSTRUCTOR;

                    // There should be a number, no idea why.
                    // I think it is for versioning of C++, because the only
                    // requirement for symbol assignment is case sensitive
                    // symbol-string comparison.
                    unsigned long unkNum;

                    _ParseMangleNumeric( gccStream, unkNum );

                    gotNamespace = true;
                }
                else if ( stracquire( gccStream, "D" ) )
                {
                    // Destructor.
                    if ( namespaces.empty() )
                        throw mangle_parse_error();

                    ns.nsType = symbolicNamespace_t::eType::OPERATOR;
                    ns.opType = eOperatorType::DESTRUCTOR;

                    // Ignore revision number.
                    unsigned long unkNum;

                    _ParseMangleNumeric( gccStream, unkNum );

                    gotNamespace = true;
                }
                else
                {
                    // We have very limited substitution support here.
                    if ( stracquire( gccStream, "S" ) )
                    {
                        unsigned long substIndex = 0;

                        if ( _ParseMangleNumeric( gccStream, substIndex ) )
                        {
                            substIndex++;
                        }

                        if ( stracquire( gccStream, "_" ) == false )
                        {
                            throw mangle_parse_error();
                        }

                        // Browse for a substitution namespace definition.
                        symbolicNamespace_t substance = BrowseSubstitutionNamespace( substIndex, symbCollect );

                        // If there is a chain of resolved-by, we add all of them as namespace tokens, since they are finalized if added.
                        {
                            size_t curCount = namespaces.size();

                            symbolicNamespace_t *resolvedBy = substance.getResolvedBy();

                            while ( resolvedBy )
                            {
                                symbolicNamespace_t subEntry;
                                subEntry.nsType = symbolicNamespace_t::eType::NAME;
                                subEntry.name = std::move( resolvedBy->name );

                                namespaces.insert( namespaces.begin() + curCount, std::move( subEntry ) );
                            }
                        }

                        // We take its attributes.
                        ns.nsType = symbolicNamespace_t::eType::NAME;
                        ns.name = std::move( substance.name );

                        gotNamespace = true;
                    }
                }

                // TODO: add more.
            }
        }

        if ( !gotNamespace )
        {
            // If we are a class namespace, this is not good.
            throw mangle_parse_error();
        }

        // Remember to register last namespace.
        shouldRegisterLastNamespace = true;

        // After each namespace there can be template arguments, check for them.
        requiresTemplateInstancingCheck = true;

        // Have to create a resolution chain of namespaces.
        shouldLinkLastNamespace = true;

        namespaces.push_back( std::move( ns ) );

        // Done with single namespace
    }

    // Done reading namespaces.
    // We must have at least one namespace.
    if ( namespaces.empty() )
        throw mangle_parse_error();

    // Return stuff.
    nsOut = std::move( namespaces );
    isConstNamespaceOut = isConstNamespace;
    return true;
}

static inline symbolType_t _GCCParseMangledSymbolType(
    const char*& gccStream, SymbolCollection& collection
)
{
    symbolType_t typeOut( NULL );

    // Are we constant?
    bool isConstant = false;

    if ( stracquire( gccStream, "K" ) )
    {
        isConstant = true;
    }

    // Parse a type in the stream, one by one.
    {
        // Check if we are a substitution.
        // Then we have to return a type we already created before.
        if ( stracquire( gccStream, "S" ) )
        {
            symbolicNamespace_t::symbolicNamespaces_t shortcutNS;

            if ( _GCCParseShortcutToken( gccStream, collection, shortcutNS ) )
            {
                // Output this symbol normally, since STD compression is just a special feature
                // of GCC mangling.
                symbolTypeSuit_regular_t std_suit;
                std_suit.isConst = isConstant;
                std_suit.valueType = eSymbolValueType::CUSTOM;
                std_suit.extTypeName = std::move( shortcutNS );

                typeOut.typeSuit = new symbolTypeSuit_regular_t( std::move( std_suit ) );
            }
            else
            {
                // Parse a number that we will use as index.
                unsigned long indexNum = 0;

                bool gotNumeric = _ParseMangleNumeric( gccStream, indexNum );

                if ( gotNumeric )
                {
                    // In that case we increase by one.
                    indexNum++;
                }

                // We need to end with a special symbol.
                bool gotSpecSymb = stracquire( gccStream, "_" );

                if ( !gotSpecSymb )
                {
                    throw mangle_parse_error();
                }

                // We index the already available types.
                typeOut = BrowseSubstitution( indexNum, collection );
            }
        }
        else
        {
            // There are multiple suits of types.
            if ( stracquire( gccStream, "F" ) )
            {
                if ( *gccStream == 0 )
                    throw mangle_parse_error();

                // We found a function type.
                // It starts with the return type and then the remainder (until end marker)
                // are all the parameters.

                symbolTypeSuit_function_t suitOut;

                suitOut.returnType = _GCCParseMangledSymbolType( gccStream, collection );

                while ( true )
                {
                    char c = *gccStream;

                    if ( c == 0 )
                        throw mangle_parse_error();

                    if ( c == 'E' )
                    {
                        gccStream++;
                        break;
                    }

                    symbolType_t paramType = _GCCParseMangledSymbolType( gccStream, collection );

                    suitOut.parameters.push_back( std::move( paramType ) );
                }

                // I guess GCC does not care about calling conventions.

                // Return it.
                typeOut.typeSuit = new symbolTypeSuit_function_t( std::move( suitOut ) );
            }
            else if ( stracquire( gccStream, "A" ) )
            {
                // Array suit.
                symbolTypeSuit_array_t suitOut;

                unsigned long arraySize;

                bool hasArraySize = _ParseMangleNumeric( gccStream, arraySize );

                if ( hasArraySize )
                {
                    suitOut.sizeOfArray = arraySize;
                }

                bool hasTerminator = stracquire( gccStream, "_" );

                if ( !hasTerminator )
                {
                    throw mangle_parse_error();
                }

                suitOut.hasIndex = hasArraySize;

                suitOut.typeOfItem = _GCCParseMangledSymbolType( gccStream, collection );

                // Return it.
                typeOut.typeSuit = new symbolTypeSuit_array_t( std::move( suitOut ) );
            }
            else
            {
                // By default we have the "regular" suit.
                symbolTypeSuit_regular_t suitOut;

                suitOut.isConst = isConstant;

                // Check for a valid type prefix.
                eSymbolTypeQualifier qual = eSymbolTypeQualifier::VALUE;

                if ( stracquire( gccStream, GCC_TYPEQUALSYMB_POINTER ) )
                {
                    qual = eSymbolTypeQualifier::POINTER;
                }
                else if ( stracquire( gccStream, GCC_TYPEQUALSYMB_REFERENCE ) )
                {
                    qual = eSymbolTypeQualifier::REFERENCE;
                }
                else if ( stracquire( gccStream, GCC_TYPEQUALSYMB_RVAL_REFERENCE ) )
                {
                    qual = eSymbolTypeQualifier::RVAL_REFERENCE;
                }

                suitOut.valueQual = qual;

                // If we are not a value qualifier, then we have to have a subtype.
                if ( qual != eSymbolTypeQualifier::VALUE )
                {
                    symbolType_t subtype = _GCCParseMangledSymbolType( gccStream, collection );

                    suitOut.subtype = new symbolType_t( std::move( subtype ) );
                }
                else
                {
                    // Else we are a type ourselves.
                    char typechar = *gccStream;

#define GCC_TYPESYMB_HELPER( name_id ) \
    if ( typechar == GCC_TYPESYMB_##name_id ) \
    { \
        suitOut.valueType = eSymbolValueType::##name_id; \
        gccStream++; \
    }

                         GCC_TYPESYMB_HELPER( VOID )
                    else GCC_TYPESYMB_HELPER( WCHAR_T )
                    else GCC_TYPESYMB_HELPER( BOOL )
                    else GCC_TYPESYMB_HELPER( CHAR )
                    else if ( typechar == 'a' )
                    {
                        suitOut.valueType = eSymbolValueType::CHAR;
                        gccStream++;
                    }
                    else GCC_TYPESYMB_HELPER( UNSIGNED_CHAR )
                    else GCC_TYPESYMB_HELPER( SHORT )
                    else GCC_TYPESYMB_HELPER( UNSIGNED_SHORT )
                    else GCC_TYPESYMB_HELPER( INT )
                    else GCC_TYPESYMB_HELPER( UNSIGNED_INT )
                    else GCC_TYPESYMB_HELPER( LONG )
                    else GCC_TYPESYMB_HELPER( UNSIGNED_LONG )
                    else GCC_TYPESYMB_HELPER( LONG_LONG )
                    else GCC_TYPESYMB_HELPER( UNSIGNED_LONG_LONG )
                    else GCC_TYPESYMB_HELPER( INT128 )
                    else GCC_TYPESYMB_HELPER( UNSIGNED_INT128 )
                    else GCC_TYPESYMB_HELPER( FLOAT )
                    else GCC_TYPESYMB_HELPER( DOUBLE )
                    else GCC_TYPESYMB_HELPER( LONG_DOUBLE )
                    else GCC_TYPESYMB_HELPER( FLOAT128 )
                    else GCC_TYPESYMB_HELPER( VARARG )
                    else
                    {
                        // If we are not one of the built-in types,
                        // we could be a custom type name!
                        std::string namestr;

                        if ( _ParseMangleBoundString( gccStream, namestr ) )
                        {
                            // Register it.
                            suitOut.valueType = eSymbolValueType::CUSTOM;

                            symbolicNamespace_t ns;
                            ns.name = std::move( namestr );

                            // Check for template params.
                            _GCCTryNameTemplateInstancing( gccStream, collection, ns );

                            suitOut.extTypeName.push_back( std::move( ns ) );
                        }
                        else
                        {
                            // Maybe a full namespace path?
                            symbolicNamespace_t::symbolicNamespaces_t namespaces;
                            bool isConstantNamespace;   // we ignore that because it makes zero sense.

                            if ( _GCCParseNamespacePath( gccStream, collection, namespaces, isConstantNamespace ) )
                            {
                                // Remember that we now accept the last namespace entry too, so register it.
                                collection.RegisterParsePoint( namespaces.back() );

                                suitOut.valueType = eSymbolValueType::CUSTOM;

                                suitOut.extTypeName = std::move( namespaces );
                            }
                            else
                            {
                                // Not supported or unknown.
                                throw mangle_parse_error();
                            }
                        }
                    }
                }

                // Finished processing the type suit, now try putting it into our type.
                typeOut.typeSuit = new symbolTypeSuit_regular_t( std::move( suitOut ) );
            }
        }
    }

    // Process the type suit.
    symbolTypeSuit_t *typeSuit = typeOut.typeSuit;

    assert( typeSuit != NULL );

    // Maybe we have to be made constant?
    if ( isConstant )
    {
        typeSuit->makeConstant();
    }

    // We want to remember complicated types for compressed lookup.
    if ( typeSuit->isComplicated() )
    {
        // Register this encountered symbol.
        collection.RegisterParsePoint( typeOut );
    }

    return typeOut;
}

bool ProgFunctionSymbol::ParseMangled( const char *codecStream )
{
    // Read any stream of mangled-ness into our storage.
    // * GCC.
    try
    {
        // Variable const qual: L
        // Class decl start: N
        // Class decl end: E
        // Class method const qual: K
        // Class constructor: C[:num:]

        const char *gccStream = codecStream;

        // Test the prefix.
        if ( stracquire( gccStream, "_Z" ) )
        {
            char begDec = *gccStream;

            bool isMultiNamespace = false;
            bool isClassMethodConst = false;

            // We need this for the substitution lookup.
            SymbolCollection symbCollect;

            // Read the namespace names.
            symbolicNamespace_t::symbolicNamespaces_t namespaces;

            if ( _GCCParseNamespacePath( gccStream, symbCollect, namespaces, isClassMethodConst ) )
            {
                isMultiNamespace = true;
            }
            else
            {
                std::string namestr;

                if ( _ParseMangleBoundString( gccStream, namestr ) )
                {
                    // Add this namespace entry.
                    symbolicNamespace_t ns;
                    ns.nsType = symbolicNamespace_t::eType::NAME;
                    ns.name = std::move( namestr );

                    namespaces.push_back( std::move( ns ) );
                }
                else
                {
                    // We could also be an operator.
                    eOperatorType opType;
                    symbolType_t opCastToType;

                    if ( _GCCParseOneOperator( gccStream, opType, opCastToType, symbCollect ) )
                    {
                        // Alright.
                        symbolicNamespace_t ns;
                        ns.nsType = symbolicNamespace_t::eType::OPERATOR;
                        ns.opType = std::move( opType );
                        ns.opCastToType = std::move( opCastToType );

                        namespaces.push_back( std::move( ns ) );
                    }
                    else
                    {
                        // Ignore vtable errors, because we focus on function symbols.
                        if ( stracquire( gccStream, "T" ) )
                        {
                            // Do not care.
                            throw mangle_parse_error();
                        }
                        else
                        {
                            // Invalid.
                            throw mangle_parse_error();
                        }
                    }
                }
            }

            // Now we read the parameter types.
            std::vector <symbolType_t> arguments;

            while ( *gccStream != 0 )
            {
                symbolType_t paramType = _GCCParseMangledSymbolType( gccStream, symbCollect );

                // Add it.
                arguments.push_back( std::move( paramType ) );
            }

            // Success!
            this->callingConv = eSymbolCallConv::UNKNOWN;
            this->namespaces = std::move( namespaces );
            this->arguments = std::move( arguments );
            this->hasConstQualifier = isClassMethodConst;

            return true;
        }
    }
    catch( mangle_parse_error& )
    {}

    // * Visual Studio
    {
        //TODO.
    }

    // None detected.
    return false;
}

static char _MSVCHexTranscode( char hexSymb )
{
    switch( hexSymb )
    {
    case '0': return 'A';
    case '1': return 'B';
    case '2': return 'C';
    case '3': return 'D';
    case '4': return 'E';
    case '5': return 'F';
    case '6': return 'G';
    case '7': return 'H';
    case '8': return 'I';
    case '9': return 'J';
    case 'A': return 'K';
    case 'B': return 'L';
    case 'C': return 'M';
    case 'D': return 'N';
    case 'E': return 'O';
    case 'F': return 'P';
    }

    // Something seriously wrong.
    throw mangle_parse_error();
}

// Number encoding.
static void _MSVCEncodeNumber( unsigned long num, std::string& msvcStream )
{
    if ( num >= 1 && num <= 10 )
    {
        char charNum = (char)num;

        msvcStream += ( '0' + ( charNum - 1 ) );
    }
    else
    {
        // Some kind of special-born encoding that replaces '0' to 'F' (hexadecimal)
        // with 'A' to 'P'. Wicked.
        // But we can use the C++ library to help us.
        std::stringstream sstream;
        sstream << std::hex << std::uppercase;
        sstream << num;

        std::string hexString = sstream.str();

        // Offset the things.
        for ( char& c : hexString )
        {
            c = _MSVCHexTranscode( c );
        }

        // Should be properly coded now.
        msvcStream += hexString;

        // We end of things.
        msvcStream += '@';
    }
}

static void _MSVCOutputSymbolType( const symbolType_t& theType, std::string& msvcStream );

static void _MSVCOutputTemplateArgument( const symbolicTemplateArg_t& arg, std::string& msvcStream )
{
    // Can be literal or type.
    symbolicTemplateArg_t::eType argType = arg.type;

    symbolicTemplateSpec_t *ptr = arg.ptr;

    if ( ptr == NULL )
        throw mangle_parse_error();

    if ( argType == symbolicTemplateArg_t::eType::TYPE )
    {
        // Pretty simple, just output it.
        const symbolType_t *symbolType = (const symbolType_t*)ptr;

        _MSVCOutputSymbolType( *symbolType, msvcStream );
    }
    else if ( argType == symbolicTemplateArg_t::eType::LITERAL )
    {
        // Not hard either.
        const symbolicLiteral_t *symbolLiteral = (const symbolicLiteral_t*)ptr;

        msvcStream += "$0";

        _MSVCEncodeNumber( symbolLiteral->literalValue, msvcStream );
    }
    else
    {
        // Something we have to yet specify.
        throw mangle_parse_error();
    }
}

static AINLINE void _MSVCOutputNamespacePath( const symbolicNamespace_t::symbolicNamespaces_t& nspath, std::string& pathOut )
{
    auto revIter = nspath.rbegin();

    while ( revIter != nspath.rend() )
    {
        const symbolicNamespace_t& nitem = *revIter;

        // We kinda depend on the namespace type.
        symbolicNamespace_t::eType nsType = nitem.nsType;

        if ( nsType == symbolicNamespace_t::eType::NAME )
        {
            // Depends on if we are a templated object or not.
            const symbolicTemplateParams_t& templateArgs = nitem.templateArgs;

            if ( templateArgs.empty() == false )
            {
                pathOut += "?$";
                pathOut += nitem.name;
                pathOut += "@";

                // Now we give all template arguments.
                for ( const symbolicTemplateArg_t& arg : templateArgs )
                {
                    _MSVCOutputTemplateArgument( arg, pathOut );
                }

                pathOut += "@";
            }
            else
            {
                pathOut += nitem.name;
                pathOut += "@";
            }
        }
        else if ( nsType == symbolicNamespace_t::eType::OPERATOR )
        {
            pathOut += "?";

            eOperatorType opType = nitem.opType;

#define MSVC_OPSYMB_HELPER( name_id ) \
    if ( opType == eOperatorType::##name_id ) \
    { \
        pathOut += MSVC_OPSYMB_##name_id##; \
    }

                 MSVC_OPSYMB_HELPER( CONSTRUCTOR )
            else MSVC_OPSYMB_HELPER( DESTRUCTOR )
            else MSVC_OPSYMB_HELPER( SQUARE_BRACKETS )
            else MSVC_OPSYMB_HELPER( ROUND_BRACKETS )
            else MSVC_OPSYMB_HELPER( POINTER )
            else MSVC_OPSYMB_HELPER( INCREMENT )
            else MSVC_OPSYMB_HELPER( DECREMENT )
            else MSVC_OPSYMB_HELPER( NEW )
            else MSVC_OPSYMB_HELPER( NEW_ARRAY )
            else MSVC_OPSYMB_HELPER( DELETE )
            else MSVC_OPSYMB_HELPER( DELETE_ARRAY )
            else MSVC_OPSYMB_HELPER( MAKE_POINTER )
            else MSVC_OPSYMB_HELPER( MAKE_REFERENCE )
            else MSVC_OPSYMB_HELPER( NOT )
            else MSVC_OPSYMB_HELPER( NEG )
            else MSVC_OPSYMB_HELPER( POINTER_RESOLUTION )
            else MSVC_OPSYMB_HELPER( MULTIPLY )
            else MSVC_OPSYMB_HELPER( DIVIDE )
            else MSVC_OPSYMB_HELPER( REMAINDER )
            else MSVC_OPSYMB_HELPER( PLUS )
            else MSVC_OPSYMB_HELPER( MINUS )
            else MSVC_OPSYMB_HELPER( LEFT_SHIFT )
            else MSVC_OPSYMB_HELPER( RIGHT_SHIFT )
            else MSVC_OPSYMB_HELPER( LESS_THAN )
            else MSVC_OPSYMB_HELPER( GREATER_THAN )
            else MSVC_OPSYMB_HELPER( LESSEQ_THAN )
            else MSVC_OPSYMB_HELPER( GREATEREQ_THAN )
            else MSVC_OPSYMB_HELPER( EQUALITY )
            else MSVC_OPSYMB_HELPER( INEQUALITY )
            else MSVC_OPSYMB_HELPER( AND )
            else MSVC_OPSYMB_HELPER( OR )
            else MSVC_OPSYMB_HELPER( XOR )
            else MSVC_OPSYMB_HELPER( LOGICAL_AND )
            else MSVC_OPSYMB_HELPER( LOGICAL_OR )
            else MSVC_OPSYMB_HELPER( ASSIGN )
            else MSVC_OPSYMB_HELPER( MULTIPLY_ASSIGN )
            else MSVC_OPSYMB_HELPER( DIVIDE_ASSIGN )
            else MSVC_OPSYMB_HELPER( REMAINDER_ASSIGN )
            else MSVC_OPSYMB_HELPER( PLUS_ASSIGN )
            else MSVC_OPSYMB_HELPER( MINUS_ASSIGN )
            else MSVC_OPSYMB_HELPER( LEFT_SHIFT_ASSIGN )
            else MSVC_OPSYMB_HELPER( RIGHT_SHIFT_ASSIGN )
            else MSVC_OPSYMB_HELPER( AND_ASSIGN )
            else MSVC_OPSYMB_HELPER( OR_ASSIGN )
            else MSVC_OPSYMB_HELPER( XOR_ASSIGN )
            else MSVC_OPSYMB_HELPER( COMMA )
            else MSVC_OPSYMB_HELPER( CAST_TO )
            else
            {
                // Unknown, sadly.
                throw mangle_parse_error();
            }
        }

        revIter++;
    }

    pathOut += "@";
}

static AINLINE char _MSVCGetCallConvSymbol( eSymbolCallConv callingConv )
{
    if ( callingConv == eSymbolCallConv::UNKNOWN )
    {
        return '?';
    }
    else if ( callingConv == eSymbolCallConv::CDECL )
    {
        return MSVC_CALLCONV_CDECL;
    }
    else if ( callingConv == eSymbolCallConv::STDCALL )
    {
        return MSVC_CALLCONV_STDCALL;
    }
    else if ( callingConv == eSymbolCallConv::FASTCALL )
    {
        return MSVC_CALLCONV_FASTCALL;
    }
    else if ( callingConv == eSymbolCallConv::THISCALL )
    {
        return MSVC_CALLCONV_THISCALL;
    }
    else
    {
        throw mangle_parse_error();
    }
}

static void _MSVCProcessFunctionToken( const symbolTypeSuit_function_t *funcTypeSuit, std::string& msvcStream )
{
    // Print out a function declaration in MSVC mangled format.

    msvcStream += "P6";

    eSymbolCallConv callingConv = funcTypeSuit->callConv;

    if ( callingConv == eSymbolCallConv::UNKNOWN )
    {
        msvcStream += MSVC_CALLCONV_CDECL;  // maybe someday we will have this information.
    }
    else
    {
        msvcStream += _MSVCGetCallConvSymbol( callingConv );
    }

    // We do have a return type this time, yay!
    _MSVCOutputSymbolType( funcTypeSuit->returnType, msvcStream );

    // Now all parameters.
    // Luckily we dont have a faggot 'skip-term-symbol' logic this time.
    for ( const symbolType_t& type : funcTypeSuit->parameters )
    {
        _MSVCOutputSymbolType( type, msvcStream );
    }

    // Terminate the stream.
    msvcStream += "@Z";
}

static void _MSVCProcessArrayToken( const symbolTypeSuit_array_t *arrayTypeSuit, std::string& msvcStream )
{
    // The array is a size declaration and following it is the type.

    msvcStream += "Y0"; // for now we only support one-dimensional arrays.

    // In MSVC mode, we must have an index.
    unsigned long index = 0;
    
    if ( arrayTypeSuit->hasIndex )
    {
        index = arrayTypeSuit->sizeOfArray;
    }
    
    _MSVCEncodeNumber( index, msvcStream );

    // Now for the type.
    _MSVCOutputSymbolType( arrayTypeSuit->typeOfItem, msvcStream );
}

AINLINE void _MSVCGetTypeString( eSymbolValueType symbType, const symbolicNamespace_t::symbolicNamespaces_t& customName, std::string& msvcStream )
{
#define MSVC_TYPESYMB_HELPER( name_id ) \
    if ( symbType == eSymbolValueType::##name_id ) \
    { \
        msvcStream += MSVC_TYPESYMB_##name_id; \
    }

         MSVC_TYPESYMB_HELPER( VOID )
    else MSVC_TYPESYMB_HELPER( WCHAR_T )
    else MSVC_TYPESYMB_HELPER( BOOL )
    else MSVC_TYPESYMB_HELPER( CHAR )
    else MSVC_TYPESYMB_HELPER( UNSIGNED_CHAR )
    else MSVC_TYPESYMB_HELPER( SHORT )
    else MSVC_TYPESYMB_HELPER( UNSIGNED_SHORT )
    else MSVC_TYPESYMB_HELPER( INT )
    else MSVC_TYPESYMB_HELPER( UNSIGNED_INT )
    else MSVC_TYPESYMB_HELPER( LONG )
    else MSVC_TYPESYMB_HELPER( UNSIGNED_LONG )
    else MSVC_TYPESYMB_HELPER( LONG_LONG )
    else MSVC_TYPESYMB_HELPER( UNSIGNED_LONG_LONG )
    // no support for INT128
    else MSVC_TYPESYMB_HELPER( FLOAT )
    else MSVC_TYPESYMB_HELPER( DOUBLE )
    else MSVC_TYPESYMB_HELPER( LONG_DOUBLE )
    // no support for FLOAT128
    else MSVC_TYPESYMB_HELPER( VARARG )
    // extension types for debuggers
    else MSVC_TYPESYMB_HELPER( UNKNOWN )
    else if ( symbType == eSymbolValueType::CUSTOM )
    {
        // We assume custom things are always classes.
        msvcStream += "V";

        // Append the namespace to it.
        _MSVCOutputNamespacePath( customName, msvcStream );
    }
    else
    {
        throw mangle_parse_error();
    }
}

static void _MSVCProcessRegularToken( const symbolTypeSuit_regular_t *regTypeSuit, std::string& msvcStream )
{
    // Qualifiers count.
    eSymbolTypeQualifier qualifier = regTypeSuit->valueQual;

    if ( qualifier == eSymbolTypeQualifier::POINTER )
    {
        const symbolType_t *subtype = regTypeSuit->subtype;

        if ( !subtype )
            throw mangle_parse_error();

        symbolTypeSuit_t *subTypeSuit = subtype->typeSuit;

        if ( !subTypeSuit )
            throw mangle_parse_error();

        // Special case for function symbols being pointed at.
        // In MSVC mode function symbols are pointers themselves, so we must 'remove a pointer'
        // by processing the function symbol directly here.
        if ( symbolTypeSuit_function_t *funcTypeSuit = dynamic_cast <symbolTypeSuit_function_t*> ( subTypeSuit ) )
        {
            // Just process directly and shorten out.
            _MSVCProcessFunctionToken( funcTypeSuit, msvcStream );
            return;
        }

        // Special case for array tokens aswell, symmetric to the cause of the function token.
        if ( symbolTypeSuit_array_t *arrayTypeSuit = dynamic_cast <symbolTypeSuit_array_t*> ( subTypeSuit ) )
        {
            // Give it the regular pointer thing.
            msvcStream += "P";
            
            if ( regTypeSuit->isConst )
            {
                msvcStream += "B";
            }
            else
            {
                msvcStream += "A";
            }

            // Once again process directly here.
            _MSVCProcessArrayToken( arrayTypeSuit, msvcStream );
            return;
        }
    }

    bool requiresStorageMod = false;

    if ( qualifier == eSymbolTypeQualifier::VALUE )
    {
        // Under MSVC values go blank.
        // So we just write the type symbol.
        _MSVCGetTypeString( regTypeSuit->valueType, regTypeSuit->extTypeName, msvcStream );
    }
    else if ( qualifier == eSymbolTypeQualifier::POINTER )
    {
        if ( regTypeSuit->isConst )
        {
            msvcStream += "Q";
        }
        else
        {
            msvcStream += "P";
        }

        requiresStorageMod = true;
    }
    else if ( qualifier == eSymbolTypeQualifier::REFERENCE )
    {
        msvcStream += "A";

        requiresStorageMod = true;
    }
    else
    {
        // Unknown or unsupported.
        throw mangle_parse_error();
    }

    if ( requiresStorageMod )
    {
        // Then we have a type that we point at, in which case we resolve its storage requirement here.
        const symbolType_t *pointAtType = regTypeSuit->subtype;

        if ( !pointAtType )
            throw mangle_parse_error();

        if ( pointAtType->isConstant() )
        {
            msvcStream += "B";
        }
        else
        {
            msvcStream += "A";
        }

        // Now go into the subtype parsing.
        _MSVCOutputSymbolType( *pointAtType, msvcStream );
    }
}

static void _MSVCOutputSymbolType( const symbolType_t& theType, std::string& msvcStream )
{
    symbolTypeSuit_t *typeSuit = theType.typeSuit;

    if ( typeSuit == NULL )
    {
        // MSVC supports the "UNKNOWN" type spec.
        msvcStream += MSVC_TYPESYMB_UNKNOWN;
        return;
    }

    // It is really simple to encode regular suited types.
    // So dispatch according to it.
    symbolTypeSuit_regular_t *regTypeSuit = dynamic_cast <symbolTypeSuit_regular_t*> ( typeSuit );

    // Regular values (POD?)
    if ( regTypeSuit )
    {
        _MSVCProcessRegularToken( regTypeSuit, msvcStream );
    }
    else
    {
        // We also have to support functions and up-cast them to pointers.
        if ( symbolTypeSuit_function_t *funcTypeSuit = dynamic_cast <symbolTypeSuit_function_t*> ( typeSuit ) )
        {
            _MSVCProcessFunctionToken( funcTypeSuit, msvcStream );
        }
        else
        {
            // Not supported yet :/
            throw mangle_parse_error();
        }
    }
}

bool ProgFunctionSymbol::OutputMangled( eManglingType type, std::string& mangledOut )
{
    std::string mangleString;

    try
    {
        if ( type == eManglingType::GCC )
        {
            // GNU Compiler Collection mangling system.
        }
        else if ( type == eManglingType::VISC )
        {
            // Microsoft Visual C++ mangling system.
            mangleString += "?";    // heading token.

            // We need to inspect the namespace ourselves.
            if ( this->namespaces.empty() )
                throw mangle_parse_error();

            const symbolicNamespace_t& lastNS = this->namespaces.back();

            // Write namespace location.
            _MSVCOutputNamespacePath( this->namespaces, mangleString );

            // Next up is the namespace visibility thing.
            // For that we do a clever little thing for now.
            eSymbolCallConv callingConv = this->callingConv;

            bool hasFunctionStorageSpec = false;

            if ( callingConv == eSymbolCallConv::THISCALL )
            {
                mangleString += MSVC_MEMBMOD_PUB_DEFAULT;

                hasFunctionStorageSpec = true;
            }
            else
            {
                mangleString += MSVC_MEMBMOD_PUB_STATIC;
            }

            if ( hasFunctionStorageSpec )
            {
                // Now is the storage specifier for this function.
                if ( this->hasConstQualifier )
                {
                    mangleString += MSVC_STORAGE_CONST;
                }
                else
                {
                    mangleString += MSVC_STORAGE_NEAR;
                }
            }

            // Calling convention.
            mangleString += _MSVCGetCallConvSymbol( callingConv );
            
            // Put the return type.
            _MSVCGetTypeString( this->returnType, this->returnTypeCustomName, mangleString );

            // Now we put all parameters.
            // There are some special rules to this.
            bool leaveOutTermSymbol = false;

            const symbolType_t::symbolTypes_t& arguments = this->arguments;

            if ( arguments.size() >= 1 )
            {
                // If we just have a void, we have to leave out the ending mark.
                const symbolType_t& firstType = arguments.front();

                symbolTypeSuit_regular_t *regFirstSymb = dynamic_cast <symbolTypeSuit_regular_t*> ( firstType.typeSuit );

                if ( regFirstSymb )
                {
                    // Are we a void value?
                    if ( regFirstSymb->valueQual == eSymbolTypeQualifier::VALUE &&
                         regFirstSymb->valueType == eSymbolValueType::VOID )
                    {
                        leaveOutTermSymbol = true;
                    }
                }
            }

            // Put out the symbols.
            for ( const symbolType_t& param : arguments )
            {
                _MSVCOutputSymbolType( param, mangleString );
            }

            mangleString += "@";    // Finishing the type list.

            // We do not care about throw declarations, yet.

            // Finish.
            // Sometimes we leave this symbol out.
            if ( !leaveOutTermSymbol )
            {
                mangleString += "Z";
            }

            mangledOut = std::move( mangleString );
            return true;
        }
    }
    catch( mangle_parse_error& )    // Errors mean bad.
    {}

    // Not supported yet.
    return false;
}
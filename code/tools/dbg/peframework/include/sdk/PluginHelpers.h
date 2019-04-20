#ifndef _PLUGIN_HELPERS_
#define _PLUGIN_HELPERS_

// Helper structure to handle plugin dependant struct registration.
template <typename structType, typename factoryType>
class PluginDependantStructRegister
{
    typedef typename factoryType::pluginOffset_t pluginOffset_t;

    factoryType *hostFactory;

    pluginOffset_t structPluginOffset;

    unsigned int pluginId;

    bool hasConstructionInitialized;

public:
    inline PluginDependantStructRegister( factoryType& theFactory, unsigned int pluginId = factoryType::ANONYMOUS_PLUGIN_ID )
    {
        this->hostFactory = NULL;
        this->pluginId = pluginId;
        this->structPluginOffset = factoryType::INVALID_PLUGIN_OFFSET;
        this->hasConstructionInitialized = true;

        RegisterPlugin( theFactory );
    }

    inline PluginDependantStructRegister( unsigned int pluginId = factoryType::ANONYMOUS_PLUGIN_ID )
    {
        this->hostFactory = NULL;
        this->pluginId = pluginId;
        this->structPluginOffset = factoryType::INVALID_PLUGIN_OFFSET;
        this->hasConstructionInitialized = false;
    }

    inline ~PluginDependantStructRegister( void )
    {
        if ( this->hasConstructionInitialized == true )
        {
            this->UnregisterPlugin();
        }
    }

    inline void RegisterPlugin( factoryType& theFactory )
    {
        this->structPluginOffset =
            theFactory.RegisterDependantStructPlugin <structType> ( this->pluginId );

        this->hostFactory = &theFactory;
    }

    inline void UnregisterPlugin( void )
    {
        if ( this->hostFactory && factoryType::IsOffsetValid( this->structPluginOffset ) )
        {
            this->hostFactory->UnregisterPlugin( this->structPluginOffset );
        }
    }

    inline structType* GetPluginStruct( typename factoryType::hostType_t *hostObj )
    {
        return factoryType::RESOLVE_STRUCT <structType> ( hostObj, this->structPluginOffset );
    }

    inline const structType* GetConstPluginStruct( const typename factoryType::hostType_t *hostObj )
    {
        return factoryType::RESOLVE_STRUCT <const structType> ( hostObj, this->structPluginOffset );
    }

    inline bool IsRegistered( void ) const
    {
        return ( factoryType::IsOffsetValid( this->structPluginOffset ) == true );
    }
};

// Helper structure for registering a plugin struct that exists depending on runtime conditions.
// It does otherwise use the same semantics as a dependant struct register.


template <typename hostType, typename factoryType, typename structType, typename metaInfoType>
struct factoryMetaDefault
{
    typedef factoryType factoryType;

    typedef factoryType endingPointFactory_t;
    typedef typename endingPointFactory_t::pluginOffset_t endingPointPluginOffset_t;

    endingPointPluginOffset_t endingPointPluginOffset;

    metaInfoType metaInfo;

    inline void Initialize( hostType *namespaceObj )
    {
        // Register our plugin.
        this->endingPointPluginOffset =
            metaInfo.RegisterPlugin <structType> ( namespaceObj, endingPointFactory_t::ANONYMOUS_PLUGIN_ID );
    }

    inline void Shutdown( hostType *namespaceObj )
    {
        // Unregister our plugin again.
        if ( this->endingPointPluginOffset != endingPointFactory_t::INVALID_PLUGIN_OFFSET )
        {
            endingPointFactory_t& endingPointFactory =
                metaInfo.ResolveFactoryLink( *namespaceObj );

            endingPointFactory.UnregisterPlugin( this->endingPointPluginOffset );
        }
    }
};

// Some really obscure thing I have found no use for yet.
// It can be used to determine plugin registration on factory construction, based on the environment.
template <typename hostType, typename factoryType, typename structType, typename metaInfoType, typename registerConditionalType>
struct factoryMetaConditional
{
    typedef factoryType factoryType;

    typedef factoryType endingPointFactory_t;
    typedef typename endingPointFactory_t::pluginOffset_t endingPointPluginOffset_t;

    endingPointPluginOffset_t endingPointPluginOffset;

    metaInfoType metaInfo;

    inline void Initialize( hostType *namespaceObj )
    {
        endingPointPluginOffset_t thePluginOffset = endingPointFactory_t::INVALID_PLUGIN_OFFSET;

        if ( registerConditionalType::MeetsCondition( namespaceObj ) )
        {
            // Register our plugin.
            thePluginOffset =
                metaInfo.RegisterPlugin <structType> ( namespaceObj, endingPointFactory_t::ANONYMOUS_PLUGIN_ID );
        }

        this->endingPointPluginOffset = thePluginOffset;
    }

    inline void Shutdown( hostType *namespaceObj )
    {
        // Unregister our plugin again.
        if ( this->endingPointPluginOffset != endingPointFactory_t::INVALID_PLUGIN_OFFSET )
        {
            endingPointFactory_t& endingPointFactory =
                metaInfo.ResolveFactoryLink( *namespaceObj );

            endingPointFactory.UnregisterPlugin( this->endingPointPluginOffset );
        }
    }
};

// Helper structure to register a plugin on the interface of another plugin.
template <typename structType, typename factoryMetaType, typename hostFactoryType>
class PluginConnectingBridge
{
public:
    typedef typename factoryMetaType::factoryType endingPointFactory_t;
    typedef typename endingPointFactory_t::pluginOffset_t endingPointPluginOffset_t;

    typedef typename endingPointFactory_t::hostType_t endingPointType_t;

    typedef hostFactoryType hostFactory_t;
    typedef typename hostFactory_t::pluginOffset_t hostPluginOffset_t;

    typedef typename hostFactory_t::hostType_t hostType_t;

private:
    PluginDependantStructRegister <factoryMetaType, hostFactoryType> connectingBridgePlugin;

    typedef factoryMetaType hostFactoryDependantStruct;
    
public:

    inline PluginConnectingBridge( void )
    {
        return;
    }

    inline PluginConnectingBridge( hostFactory_t& hostFactory ) : connectingBridgePlugin( hostFactory )
    {
        return;
    }

    inline void RegisterPluginStruct( hostFactory_t& theFactory )
    {
        connectingBridgePlugin.RegisterPlugin( theFactory );
    }

    inline void UnregisterPluginStruct( void )
    {
        connectingBridgePlugin.UnregisterPlugin();
    }

    inline factoryMetaType* GetMetaStruct( hostType_t *host )
    {
        return connectingBridgePlugin.GetPluginStruct( host );
    }

    inline structType* GetPluginStructFromMetaStruct( factoryMetaType *metaStruct, endingPointType_t *endingPoint )
    {
        return endingPointFactory_t::RESOLVE_STRUCT <structType> ( endingPoint, metaStruct->endingPointPluginOffset );
    }

    inline structType* GetPluginStruct( hostType_t *host, endingPointType_t *endingPoint )
    {
        structType *resultStruct = NULL;
        {
            hostFactoryDependantStruct *metaStruct = GetMetaStruct( host );

            if ( metaStruct )
            {
                resultStruct = GetPluginStructFromMetaStruct( metaStruct, endingPoint );
            }
        }
        return resultStruct;
    }
};

// Helper structure to register a factory type as extension into a factory type.
template <typename whatFactory, typename intoFactory>
struct PluginDependantFactoryRegister
{
    inline PluginDependantFactoryRegister( void )
    {
        this->regFact = NULL;
        this->offset = intoFact::INVALID_PLUGIN_OFFSET;
    }
    
    inline ~PluginDependantFactoryRegister( void )
    {
        // I guess we do not have to unregister ourselves.
        // We cannot guarrantee this safely anyway.
    }

    inline void RegisterPlugin( whatFactory& whatFact, intoFactory& intoFact )
    {
        this->_pluginInterface.constrFact = &whatFact;

        this->offset =
            intoFact.RegisterPlugin( intoFactory::ANONYMOUS_PLUGIN_ID, &this->_pluginInterface );

        this->regFact = &infoFact;
    }

    inline void UnregisterPlugin( void )
    {
        if ( intoFactory::IsOffsetValid( this->offset ) )
        {
            this->regFact->UnregisterPlugin( offset );

            this->offset = intoFactory::INVALID_PLUGIN_OFFSET;
        }
    }

    typedef typename whatFactory::hostType_t regType_t;
    typedef typename intoFactory::hostType_t hostedOn_t;

    inline regType_t* GetPluginStruct( hostedOn_t *obj )
    {
        return intoFactory::RESOLVE_PLUGIN_STRUCT <regType_t> ( obj, this->offset );
    }

    inline const regType_t* GetConstPluginStruct( const hostedOn_t *obj )
    {
        return intoFactory::RESOLVE_PLUGIN_STRUCT <regType_t> ( obj, this->offset );
    }

private:
    typedef typename intoFactory::pluginOffset_t hostedPluginOffset_t;
    typedef typename intoFactory::pluginDescriptorType hostedPluginDesc_t;
    
    struct factConnectionInterface : public intoFactory::pluginInterface
    {
        bool OnPluginConstruct( hostedOn_t *object, hostedPluginOffset_t pluginOffset, hostedPluginDesc_t pluginId ) override
        {
            void *structMem = pluginId.RESOLVE_STRUCT <void> ( object, pluginOffset );

            if ( structMem == NULL )
                return false;

            // Construct the thing.
            bool constrSuccess = this->constrFact.ConstructPlacement( structMem );

            if ( constrSuccess )
            {
                // Initialize the struct.
                regType_t *newObj = (regType_t*)structMem;

                try
                {
                    newObj->Initialize( object );
                }
                catch( ... )
                {
                    this->constrFact.DestroyPlacement( newObj );

                    throw;
                }
            }

            return constrSuccess;
        }

        void OnPluginDestruct( hostedOn_t *object, hostedPluginOffset_t pluginOffset, hostedPluginDesc_t pluginId ) override
        {
            regType_t *constrObj = pluginId.RESOLVE_STRUCT <regType_t> ( object, pluginOffset );

            if ( constrObj == NULL )
                return;

            // First deinitialize the thing.
            {
                constrObj->Shutdown( object );
            }

            // Now destroy it.
            this->constrFact.DestroyPlacement( constrObj );
        }

        bool OnPluginAssign( hostedOn_t *dstObject, const hostedOn_t *srcObject, hostedPluginOffset_t pluginOffset, hostedPluginDesc_t pluginId ) override
        {
            // Assign stuff, I guess.
            regType_t *dstConstrObj = pluginId.RESOLVE_STRUCT <regType_t> ( dstObject, pluginOffset );
            const regType_t *srcConstrObj = pluginId.RESOLVE_STRUCT  <regType_t> ( srcObject, pluginOffset );

            // Do it, eh.
            return this->constrFact.Assign( dstConstrObj, srcConstrObj );
        }

        whatFactory *constrFact;
    };
    factConnectionInterface _pluginInterface;

    intoFactory *regFact;

    hostedPluginOffset_t offset;
};

template <typename typeSysType, typename factoryType, typename factPipelineType>
struct TypeSystemFactoryTypeRegistration
{
    typedef typename typeSysType::systemPointer_t systemPointer_t;

    inline void Initialize( systemPointer_t *sysPtr )
    {
        // Set up the type interface.
        _typeInterface.sysPtr = sysPtr;

        // Register the type.
        typeInfoBase *typeInfo = NULL;

        if ( factoryType *factPtr = factPipelineType::getFactory( sysPtr ) )
        {
            typeSysType& typeSys = factPipelineType::getTypeSystem( sysPtr );

            const char *typeName = factPipelineType::getTypeName();

            typeInfo =
                typeSys.RegisterType( typeName, &this->_typeInterface );
        }

        this->registeredType = typeInfo;
    }

    inline void Shutdown( systemPointer_t *sysPtr )
    {
        // Delete the type.
        if ( typeInfoBase *typeInfo = this->registeredType )
        {
            typeSysType& typeSys = factPipelineType::getTypeSystem( sysPtr );

            typeSys.DeleteType( typeInfo );

            this->registeredType = NULL;
        }
    }

    typedef typename typeSysType::typeInfoBase typeInfoBase;

    inline typeInfoBase* GetType( void )
    {
        return this->registeredType;
    }

private:
    struct factLinkTypeInterface : public typeSysType::typeInterface
    {
        typedef typename factPipelineType::base_constructor base_constructor;
        typedef typename factoryType::hostType_t hostType_t;

        void Construct( void *mem, systemPointer_t *sysPtr, void *constr_params ) const override
        {
            base_constructor constr( sysPtr, constr_params );

            factoryType *factPtr = factPipelineType::getFactory( sysPtr );

            factPtr->ConstructPlacementEx( mem, constr );
        }

        void CopyConstruct( void *mem, const void *srcMem ) const override
        {
            factoryType *factPtr = factPipelineType::getFactory( sysPtr );

            factPtr->ClonePlacement( mem, (const hostType_t*)srcMem );
        }

        void Destruct( void *mem ) const override
        {
            factoryType *factPtr = factPipelineType::getFactory( sysPtr );

            factPtr->DestroyPlacement( (hostType_t*)mem );
        }

        size_t GetTypeSize( systemPointer_t *sysPtr, void *constr_params ) const override
        {
            factoryType *factPtr = factPipelineType::getFactory( sysPtr );

            return factPtr->GetClassSize();
        }

        size_t GetTypeSizeByObject( systemPointer_t *sysPtr, const void *mem ) const override
        {
            factoryType *factPtr = factPipelineType::getFactory( sysPtr );

            return factPtr->GetClassSize();
        }

        systemPointer_t *sysPtr;
    };
    factLinkTypeInterface _typeInterface;

    typeInfoBase *registeredType;
};

namespace factRegPipes
{

// Factory registration pipeline with default base constructor.
template <typename systemPointer_t, typename constrType>
struct defconstr_fact_pipeline_base
{
    systemPointer_t *sysPtr;
    void *constr_params;

    AINLINE defconstr_fact_pipeline_base( systemPointer_t *sysPtr, void *constr_params )
    {
        this->sysPtr = sysPtr;
        this->constr_params = constr_params;
    }

    AINLINE constrType* Construct( void *mem ) const
    {
        return new (mem) constrType( this->sysPtr, this->constr_params );
    }
};

};

#endif //_PLUGIN_HELPERS_
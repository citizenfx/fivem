// Windows NT Portable Executable file loader written by The_GTA.
// Inspired by Bas Timmer's PE loading inside of FiveM.

// Target spec: Revision 10 – June 15, 2016
// https://www.microsoft.com/en-us/download/details.aspx?id=19509

#ifndef _PELOADER_CORE_
#define _PELOADER_CORE_

#include "pestream.h"

#include <sdk/rwlist.hpp>
#include <sdk/MemoryRaw.h>
#include <sdk/MemoryUtils.h>
#include <sdk/MemoryUtils.stream.h>

#include <unordered_map>
#include <map>
#include <set>
#include <functional>

#include "peloader.common.h"

struct PEFile
{
    PEFile( void );
    ~PEFile( void );

    // Please make sure that you do not put any raw pointer relationships or resource ownership into the PEFile class directly.
    // (directly as in put members into PEFile, instead put them in classes as members into PEFile)
    // Otherwise we would have to fully implement the move-construction and move-assignment functions.

    PEFile( const PEFile& right ) = delete;        // not sure if required/wanted or not.
    PEFile( PEFile&& right ) = default;

    PEFile& operator = ( const PEFile& right ) = delete;
    PEFile& operator = ( PEFile&& right ) = default;

    void LoadFromDisk( PEStream *peStream );
    void WriteToStream( PEStream *peStream );

    bool HasRelocationInfo( void ) const;
    bool HasLinenumberInfo( void ) const;
    bool HasDebugInfo( void ) const;

    bool IsDynamicLinkLibrary( void ) const;

    // NOTE THAT: the general PE headers are open here.
    // This does not mean that modifiying them is considered valid.
    // Please do not modify anything that is being used by PEFramework directly for data structuring
    // such as file alignment. A smarter way to abstract access should be considered.

    // DOS information.
    struct DOSStub
    {
        std::uint16_t cblp;
        std::uint16_t cp;
        std::uint16_t crlc;
        std::uint16_t cparhdr;
        std::uint16_t minalloc, maxalloc;
        std::uint16_t ss;
        std::uint16_t sp;
        std::uint16_t csum;
        std::uint16_t ip;
        std::uint16_t cs;
        std::uint16_t lfarlc;
        std::uint16_t ovno;
        std::uint16_t reserved1[4];
        std::uint16_t oemid;
        std::uint16_t oeminfo;
        std::uint16_t reserved2[10];

        // Actual DOS program data.
        std::vector <unsigned char> progData;
    };
    DOSStub dos_data;

    // Start of PE stuff.
    struct PEFileInfo
    {
        std::uint16_t machine_id = 0;
        std::uint32_t timeDateStamp = 0;

        // More meta information.
        bool isExecutableImage = false;
        bool hasLocalSymbols = false;
        bool hasAggressiveTrim = false;
        bool largeAddressAware = false;
        bool bytesReversedLO = false;
        bool madeFor32Bit = true;
        bool removableRunFromSwap = false;
        bool netRunFromSwap = false;
        bool isSystemFile = false;
        bool isDLL = false;
        bool upSystemOnly = false;
        bool bytesReversedHI = false;

        // Other stuff is used for parsing more advanced business.
    };
    PEFileInfo pe_finfo;

    // Executable sections.
    struct PERelocation
    {
        union
        {
            std::uint32_t virtAddr;
            std::uint32_t relocCount;
        };

        std::uint32_t symbolTableIndex;
        std::uint16_t type;
    };

    struct PELinenumber
    {
        union
        {
            std::uint32_t symTableIndex;
            std::uint32_t virtAddr;
        };
        std::uint16_t number;
    };

private:
    struct PESectionMan;
    struct PEDataStream;

public:
    struct PESection
    {
        friend struct PESectionMan;
        friend struct PEDataStream;

        PESection( void );
        PESection( const PESection& right ) = delete;
        PESection( PESection&& right )
            : shortName( std::move( right.shortName ) ), virtualSize( std::move( right.virtualSize ) ),
              virtualAddr( std::move( right.virtualAddr ) ), relocations( std::move( right.relocations ) ),
              linenumbers( std::move( right.linenumbers ) ), chars( std::move( right.chars ) ),
              isFinal( std::move( right.isFinal ) ), dataAlloc( std::move( right.dataAlloc ) ),
              dataRefList( std::move( right.dataRefList ) ), dataAllocList( std::move( right.dataAllocList ) ),
              streamAllocMan( std::move( right.streamAllocMan ) ), stream( std::move( right.stream ) ),
              placedOffsets( std::move( right.placedOffsets ) ), RVAreferalList( std::move( right.RVAreferalList ) )
        {
            // Since I have been writing this, how about a move constructor that allows
            // default-construction of all members but on top of that executes its own constructor body?

            // We keep a list of RVAs that point to us, which needs updating.
            patchSectionPointers();

            // If we belong to a PE image, we must move our node over.
            moveFromOwnerImage( right );
        }
        ~PESection( void );

    private:
        inline void moveFromOwnerImage( PESection& right )
        {
            PESectionMan *ownerImage = right.ownerImage;

            if ( ownerImage )
            {
                this->sectionNode.moveFrom( std::move( right.sectionNode ) );

                right.ownerImage = NULL;
            }

            this->ownerImage = ownerImage;
        }

        inline void unregisterOwnerImage( void )
        {
            if ( PESectionMan *ownerImage = this->ownerImage )
            {
                LIST_REMOVE( this->sectionNode );

                this->ownerImage = NULL;
            }
        }

        inline void patchSectionPointers( void )
        {
            // First we want to fix the allocations that have been made on this section.
            LIST_FOREACH_BEGIN( PESectionAllocation, this->dataAllocList.root, sectionNode )

                item->theSection = this;

            LIST_FOREACH_END

            // Section data references.
            LIST_FOREACH_BEGIN( PESectionReference, this->dataRefList.root, sectionNode )

                item->theSect = this;

            LIST_FOREACH_END

            // Then fix the RVAs that could target us.
            LIST_FOREACH_BEGIN( PEPlacedOffset, this->RVAreferalList.root, targetNode )

                item->targetSect = this;

            LIST_FOREACH_END
        }

    public:
        inline PESection& operator =( const PESection& right ) = delete;
        inline PESection& operator =( PESection&& right )
        {
            // The same default-assignment paradigm could be applied here as
            // for the move constructor.

            this->shortName = std::move( right.shortName );
            this->virtualSize = std::move( right.virtualSize );
            this->virtualAddr = std::move( right.virtualAddr );
            this->relocations = std::move( right.relocations );
            this->linenumbers = std::move( right.linenumbers );
            this->chars = std::move( right.chars );
            this->isFinal = std::move( right.isFinal );
            this->dataAlloc = std::move( right.dataAlloc );
            this->dataRefList = std::move( right.dataRefList );
            this->dataAllocList = std::move( right.dataAllocList );
            this->streamAllocMan = std::move( right.streamAllocMan );
            this->stream = std::move( right.stream );
            this->placedOffsets = std::move( right.placedOffsets );
            this->RVAreferalList = std::move( right.RVAreferalList );

            patchSectionPointers();

            // Update PE image.
            {
                // Make sure we long to no more PE image anymore
                unregisterOwnerImage(),

                // Set us into the new owner image.
                moveFromOwnerImage( right );
            }

            return *this;
        }

        std::string shortName;
    private:
        std::uint32_t virtualSize;
        std::uint32_t virtualAddr;

    public:
        std::vector <PERelocation> relocations;
        std::vector <PELinenumber> linenumbers;

        enum class eAlignment
        {
            BYTES_UNSPECIFIED,
            BYTES_1,
            BYTES_2,
            BYTES_4,
            BYTES_8,
            BYTES_16,
            BYTES_32,
            BYTES_64,
            BYTES_128,
            BYTES_256,
            BYTES_512,
            BYTES_1024,
            BYTES_2048,
            BYTES_4096,
            BYTES_8192
        };

        // Characteristics.
        struct
        {
            bool sect_hasNoPadding;
            bool sect_containsCode;
            bool sect_containsInitData;
            bool sect_containsUninitData;
            bool sect_link_other;
            bool sect_link_info;
            bool sect_link_remove;
            bool sect_link_comdat;
            bool sect_noDeferSpecExcepts;
            bool sect_gprel;
            bool sect_mem_farData;
            bool sect_mem_purgeable;
            bool sect_mem_16bit;
            bool sect_mem_locked;
            bool sect_mem_preload;
        
            eAlignment sect_alignment;

            bool sect_link_nreloc_ovfl;
            bool sect_mem_discardable;
            bool sect_mem_not_cached;
            bool sect_mem_not_paged;
            bool sect_mem_shared;
            bool sect_mem_execute;
            bool sect_mem_read;
            bool sect_mem_write;
        } chars;

    private:
        // Meta-data that we manage.
        // * Allocation status.
        bool isFinal;       // if true then virtualSize is valid.

        typedef InfiniteCollisionlessBlockAllocator <std::uint32_t> sectionSpaceAlloc_t;

    public:
        // Pointer to a PESection that is maintained across lifetime of PESection.
        // If PESection is prematurely destroyed then this reference will be NULLed.
        struct PESectionReference
        {
            friend struct PESection;

            inline PESectionReference( PESection *theSect = NULL )
            {
                this->theSect = theSect;

                if ( theSect )
                {
                    LIST_INSERT( theSect->dataRefList.root, this->sectionNode );
                }

                // No section means not adding to list.
            }

            inline PESectionReference( const PESectionReference& right ) = delete;
            inline PESectionReference( PESectionReference&& right )
            {
                PESection *theSect = right.theSect;

                this->theSect = theSect;

                if ( theSect )
                {
                    // If we have a section, then the node is successfully linked into a section.
                    this->sectionNode.moveFrom( std::move( right.sectionNode ) );

                    // We turn the moved-from object invalid.
                    right.theSect = NULL;
                }
            }

            inline ~PESectionReference( void )
            {
                if ( PESection *theSect = this->theSect )
                {
                    LIST_REMOVE( this->sectionNode );

                    this->theSect = NULL;
                }
            }

            inline PESectionReference& operator = ( const PESectionReference& right ) = delete;
            inline PESectionReference& operator = ( PESectionReference&& right )
            {
                // This is what can be kind of done by default.
                // Reason why C++ allows different is that you can optimize this.
                this->~PESectionReference();

                return *new (this) PESectionReference( std::move( right ) );
            }

            inline PESection* GetSection( void ) const
            {
                return this->theSect;
            }

        protected:
            // Called when unlinking from list in internal process.
            virtual void clearLink( void )
            {
                this->theSect = NULL;
            }

            PESection *theSect;

            RwListEntry <PESectionReference> sectionNode;
        };

        // Pointer into data inside of a PESection.
        struct PESectionDataReference : public PESectionReference
        {
            friend struct PESection;
            friend struct PEDataStream;

            inline PESectionDataReference( void )
            {
                this->sectOffset = 0;
                this->dataSize = 0;
            }

            inline PESectionDataReference( PESection *theSect, std::uint32_t sectOffset, std::uint32_t dataSize = 0 ) : PESectionReference( theSect )
            {
                this->sectOffset = sectOffset;
                this->dataSize = dataSize;
            }

            inline PESectionDataReference( const PESectionDataReference& right ) = delete;
            inline PESectionDataReference( PESectionDataReference&& right )
                : PESectionReference( std::move( right ) ),
                  sectOffset( std::move( right.sectOffset ) ), dataSize( std::move( right.dataSize ) )
            {
                return;
            }

            // We mirror a lot from PESectionAllocation, but remember
            // that the types are fundamentally meant for different purposes.

            inline ~PESectionDataReference( void )      {}

            inline PESectionDataReference& operator = ( const PESectionDataReference& right ) = delete;
            inline PESectionDataReference& operator = ( PESectionDataReference&& right ) = default;

            inline std::uint32_t GetSectionOffset( void ) const
            {
                if ( this->theSect == NULL )
                {
                    return 0;
                }

                return this->sectOffset;
            }

            inline std::uint32_t GetRVA( void ) const
            {
                if ( this->theSect == NULL )
                {
                    // Zero RVA is valid.
                    return 0;
                }

                return this->theSect->ResolveRVA( this->sectOffset );
            }

            inline std::uint32_t GetDataSize( void ) const
            {
                if ( this->theSect == NULL )
                {
                    return 0;
                }

                return this->dataSize;
            }

            void clearLink( void ) override
            {
                PESectionReference::clearLink();

                this->sectOffset = 0;
                this->dataSize = 0;
            }

        private:
            std::uint32_t sectOffset;
            std::uint32_t dataSize;
        };

        // We need RVA finalization patches which come in the form of virtual
        // RVA registrations into a section. This can be used to register a VA or RVA for writing.
        struct PEPlacedOffset
        {
            friend struct PESection;

            enum class eOffsetType
            {
                RVA,
                VA_32BIT,
                VA_64BIT
            };

            PEPlacedOffset( std::uint32_t dataOffset, PESection *targetSect, std::uint32_t offsetIntoSect, eOffsetType offType = eOffsetType::RVA );

            inline PEPlacedOffset( PEPlacedOffset&& right )
            {
                PESection *targetSect = right.targetSect;

                this->dataOffset = right.dataOffset;
                this->targetSect = targetSect;
                this->offsetIntoSect = right.offsetIntoSect;
                this->offsetType = right.offsetType;

                if ( targetSect )
                {
                    this->targetNode.moveFrom( std::move( right.targetNode ) );

                    right.targetSect = NULL;
                }
            }

            inline PEPlacedOffset( const PEPlacedOffset& right ) = delete;

            inline ~PEPlacedOffset( void )
            {
                if ( this->targetSect )
                {
                    LIST_REMOVE( this->targetNode );
                }
            }

            inline PEPlacedOffset& operator =( PEPlacedOffset&& right )
            {
                this->~PEPlacedOffset();

                new (this) PEPlacedOffset( std::move( right ) );

                return *this;
            }
            inline PEPlacedOffset& operator =( const PEPlacedOffset& right ) = delete;

            // Management API.
            void WriteIntoData( PEFile *peImage, PESection *writeSect, std::uint64_t imageBase ) const;

        private:
            std::int32_t dataOffset;        // the offset into the section where the RVA has to be written.
            PESection *targetSect;          // before getting a real RVA the section has to be allocated.
            std::int32_t offsetIntoSect;    // we have to add this to the section placement to get real RVA.

            eOffsetType offsetType;         // what kind of offset we should put

            RwListEntry <PEPlacedOffset> targetNode;    // list node inside target section to keep pointer valid.
        };

        std::vector <PEPlacedOffset> placedOffsets;     // list of all RVAs that are in the data of this section.

        RwList <PEPlacedOffset> RVAreferalList;     // list of all our placed RVAs that refer to this section.

        struct PESectionAllocation
        {
            friend struct PESection;

            // TODO: once we begin differing between PE file version we have to be
            // careful about maintaining allocations.

            inline PESectionAllocation( void )
            {
                this->theSection = NULL;
                this->sectOffset = 0;
                this->dataSize = 0;
            }

            inline PESectionAllocation( PESectionAllocation&& right )
                : sectOffset( std::move( right.sectOffset ) ), dataSize( std::move( right.dataSize ) )
            {
                PESection *newSectionHost = right.theSection;

                this->theSection = newSectionHost;

                if ( newSectionHost )
                {
                    // If the section is final, we do not exist
                    // in the list, because final sections do not have to
                    // know about existing chunks.
                    // Keeping a list would over-complicate things.
                    if ( newSectionHost->isFinal == false )
                    {
                        this->sectionBlock.moveFrom( std::move( right.sectionBlock ) );
                    }

                    // Add to general allocation list.
                    this->sectionNode.moveFrom( std::move( right.sectionNode ) );
                }

                // Invalidate the old section.
                right.theSection = NULL;
            }
            inline PESectionAllocation( const PESectionAllocation& right ) = delete;

        private:
            inline void removeFromSection( void )
            {
                // If we are allocated on a section, we want to remove ourselves.
                if ( PESection *sect = this->theSection )
                {
                    if ( sect->isFinal == false )
                    {
                        // Block remove.
                        sect->dataAlloc.RemoveBlock( &this->sectionBlock );
                    }

                    // General list remove.
                    LIST_REMOVE( this->sectionNode );

                    this->theSection = NULL;
                }
            }

        public:
            inline void operator = ( PESectionAllocation&& right )
            {
                // Actually the same as the destructor does.
                this->removeFromSection();

                new (this) PESectionAllocation( std::move( right ) );
            }
            inline void operator = ( const PESectionAllocation& right ) = delete;

            inline ~PESectionAllocation( void )
            {
                this->removeFromSection();
            }

            // Data-access methods for this allocation
            void WriteToSection( const void *dataPtr, std::uint32_t dataSize, std::int32_t dataOff = 0 );
            
            // For allocating placed RVAs into allocated structs.
            void RegisterTargetRVA(
                std::uint32_t patchOffset, PESection *targetSect, std::uint32_t targetOff,
                PEPlacedOffset::eOffsetType offType = PEPlacedOffset::eOffsetType::RVA
            );
            void RegisterTargetRVA(
                std::uint32_t patchOffset, const PESectionAllocation& targetInfo, std::uint32_t targetOff = 0,
                PEPlacedOffset::eOffsetType offType = PEPlacedOffset::eOffsetType::RVA
            );
            void RegisterTargetRVA(
                std::uint32_t patchOffset, const PESectionDataReference& targetInfo, std::uint32_t targetOff = 0,
                PEPlacedOffset::eOffsetType offType = PEPlacedOffset::eOffsetType::RVA
            );

        private:
            PESection *theSection;
            std::uint32_t sectOffset;
            std::uint32_t dataSize;     // if 0 then true size not important/unknown.

        public:
            inline PESection* GetSection( void ) const          { return this->theSection; }
            inline std::uint32_t GetDataSize( void ) const      { return this->dataSize; }

            inline std::uint32_t ResolveInternalOffset( std::uint32_t offsetInto ) const
            {
                if ( this->theSection == NULL )
                {
                    throw peframework_exception(
                        ePEExceptCode::RUNTIME_ERROR,
                        "attempt to resolve unallocated allocation offset"
                    );
                }

                return ( this->sectOffset + offsetInto );
            }

            inline std::uint32_t ResolveOffset( std::uint32_t offset ) const
            {
                PESection *theSection = this->theSection;

                if ( theSection == NULL )
                {
                    throw peframework_exception(
                        ePEExceptCode::RUNTIME_ERROR,
                        "attempt to resolve unallocated RVA"
                    );
                }

                return theSection->ResolveRVA( this->sectOffset + offset );
            }

            inline bool IsAllocated( void ) const
            {
                return ( theSection != NULL );
            }

            inline PESectionAllocation CloneOnlyFinal( void ) const
            {
                PESection *allocSect = this->theSection;

                if ( allocSect == NULL )
                {
                    return PESectionAllocation();
                }

                if ( allocSect->IsFinal() == false )
                {
                    return PESectionAllocation();
                }

                PEFile::PESectionAllocation newAlloc;
                allocSect->SetPlacedMemoryInline( newAlloc, this->sectOffset, this->dataSize );

                return newAlloc;
            }

            // We can spawn a data reference from any allocation.
            inline operator PESectionDataReference ( void )
            {
                return PESectionDataReference( this->theSection, this->sectOffset, this->dataSize );
            }

			inline PESectionDataReference ToReference(size_t offset, size_t size)
			{
				return PESectionDataReference( this->theSection, this->sectOffset + offset, size );
			}

            // Every allocation can ONLY exist on ONE section.

            sectionSpaceAlloc_t::block_t sectionBlock;

            RwListEntry <PESectionAllocation> sectionNode;  // despite having a collision-based list node we need a general node aswell.

            // Write helpers for native numbers.
#define PESECT_WRITEHELPER( typeName, type ) \
            inline void Write##typeName( type value, std::int32_t dataOff = 0 ) \
            { \
                this->WriteToSection( &value, sizeof(value), dataOff ); \
            }

            PESECT_WRITEHELPER( Int8, std::int8_t );
            PESECT_WRITEHELPER( Int16, std::int16_t );
            PESECT_WRITEHELPER( Int32, std::int32_t );
            PESECT_WRITEHELPER( Int64, std::int64_t );
            PESECT_WRITEHELPER( UInt8, std::uint8_t );
            PESECT_WRITEHELPER( UInt16, std::uint16_t );
            PESECT_WRITEHELPER( UInt32, std::uint32_t );
            PESECT_WRITEHELPER( UInt64, std::uint64_t );
        };

        // API to register RVAs for commit phase.
        void RegisterTargetRVA(
            std::uint32_t patchOffset, PESection *targetSect, std::uint32_t targetOffset,
            PEPlacedOffset::eOffsetType offsetType = PEPlacedOffset::eOffsetType::RVA
        );
        void RegisterTargetRVA(
            std::uint32_t patchOffset, const PESectionAllocation& targetInfo,
            PEPlacedOffset::eOffsetType offsetType = PEPlacedOffset::eOffsetType::RVA
        );

        // General method and initialization.
        void SetPlacementInfo( std::uint32_t virtAddr, std::uint32_t virtSize );

        inline std::uint32_t GetVirtualAddress( void ) const
        {
            if ( this->ownerImage == NULL )
            {
                throw peframework_exception(
                    ePEExceptCode::RUNTIME_ERROR,
                    "attempt to get virtual address from section unbound to image"
                );
            }

            return this->virtualAddr;
        }

        inline std::uint32_t GetVirtualSize( void ) const
        {
            if ( this->isFinal == false )
            {
                throw peframework_exception(
                    ePEExceptCode::RUNTIME_ERROR,
                    "attempt to get virtual size from unfinished section"
                );
            }

            return this->virtualSize;
        }

        inline bool IsFinal( void ) const       { return this->isFinal; }

        // Allocation methods.
        std::uint32_t Allocate( PESectionAllocation& blockMeta, std::uint32_t allocSize, std::uint32_t alignment = sizeof(std::uint32_t) );
        void SetPlacedMemory( PESectionAllocation& blockMeta, std::uint32_t allocOff, std::uint32_t allocSize = 0u );
        void SetPlacedMemoryInline( PESectionAllocation& blockMeta, std::uint32_t allocOff, std::uint32_t allocSize = 0u );

        std::uint32_t ResolveRVA( std::uint32_t sectOffset ) const;

        void SetPENativeFlags( std::uint32_t flags );
        std::uint32_t GetPENativeFlags( void ) const;

        // If we are final, we DO NOT keep a list of allocations.
        // Otherwise we keep a collisionless struct of allocations we made.
        sectionSpaceAlloc_t dataAlloc;

        // List which contains either pure data pointers into section data that will
        // be transformed into PEDataStream access objects or pure references to
        // PESection objects.
        RwList <PESectionReference> dataRefList;

        // List which contains unordered allocated chunks, mostly useful for
        // final sections.
        RwList <PESectionAllocation> dataAllocList;

        inline bool IsEmpty( void ) const
        {
            if ( isFinal )
            {
                return ( this->virtualSize == 0 );
            }
            else
            {
                return ( LIST_EMPTY( this->dataAlloc.blockList.root ) == true ) && ( this->stream.Size() == 0 );
            }
        }

private:
        // Writing and possibly reading from this data section
        // should be done through this memory stream.
        BasicMemStream::basicMemStreamAllocMan <std::int32_t> streamAllocMan;
public:
        typedef BasicMemStream::basicMemoryBufferStream <std::int32_t> memStream;

        memStream stream;

        // Call just before placing into image.
        void Finalize( void );
        void FinalizeProfound( std::uint32_t virtualSize );

        // Node into the list of sections in a PESectionMan.
        RwListEntry <PESection> sectionNode;
        PESectionMan *ownerImage;
    };
    using PEPlacedOffset = PESection::PEPlacedOffset;
    using PESectionReference = PESection::PESectionReference;
    using PESectionDataReference = PESection::PESectionDataReference;
    using PESectionAllocation = PESection::PESectionAllocation;

    struct PEOptHeader
    {
        std::uint8_t majorLinkerVersion = 0;
        std::uint8_t minorLinkerVersion = 0;
        std::uint32_t sizeOfCode = 0;
        std::uint32_t sizeOfInitializedData = 0;
        std::uint32_t sizeOfUninitializedData = 0;
        PESectionDataReference addressOfEntryPointRef;
        std::uint32_t baseOfCode = 0;
        std::uint32_t baseOfData = 0;

        std::uint64_t imageBase = 0;
        std::uint32_t fileAlignment = 0x200;
        std::uint16_t majorOSVersion = 0;
        std::uint16_t minorOSVersion = 0;
        std::uint16_t majorImageVersion = 0;
        std::uint16_t minorImageVersion = 0;
        std::uint16_t majorSubsysVersion = 0;
        std::uint16_t minorSubsysVersion = 0;
        std::uint32_t win32VersionValue = 0;
        std::uint32_t sizeOfImage = 0;
        std::uint32_t sizeOfHeaders = 0;
        std::uint32_t checkSum = 0;
        std::uint16_t subsys = 0;
        std::uint64_t sizeOfStackReserve = 0;
        std::uint64_t sizeOfStackCommit = 0;
        std::uint64_t sizeOfHeapReserve = 0;
        std::uint64_t sizeOfHeapCommit = 0;
        std::uint32_t loaderFlags = 0;

        // DLL flags.
        bool dll_supportsHighEntropy = false;
        bool dll_hasDynamicBase = false;
        bool dll_forceIntegrity = false;
        bool dll_nxCompat = false;
        bool dll_noIsolation = false;
        bool dll_noSEH = false;
        bool dll_noBind = false;
        bool dll_appContainer = false;
        bool dll_wdmDriver = false;
        bool dll_guardCF = false;
        bool dll_termServAware = false;

        // More advanced stuff to follow.
    };
    PEOptHeader peOptHeader;

private:
    // Data inside of a PE file is stored in sections which have special
    // rules if they ought to be "zero padded".
    struct PEDataStream
    {
        inline PEDataStream( void )
        {
            this->accessSection = NULL;
            this->dataOffset = 0;
            this->seek_off = 0;
        }

        inline PEDataStream( PESection *theSection, std::uint32_t dataOffset )
        {
            this->accessSection = theSection;
            this->dataOffset = dataOffset;
            this->seek_off = 0;
        }

        static inline PEDataStream fromDataRef( const PESectionDataReference& dataRef )
        {
            return PEDataStream( dataRef.theSect, dataRef.sectOffset );
        }

        inline void Seek( std::uint32_t offset )
        {
            this->seek_off = offset;
        }

        inline std::uint32_t Tell( void )
        {
            return this->seek_off;
        }

        inline void Read( void *dataBuf, std::uint32_t readCount )
        {
            PESection *theSection = this->accessSection;

            if ( !theSection )
            {
                throw peframework_exception(
                    ePEExceptCode::RUNTIME_ERROR,
                    "attempt to read from invalid PE data stream"
                );
            }

            typedef sliceOfData <std::uint32_t> sectionSlice_t;

            // Get the slice of the present data.
            const std::uint32_t sectVirtualAddr = theSection->virtualAddr;
            const std::uint32_t sectVirtualSize = theSection->virtualSize;

            sectionSlice_t dataSlice( 0, theSection->stream.Size() );

            // Get the slice of the zero padding.
            const std::uint32_t validEndPoint = ( sectVirtualSize );

            sectionSlice_t zeroSlice = sectionSlice_t::fromOffsets( dataSlice.GetSliceEndPoint() + 1, validEndPoint );

            // Now the slice of our read operation.
            sectionSlice_t opSlice( ( this->dataOffset + this->seek_off ), readCount );

            // Begin output to buffer operations.
            char *outputPtr = (char*)dataBuf;

            std::uint32_t totalReadCount = 0;

            // First return the amount of data that was requested, if it counts.
            sectionSlice_t retDataSlice;

            if ( opSlice.getSharedRegion( dataSlice, retDataSlice ) )
            {
                std::uint32_t numReadData = retDataSlice.GetSliceSize();

                const void *srcDataPtr = ( (const char*)theSection->stream.Data() + retDataSlice.GetSliceStartPoint() );

                memcpy( outputPtr, srcDataPtr, numReadData );

                outputPtr += numReadData;

                totalReadCount += numReadData;
            }

            // Next see if we have to return any zeroes.
            if ( opSlice.getSharedRegion( zeroSlice, retDataSlice ) )
            {
                std::uint32_t numZeroes = retDataSlice.GetSliceSize();

                memset( outputPtr, 0, numZeroes );

                outputPtr += numZeroes;

                totalReadCount += numZeroes;
            }

            this->seek_off += readCount;

            if ( totalReadCount != readCount )
            {
                throw peframework_exception(
                    ePEExceptCode::ACCESS_OUT_OF_BOUNDS,
                    "PE file out-of-bounds section read exception"
                );
            }
        }

    private:
        PESection *accessSection;
        std::uint32_t dataOffset;
        std::uint32_t seek_off;
    };

    template <typename charType>
    inline static void ReadPEString(
        PEDataStream& stream, std::basic_string <charType>& strOut
    )
    {
        while ( true )
        {
            charType c;
            stream.Read( &c, sizeof(c) );

            if ( c == '\0' )
            {
                break;
            }

            strOut += c;
        }
    }

    struct PESectionMan
    {
        PESectionMan( std::uint32_t sectionAlignment, std::uint32_t imageBase );
        PESectionMan( const PESectionMan& right ) = delete;
        PESectionMan( PESectionMan&& right );
        ~PESectionMan( void );

        PESectionMan& operator = ( const PESectionMan& right ) = delete;
        PESectionMan& operator = ( PESectionMan&& right );

        // Private section management API.
        PESection* AddSection( PESection&& theSection );
        PESection* PlaceSection( PESection&& theSection );
        bool RemoveSection( PESection *section );

        bool FindSectionSpace( std::uint32_t spanSize, std::uint32_t& addrOut );

        std::uint32_t GetSectionAlignment( void ) const { return this->sectionAlignment; }
        std::uint32_t GetImageBase( void ) const        { return this->imageBase; }

        std::uint32_t GetSectionCount( void ) const     { return this->numSections; }

        inline std::uint32_t GetImageSize( void )
        {
            // Pretty easy to get because we have an address-sorted list of sections.
            std::uint32_t unalignedMemImageEndOffMax = sectAllocSemantics::GetSpanSize( sectVirtualAllocMan );

            return ALIGN_SIZE( unalignedMemImageEndOffMax, this->sectionAlignment );
        }

        // Function to get the section and the offset into it for a RVA.
        template <typename sectLocationProviderType>
        inline bool GetPEDataLocationGeneric( const sectLocationProviderType& sectLocProv, std::uint32_t rvirtAddr, std::uint32_t rvirtSize, std::uint32_t *allocOffOut, PESection **allocSectOut = NULL, std::uint32_t *sectIndexOut = NULL ) const
        {
            typedef sliceOfData <std::uint32_t> memSlice_t;

            // Create a memory slice of the request.
            memSlice_t requestRegion( rvirtAddr, rvirtSize );

            std::uint32_t sectIndex = 0;

            LIST_FOREACH_BEGIN( PESection, this->sectionList.root, sectionNode )
        
                // We only support that for sections whose data is figured out already.
                if ( item->isFinal )
                {
                    // Create a memory slice of this section.
                    std::uint32_t sectAddr, sectSize;
                    {
                        sectAddr = sectLocProv.GetSectionLocation( item, sectIndex );
                        sectSize = item->virtualSize;
                    }

                    memSlice_t sectRegion( sectAddr, sectSize );

                    // Our memory request has to be entirely inside of a section.
                    memSlice_t::eIntersectionResult intResult = requestRegion.intersectWith( sectRegion );

                    if ( intResult == memSlice_t::INTERSECT_INSIDE ||
                         intResult == memSlice_t::INTERSECT_EQUAL )
                    {
                        if ( allocSectOut )
                        {
                            *allocSectOut = item;
                        }

                        if ( allocOffOut )
                        {
                            *allocOffOut = (uint32_t)( rvirtAddr - sectAddr );
                        }

                        if ( sectIndexOut )
                        {
                            *sectIndexOut = sectIndex;
                        }

                        return true;
                    }
                }

                sectIndex++;
        
            LIST_FOREACH_END

            // Not found.
            return false;
        }

    private:
        struct mainSectLocProv
        {
            AINLINE static std::uint32_t GetSectionLocation( const PESection *theSect, std::uint32_t sectIndex )
            {
                return theSect->virtualAddr;
            }
        };

    public:
        inline bool GetPEDataLocation( std::uint32_t rvirtAddr, std::uint32_t *allocOffOut, PESection **allocSectOut = NULL, std::uint32_t *sectIndexOut = NULL ) const
        {
            mainSectLocProv locProv;

            return GetPEDataLocationGeneric( locProv, rvirtAddr, 1, allocOffOut, allocSectOut, sectIndexOut );
        }

        inline bool GetPEDataLocationEx( std::uint32_t rvirtAddr, std::uint32_t rvirtSize, std::uint32_t *allocOffOut, PESection **allocSectOut = NULL, std::uint32_t *sectIndexOut = NULL ) const
        {
            mainSectLocProv locProv;

            return GetPEDataLocationGeneric( locProv, rvirtAddr, rvirtSize, allocOffOut, allocSectOut, sectIndexOut );
        }

        // Function to get a data pointer of data directories.
        template <typename sectLocationProviderType>
        inline bool GetPEDataStreamGeneric(
            const sectLocationProviderType& sectLocProv,
            std::uint32_t rvirtAddr, PEDataStream& streamOut,
            PESection **allocSectOut = NULL
        )
        {
            // We return a stream into a section.
            std::uint32_t offsetIntoSect;
            PESection *allocSect;

            bool gotLocation = GetPEDataLocationGeneric( sectLocProv, rvirtAddr, 1, &offsetIntoSect, &allocSect );

            if ( !gotLocation )
                return false;

            streamOut = PEDataStream( allocSect, offsetIntoSect );

            if ( allocSectOut )
            {
                *allocSectOut = allocSect;
            }

            return true;
        }

        inline bool GetPEDataStream(
            std::uint32_t rvirtAddr, PEDataStream& streamOut,
            PESection **allocSectOut = NULL
        )
        {
            mainSectLocProv locProv;

            return GetPEDataStreamGeneric( locProv, rvirtAddr, streamOut, allocSectOut );
        }

        inline bool ReadPEData(
            std::uint32_t dataOffset, std::uint32_t dataSize,
            void *dataBuf, PESection **sectionOut
        )
        {
            PEDataStream stream;

            bool gotData = GetPEDataStream( dataOffset, stream, sectionOut );

            if ( !gotData )
            {
                return false;
            }

            stream.Read( dataBuf, dataSize );

            return true;
        }

        inline bool ReadPEString(
            std::uint32_t dataOffset, std::string& strOut,
            PESection **sectionOut
        )
        {
            PEDataStream stream;

            bool gotData = GetPEDataStream( dataOffset, stream, sectionOut );

            if ( !gotData )
                return false;

            PEFile::ReadPEString( stream, strOut );
            return true;
        }

        // Convert RVA to section data reference.
        inline PESectionDataReference ResolveRVAToRef( uint32_t rva )
        {
            if ( rva != 0 )
            {
                PESection *theSect;
                std::uint32_t sectOff;

                bool gotLocation = GetPEDataLocation( rva, &sectOff, &theSect );

                if ( !gotLocation )
                {
                    throw peframework_exception(
                        ePEExceptCode::ACCESS_OUT_OF_BOUNDS,
                        "invalid PE relative virtual address resolution"
                    );
                }

                return PESectionDataReference( theSect, sectOff );
            }

            return PESectionDataReference();
        }

    private:
        std::uint32_t sectionAlignment;
        std::uint32_t imageBase;

        struct sectVirtualAllocMan_t
        {
            AINLINE sectVirtualAllocMan_t( void ) = default;
            AINLINE sectVirtualAllocMan_t( const sectVirtualAllocMan_t& right ) = delete;
            AINLINE sectVirtualAllocMan_t( sectVirtualAllocMan_t&& right ) = default;

            AINLINE sectVirtualAllocMan_t& operator = ( const sectVirtualAllocMan_t& right ) = delete;
            AINLINE sectVirtualAllocMan_t& operator = ( sectVirtualAllocMan_t&& right ) = default;

            typedef sliceOfData <decltype(PESection::virtualAddr)> memSlice_t;

            struct blockIter_t
            {
                AINLINE blockIter_t( void )
                {
                    return;
                }

                AINLINE blockIter_t( RwListEntry <PESection>& node )
                {
                    this->node_iter = &node;
                }

                AINLINE void Increment( void )
                {
                    this->node_iter = this->node_iter->next;
                }

            private:
                AINLINE PESection* GetCurrentSection( void ) const
                {
                    return LIST_GETITEM( PESection, this->node_iter, sectionNode );
                }

            public:
                AINLINE memSlice_t GetMemorySlice( void ) const
                {
                    PESection *sect = GetCurrentSection();

                    return memSlice_t( sect->virtualAddr, sect->virtualSize );
                }

                AINLINE PESection* GetNativePointer( void ) const
                {
                    return GetCurrentSection();
                }

                RwListEntry <PESection> *node_iter;
            };

            AINLINE PESectionMan* GetManager( void )
            {
                return (PESectionMan*)( this - offsetof(PESectionMan, sectVirtualAllocMan) );
            }

            AINLINE blockIter_t GetFirstMemoryBlock( void )
            {
                return ( *GetManager()->sectionList.root.next );
            }

            AINLINE blockIter_t GetLastMemoryBlock( void )
            {
                return ( *GetManager()->sectionList.root.prev );
            }

            AINLINE bool HasMemoryBlocks( void )
            {
                return ( LIST_EMPTY( GetManager()->sectionList.root ) == false );
            }

            AINLINE blockIter_t GetRootNode( void )
            {
                return ( GetManager()->sectionList.root );
            }

            AINLINE blockIter_t GetAppendNode( blockIter_t iter )
            {
                return iter;
            }

            AINLINE bool IsEndMemoryBlock( const blockIter_t& iter )
            {
                return ( iter.node_iter == &GetManager()->sectionList.root );
            }

            AINLINE bool IsInAllocationRange( const memSlice_t& memRegion )
            {
                const memSlice_t peFileRegion( 0, std::numeric_limits <std::int32_t>::max() );

                memSlice_t::eIntersectionResult intResult = memRegion.intersectWith( peFileRegion );

                return ( intResult == memSlice_t::INTERSECT_EQUAL || intResult == memSlice_t::INTERSECT_INSIDE );
            }
        };

        sectVirtualAllocMan_t sectVirtualAllocMan;

        typedef FirstPassAllocationSemantics <decltype(PESection::virtualAddr), sectVirtualAllocMan_t> sectAllocSemantics;

    public:
        unsigned int numSections;

        RwList <PESection> sectionList;     // all sections belong to a PEFile MUST have a valid allocation spot.
    };

    PESectionMan sections;

    // We need to know about file-space section allocations.
    // This is mainly used for reflection purposes during PE serialization.
    struct sect_allocInfo
    {
        std::uint32_t alloc_off;
    };

    typedef std::unordered_map <std::uint32_t, sect_allocInfo> sect_allocMap_t;

    struct PEFileSpaceData
    {
        inline PEFileSpaceData( void )
        {
            // We start out without any storage.
            this->storageType = eStorageType::NONE;
        }

        inline PEFileSpaceData( const PEFileSpaceData& right ) = delete;
        inline PEFileSpaceData( PEFileSpaceData&& right ) = default;

        inline PEFileSpaceData& operator = ( const PEFileSpaceData& right ) = delete;
        inline PEFileSpaceData& operator = ( PEFileSpaceData&& right ) = default;

        // Management API.
        void ReadFromFile( PEStream *peStream, const PESectionMan& sections, std::uint32_t rva, std::uint32_t filePtr, std::uint32_t dataSize );

        void ResolveDataPhaseAllocation( std::uint32_t& rvaOut, std::uint32_t& sizeOut ) const;
        std::uint32_t ResolveFinalizationPhase( PEStream *peStream, PEloader::FileSpaceAllocMan& allocMan, const sect_allocMap_t& sectFileAlloc ) const;

        // Call this to check if this storage even needs to be finalized.
        bool NeedsFinalizationPhase( void ) const;

    private:
        struct fileSpaceStreamBufferManager
        {
            inline PEFileSpaceData* GetManager( void )
            {
                return (PEFileSpaceData*)( (char*)this - offsetof(PEFileSpaceData, streamMan) );
            }

            void EstablishBufferView( void*& memPtr, std::int32_t& streamSize, std::int32_t reqSize );
        };

        fileSpaceStreamBufferManager streamMan;

    public:
        typedef memoryBufferStream <std::int32_t, fileSpaceStreamBufferManager, false, false> fileSpaceStream_t;

        // General API about data.
        void ClearData( void );

        // Stream access to this data.
        fileSpaceStream_t OpenStream( bool createNew = false );

    private:
        enum class eStorageType
        {
            SECTION,            // stores data within address space
            FILE,               // stores data appended after the PE file
            NONE                // no storage at all
        };

        eStorageType storageType;
        PESectionAllocation sectRef;    // valid if storageType == SECTION
        std::vector <char> fileRef;     // valid if storageType == FILE
    };

public:
    using fileSpaceStream_t = PEFileSpaceData::fileSpaceStream_t;

    // Generic section management API.
    PESection* AddSection( PESection&& theSection );
    PESection* PlaceSection( PESection&& theSection );
    PESection* FindFirstSectionByName( const char *name );
    PESection* FindFirstAllocatableSection( void );
    PESection* FindSectionByRVA( std::uint32_t rva, std::uint32_t *sectIndexOut = NULL, std::uint32_t *sectOffOut = NULL );
    bool RemoveSection( PESection *section );

    std::uint32_t GetSectionCount( void ) const     { return this->sections.GetSectionCount(); }
    std::uint32_t GetSectionAlignment( void ) const { return this->sections.GetSectionAlignment(); }

    bool FindSectionSpace( std::uint32_t spanSize, std::uint32_t& addrOut );

    void ForAllSections( std::function <void ( PESection* )> cb );
    void ForAllSections( std::function <void ( const PESection* )> cb ) const;

    // More advanced helpers.
    PESectionDataReference ResolveRVAToRef( std::uint32_t rva )     { return this->sections.ResolveRVAToRef( rva ); }

    // Simple helpers.
    template <typename charType>
    static inline PESectionAllocation WriteZeroTermString( PESection& writeSect, const std::basic_string <charType>& string )
    {
        const std::uint32_t writeCount = (std::uint32_t)( string.size() + 1 );

        const std::uint32_t writeSize = ( writeCount * sizeof(charType) );

        PESectionAllocation allocEntry;
        writeSect.Allocate( allocEntry, writeSize, sizeof(charType) );
                        
        allocEntry.WriteToSection( string.c_str(), writeSize );

        return allocEntry;
    }

    // Data directory business.
    struct PEExportDir
    {
        inline PEExportDir( void ) = default;

        inline PEExportDir( const PEExportDir& right ) = delete;
        inline PEExportDir( PEExportDir&& right ) = default;

        inline PEExportDir& operator = ( const PEExportDir& right ) = delete;
        inline PEExportDir& operator = ( PEExportDir&& right ) = default;

        std::uint32_t chars = 0;
        std::uint32_t timeDateStamp = 0;
        std::uint16_t majorVersion = 0;
        std::uint16_t minorVersion = 0;
        std::string name;   // NOTE: name table is serialized lexigraphically.
        std::uint32_t ordinalBase = 0;

        PESectionAllocation nameAllocEntry;

        struct func
        {
            // Exported items can be either forwarders or section RVAs.

            // Mandatory valid fields for each export.
            PESectionDataReference expRef;  // only valid if not a forwarder.
            std::string forwarder;
            bool isForwarder;

            // definition of ordinal: index into function array.
            // thus it is given implicitly.
        };
        std::vector <func> functions;

        // Name map.
        struct mappedName
        {
            std::string name;
            mutable PESectionAllocation nameAllocEntry;

            inline bool operator < ( const std::string& right ) const
            {
                return ( this->name < right );
            }

            inline bool operator < ( const mappedName& right ) const
            {
                return operator < ( right.name );
            }
        };

    private:
        struct _funcNameMapComparator
        {
            inline bool operator()( const mappedName& left, const mappedName& right ) const
            {
                return operator() ( left.name, right.name );
            }

            inline bool operator()( const mappedName& left, const std::string& right ) const
            {
                return operator() ( left.name, right );
            }

            inline bool operator()( const std::string& left, const mappedName& right ) const
            {
                return operator() ( left, right.name );
            }

            inline bool operator()( const std::string& left, const std::string& right ) const
            {
                return ( left < right );
            }

            typedef std::string is_transparent;
        };

    public:
        std::map <mappedName, size_t, _funcNameMapComparator> funcNameMap;

        // Helper API.
        // (all ordinals have to be local to this image ordinal base)
        std::uint32_t AddExport( func&& entryToTakeOver );
        void MapName( std::uint32_t ordinal, const char *name );
        void RemoveExport( std::uint32_t ordinal );

        func* ResolveExport( bool isOrdinal, std::uint32_t ordinal, const std::string& name );

        PESectionAllocation funcAddressAllocEntry;
        PESectionAllocation funcNamesAllocEntry;
        PESectionAllocation funcOrdinalsAllocEntry;

        PESectionAllocation allocEntry;
    };
    PEExportDir exportDir;

    // Import informations.
    struct PEImportDesc
    {
        inline PEImportDesc( void ) = default;
        inline PEImportDesc( const char *moduleName ) : DLLName( moduleName )       {}
        inline PEImportDesc( const PEImportDesc& right ) = delete;
        inline PEImportDesc( PEImportDesc&& right ) = default;

        inline PEImportDesc& operator = ( const PEImportDesc& right ) = delete;
        inline PEImportDesc& operator = ( PEImportDesc&& right ) = default;

        struct importFunc
        {
            std::uint16_t ordinal_hint;
            std::string name;
            bool isOrdinalImport;

            PESectionAllocation nameAllocEntry;
        };

        typedef std::vector <importFunc> functions_t;

        // Query API.
        const importFunc* FindImportEntry( std::uint16_t ordinal_hint, const char *name, bool isOrdinalImport, std::uint32_t *indexOut = NULL ) const;

        // Helper API.
        static functions_t ReadPEImportFunctions( PESectionMan& sections, std::uint32_t rva, PESectionAllocation& allocEntry, bool isExtendedFormat );
        static PESectionAllocation WritePEImportFunctions( PESection& writeSect, const functions_t& functionList, bool isExtendedFormat );

        static void AllocatePEImportFunctionsData( PESection& writeSect, functions_t& functionList );

        static functions_t CreateEquivalentImportsList( const functions_t& right );

        functions_t funcs;
        std::string DLLName;

        PESectionAllocation impNameArrayAllocEntry;
        PESectionAllocation DLLName_allocEntry;
        
        // Meta-information we must keep because it is baked
        // by compilers.
        PESectionDataReference firstThunkRef;
    };
    std::vector <PEImportDesc> imports;

    PESectionAllocation importsAllocEntry;

    // Resource information.
    struct PEResourceItem abstract
    {
        enum class eType
        {
            DIRECTORY,
            DATA
        };

        inline PEResourceItem( eType typeDesc, bool isIdentifierName, std::u16string name, std::uint16_t identifier )
            : itemType( std::move( typeDesc ) ),
              hasIdentifierName( std::move( isIdentifierName ) ), name( std::move( name ) ), identifier( std::move( identifier ) )
        {
            return;
        }

        virtual ~PEResourceItem( void )
        {
            return;
        }

        // Helpers.
        std::wstring GetName( void ) const;

        eType itemType;
        std::u16string name;            // valid if hasIdentifierName == false
        std::uint16_t identifier;       // valid if hasIdentifierName == true
        bool hasIdentifierName;         // if true then identifier field is valid, name is not
    };

    struct PEResourceInfo : public PEResourceItem
    {
        inline PEResourceInfo( bool isIdentifierName, std::u16string name, std::uint16_t identifier, PESectionDataReference dataRef )
            : PEResourceItem( eType::DATA, std::move( isIdentifierName ), std::move( name ), std::move( identifier ) ),
              sectRef( std::move( dataRef ) )
        {
            this->codePage = 0;
            this->reserved = 0;
        }

        // Important rule is that resource allocations are
        // stored in the resource section.
        PESectionDataReference sectRef;     // we link resources to data in sections.
        std::uint32_t codePage;
        std::uint32_t reserved;
    };
    
    struct PEResourceDir : public PEResourceItem
    {
        inline PEResourceDir( bool isIdentifierName, std::u16string name, std::uint16_t identifier )
            : PEResourceItem( eType::DIRECTORY, std::move( isIdentifierName ), std::move( name ), std::move( identifier ) )
        {
            this->characteristics = 0;
            this->timeDateStamp = 0;
            this->majorVersion = 0;
            this->minorVersion = 0;
        }

        inline PEResourceDir( const PEResourceDir& right ) = delete;
        inline PEResourceDir( PEResourceDir&& right ) = default;

        inline ~PEResourceDir( void )
        {
            // We need to destroy all our children, because they are
            // dynamically allocated.
            for ( PEResourceItem *item : this->namedChildren )
            {
                delete item;
            }

            this->namedChildren.clear();

            for ( PEResourceItem *item : this->idChildren )
            {
                delete item;
            }

            this->idChildren.clear();
        }

        inline PEResourceDir& operator = ( const PEResourceDir& right ) = delete;
        inline PEResourceDir& operator = ( PEResourceDir&& right ) = default;

        // Helper API.
        PEResourceItem* FindItem( bool isIdentifierName, const std::u16string& name, std::uint16_t identifier );

        bool AddItem( PEResourceItem *theItem );
        bool RemoveItem( const PEResourceItem *theItem );
        bool IsEmpty( void ) const
        {
            return ( this->namedChildren.empty() && this->idChildren.empty() );
        }

        template <typename callbackType>
        inline void ForAllChildren( const callbackType& cb ) const
        {
            for ( const PEResourceItem *childItem : this->namedChildren )
            {
                cb( childItem, false );
            }

            for ( const PEResourceItem *childItem : this->idChildren )
            {
                cb( childItem, true );
            }
        }

        std::uint32_t characteristics;
        std::uint32_t timeDateStamp;
        std::uint16_t majorVersion;
        std::uint16_t minorVersion;
        
    private:
        struct _compareNamedEntry
        {
            inline bool operator() ( const PEResourceItem *left, const PEResourceItem *right ) const
            {
                return ( left->name < right->name );
            }

            inline bool operator() ( const std::u16string& left, const PEResourceItem *right ) const
            {
                return ( left < right->name );
            }

            inline bool operator() ( const PEResourceItem *left, const std::u16string& right ) const
            {
                return ( left->name < right );
            }

            typedef std::u16string is_transparent;
        };

        struct _compareIDEntry
        {
            inline bool operator() ( const PEResourceItem *left, const PEResourceItem *right ) const
            {
                return ( left->identifier < right->identifier );
            }

            inline bool operator() ( std::uint16_t left, const PEResourceItem *right ) const
            {
                return ( left < right->identifier );
            }

            inline bool operator() ( const PEResourceItem *left, std::uint16_t right ) const
            {
                return ( left->identifier < right );
            }

            typedef std::uint16_t is_transparent;
        };

    public:
        // We contain named and id entries.
        std::set <PEResourceItem*, _compareNamedEntry> namedChildren;
        std::set <PEResourceItem*, _compareIDEntry> idChildren;
    };
    PEResourceDir resourceRoot;
    
    PESectionAllocation resAllocEntry;

    struct PERuntimeFunction
    {
        PESectionDataReference beginAddrRef;
        PESectionDataReference endAddrRef;
        PESectionDataReference unwindInfoRef;
    };
    std::vector <PERuntimeFunction> exceptRFs;

    PESectionAllocation exceptAllocEntry;

    struct PESecurity
    {
        // We just keep the certificate data around for anyone to care about
        PEFileSpaceData certStore;
    };
    PESecurity securityCookie;

    // Base relocations are documented to be per 4K page, so let's take advantage of that.
    static constexpr std::uint32_t baserelocChunkSize = 0x1000;

    struct PEBaseReloc
    {
        inline PEBaseReloc( void ) = default;
        inline PEBaseReloc( const PEBaseReloc& right ) = delete;
        inline PEBaseReloc( PEBaseReloc&& right ) = default;

        inline PEBaseReloc& operator = ( const PEBaseReloc& right ) = delete;
        inline PEBaseReloc& operator = ( PEBaseReloc&& right ) = default;

        std::uint32_t offsetOfReloc;

        enum class eRelocType : std::uint16_t
        {
            ABSOLUTE,
            HIGH,
            LOW,
            HIGHLOW,
            HIGHADJ,
            MACHINE_5,
            RESERVED,
            MACHINE_7,
            MACHINE_8,
            MACHINE_9,
            DIR64
        };

        static inline eRelocType GetRelocTypeForPointerSize( size_t pointerSize )
        {
            if ( pointerSize == 4 )
            {
                return eRelocType::HIGHLOW;
            }
            else if ( pointerSize == 8 )
            {
                return eRelocType::DIR64;
            }

            return eRelocType::ABSOLUTE;
        }

        struct item
        {
            std::uint16_t offset : 12;
            std::uint16_t type : 4;     // had to change this away from enum because GCC is being a bitch
        };
        static_assert( sizeof(item) == sizeof(std::uint16_t), "invalid item size" );

        std::vector <item> items;
    };
    std::map <std::uint32_t, PEBaseReloc> baseRelocs;

    PESectionAllocation baseRelocAllocEntry;

    struct PEDebugDesc
    {
        inline PEDebugDesc( void )
        {
            this->characteristics = 0;
            this->timeDateStamp = 0;
            this->majorVer = 0;
            this->minorVer = 0;
            this->type = 0;
        }

        inline PEDebugDesc( const PEDebugDesc& right ) = delete;
        inline PEDebugDesc( PEDebugDesc&& right ) = default;

        inline PEDebugDesc& operator = ( const PEDebugDesc& right ) = delete;
        inline PEDebugDesc& operator = ( PEDebugDesc&& right ) = default;

        std::uint32_t characteristics;
        std::uint32_t timeDateStamp;
        std::uint16_t majorVer, minorVer;
        std::uint32_t type;             // can be any documented or undocumented value

        PEFileSpaceData dataStore;
    };
    std::vector <PEDebugDesc> debugDescs;

    PESectionAllocation debugDescsAlloc;

    struct PEGlobalPtr
    {
        inline PEGlobalPtr( void )
        {
            this->ptrOffset = 0;
        }

        std::uint32_t ptrOffset;
    };
    PEGlobalPtr globalPtr;

    struct PEThreadLocalStorage
    {
        inline PEThreadLocalStorage( void )
        {
            this->sizeOfZeroFill = 0;
            this->characteristics = 0;
        }

        inline PEThreadLocalStorage( const PEThreadLocalStorage& right ) = delete;
        inline PEThreadLocalStorage( PEThreadLocalStorage&& right ) = default;

        inline PEThreadLocalStorage& operator = ( const PEThreadLocalStorage& right ) = delete;
        inline PEThreadLocalStorage& operator = ( PEThreadLocalStorage&& right ) = default;

        inline bool NeedsWriting( void ) const
        {
            if ( this->startOfRawDataRef.GetRVA() != 0 ||
                 this->endOfRawDataRef.GetRVA() != 0 ||
                 this->addressOfIndexRef.GetRVA() != 0 ||
                 this->addressOfCallbacksRef.GetRVA() != 0 ||
                 this->sizeOfZeroFill != 0 ||
                 this->characteristics != 0 )
            {
                return true;
            }

            return false;
        }

        // For maintenance reasons, we store RVAs in-memory.
        // The serialized PE format actually expects VAs.

        PESectionDataReference startOfRawDataRef;
        PESectionDataReference endOfRawDataRef;
        PESectionDataReference addressOfIndexRef;
        PESectionDataReference addressOfCallbacksRef;
        std::uint32_t sizeOfZeroFill;
        std::uint32_t characteristics;

        PESectionAllocation allocEntry;
    };
    PEThreadLocalStorage tlsInfo;

    struct PELoadConfig
    {
        inline PELoadConfig( void ) = default;

        inline PELoadConfig( const PELoadConfig& right ) = delete;
        inline PELoadConfig( PELoadConfig&& right ) = default;

        inline PELoadConfig& operator = ( const PELoadConfig& right ) = delete;
        inline PELoadConfig& operator = ( PELoadConfig&& right ) = default;

        std::uint32_t timeDateStamp = 0;
        std::uint16_t majorVersion = 0;
        std::uint16_t minorVersion = 0;
        std::uint32_t globFlagsClear = 0;
        std::uint32_t globFlagsSet = 0;
        std::uint32_t critSecDefTimeOut = 0;
        std::uint64_t deCommitFreeBlockThreshold = 0;
        std::uint64_t deCommitTotalFreeThreshold = 0;
        PESectionDataReference lockPrefixTableRef;
        std::uint64_t maxAllocSize = 0;
        std::uint64_t virtualMemoryThreshold = 0;
        std::uint64_t processAffinityMask = 0;
        std::uint32_t processHeapFlags = 0;
        std::uint16_t CSDVersion = 0;
        std::uint16_t reserved1 = 0;
        PESectionDataReference editListRef;
        PESectionDataReference securityCookieRef;
        PESectionDataReference SEHandlerTableRef;
        std::uint64_t SEHandlerCount = 0;
        PESectionDataReference guardCFCheckFunctionPtrRef;
        std::uint64_t reserved2 = 0;
        PESectionDataReference guardCFFunctionTableRef;
        std::uint64_t guardCFFunctionCount = 0;
        std::uint32_t guardFlags = 0;

        bool isNeeded = false;

        PESectionAllocation allocEntry;
    };
    PELoadConfig loadConfig;

    struct PEBoundImport
    {
        inline PEBoundImport( void ) = default;
        inline PEBoundImport( const PEBoundImport& right ) = delete;
        inline PEBoundImport( PEBoundImport&& right ) = default;
        
        inline PEBoundImport& operator = ( const PEBoundImport& right ) = delete;
        inline PEBoundImport& operator = ( PEBoundImport&& right ) = default;

        std::uint32_t timeDateStamp;
        std::string DLLName;

        std::vector <PEBoundImport> forw_bindings;
    };
    std::vector <PEBoundImport> boundImports;
    // boundImports are always written outside of PE sections.

    struct PEThunkIATStore
    {
        inline PEThunkIATStore( void )
        {
            this->thunkDataStart = 0;
            this->thunkDataSize = 0;
        }

        std::uint32_t thunkDataStart;
        std::uint32_t thunkDataSize;
    };
    PEThunkIATStore iatThunkAll;

    struct PEDelayLoadDesc
    {
        // Uses a similar layout to the PEImportDesc data.
        std::uint32_t attrib;
        std::string DLLName;
        PESectionAllocation DLLName_allocEntry;
        PESectionAllocation DLLHandleAlloc;     // just storage (for the NT loader) in the size of a pointer
        PESectionDataReference IATRef;
        PEImportDesc::functions_t importNames;
        PESectionAllocation importNamesAllocEntry;
        PESectionDataReference boundImportAddrTableRef;
        PESectionDataReference unloadInfoTableRef;
        std::uint32_t timeDateStamp;
    };
    std::vector <PEDelayLoadDesc> delayLoads;

    PESectionAllocation delayLoadsAllocEntry;

    struct PECommonLanguageRuntimeInfo
    {
        inline PECommonLanguageRuntimeInfo( void )
        {
            this->dataOffset = 0;
            this->dataSize = 0;
        }

        std::uint32_t dataOffset;
        std::uint32_t dataSize;
    };
    PECommonLanguageRuntimeInfo clrInfo;

    // Meta-data.
    bool isExtendedFormat;  // if true then we are PE32+ format.
    // NOTE: it is (theoretically) valid to travel a 32bit executable in PE32+ format.

    // Relocation API.
    void AddRelocation( std::uint32_t rva, PEBaseReloc::eRelocType relocType );
    void RemoveRelocations( std::uint32_t rva, std::uint32_t regionSize );

    void OnWriteAbsoluteVA( PESection *writeSect, std::uint32_t sectOff, bool is64Bit );

    // Import API.
    PEImportDesc* FindImportDescriptor( const char *moduleName );
    PEImportDesc& EstablishImportDescriptor( const char *moduleName );

    // Debug API.
    PEDebugDesc& AddDebugData( std::uint32_t debugType );
    bool ClearDebugDataOfType( std::uint32_t debugType );

    // Information API.
    inline std::uint64_t GetImageBase( void ) const     { return this->peOptHeader.imageBase; }

    // Helper functions to off-load the duty work from the main
    // serialization function.
    // Could actually be required by outside code because of PEStructures.
    std::uint16_t GetPENativeFileFlags( void );
    std::uint16_t GetPENativeDLLOptFlags( void );

public:
    void CommitDataDirectories( void );
};

#endif //_PELOADER_CORE_

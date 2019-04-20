#ifndef _PEFRAMEWORK_STREAM_
#define _PEFRAMEWORK_STREAM_

#include <cstddef>

#include <sdk/MacroUtils.h>

typedef long long pe_file_ptr_t;

struct PEStream abstract
{
    virtual size_t Read( void *buf, size_t readCount ) = 0;
    virtual bool Write( const void *buf, size_t writeCount ) = 0;
    virtual bool Seek( pe_file_ptr_t ptr ) = 0;
    virtual pe_file_ptr_t Tell( void ) const = 0;

    // Helpers.
    template <typename structType>
    inline bool ReadStruct( structType& typeOut )
    {
        size_t readCount = this->Read( &typeOut, sizeof(structType) );

        return ( readCount == sizeof(structType) );
    }

    template <typename structType>
    inline bool WriteStruct( const structType& typeIn )
    {
        size_t writeCount = this->Write( &typeIn, sizeof(structType) );

        return ( writeCount == sizeof(structType) );
    }
};

#include <iostream>

// Helper STL wrapper.
struct PEStreamSTL : public PEStream
{
    inline PEStreamSTL( std::iostream *implStream )
    {
        this->implStream = implStream;
    }

    size_t Read( void *buf, size_t readCount ) override
    {
        std::iostream *stream = this->implStream;

        stream->read( (char*)buf, (std::streamsize)readCount );
        
        if ( stream->bad() )
            return 0;

        return (size_t)stream->gcount();
    }

    bool Write( const void *buf, size_t writeCount ) override
    {
        std::iostream *stream = this->implStream;

        stream->write( (const char*)buf, (std::streamsize)writeCount );

        return stream->good();
    }

    pe_file_ptr_t Tell( void ) const override
    {
        return this->implStream->tellg();
    }

    bool Seek( pe_file_ptr_t pos ) override
    {
        std::iostream *stream = this->implStream;

        stream->seekg( pos );

        return stream->good();
    }

private:
    std::iostream *implStream;
};

#endif //_PEFRAMEWORK_STREAM_
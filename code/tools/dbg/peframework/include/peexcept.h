#ifndef _PEFRAMEWORK_EXCEPTION_MANAGEMENT_
#define _PEFRAMEWORK_EXCEPTION_MANAGEMENT_

// Basic exception type for PEFramework.
enum class ePEExceptCode
{
    RESOURCE_ERROR,         // error based on data provided by files on disk
    RUNTIME_ERROR,          // error based on wrong assumptions of PEFramework code
    ACCESS_OUT_OF_BOUNDS,   // error based on data access out-of-bounds (any kind)
    CORRUPT_PE_STRUCTURE,   // error based on wrong parameters inside PE files
    UNSUPPORTED,            // error based on not-yet-implemented features

    GENERIC                 // generic error, use when cannot be categorized
};

struct peframework_exception
{
    // Note that descString must not be malloc'ed.
    inline peframework_exception( ePEExceptCode code, const char *descString )
    {
        this->codeval = code;
        this->what = descString;
    }

    inline ePEExceptCode code( void ) const
    {
        return this->codeval;
    }

    inline const char* desc_str( void ) const
    {
        return this->what;
    }

private:
    ePEExceptCode codeval;
    const char *what;
};

#endif //_PEFRAMEWORK_EXCEPTION_MANAGEMENT_
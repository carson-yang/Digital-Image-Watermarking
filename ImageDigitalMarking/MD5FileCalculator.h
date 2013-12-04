//*****************************************************************************
// FILE: MD5FileCalculator.h
// DESC: Header for CMD5FileCalculator class.
//
// By Giovanni Dicanio <giovanni.dicanio@gmail.com>
// 2008, September 26th
//*****************************************************************************

#pragma once


///////////////////////////////////////////////////////////////////////////////
// Class to compute MD5 digest of files.
///////////////////////////////////////////////////////////////////////////////
class CMD5FileCalculator
{
    CLASS_BAN_COPY( CMD5FileCalculator )

public:

    //
    // *** CLASS USAGE ***
    //
    // Use this class something like this:
    //
    //  CMD5FileCalculator md5Calc( filename );
    //  if ( ! md5Calc.IsInitOK() )
    //      ... initialization error
    //
    //  // Hashing loop
    //  while ( md5Calc.HashMore() )
    //          ; // or do something like sending messages to a progress bar...
    //
    //  if ( ! md5Calc.IsDigestReady() )
    //       ... error occurred during hashing
    //
    //  // All right: the MD5 digest can be get using
    //  // md5Calc.GetMD5Digest()
    //


    //
    // NOTE - use in non-MFC projects
    // ------------------------------
    // I used ATLASSERT instead of ASSERT in this class, so this class can be
    // reused also in non-MFC projects, just including proper ATL files (like
    // those that define CString class).
    //



    // After constructor, call the IsInitOK method to check 
    // if the initialization was successful
    explicit CMD5FileCalculator( 
        
        // Compute MD5 digest of this file
        LPCTSTR filename, 
        
        // Optional: size in bytes of read chunk (default is 2 MB).
        // File larger than this size are processed chunk-by-chunk.
        DWORD chunkSize  = 2 * 1024 * 1024 // 2 MB
    );
    
    // Resources cleanup
    ~CMD5FileCalculator();

    // Call this method after construction, to check if construction was OK
    BOOL IsInitOK() const;

    CString GetFileName() const;
    CString GetMD5Digest() const;
    __int64 GetFileSize() const;    // in bytes


    // This method performs partial hashing of file.
    // Call this method in a loop, until it returns FALSE.
    // When this method returns FALSE, it means that the hashing
    // is terminated. This can be caused by either normal completion, or error.
    //
    // Call IsDigestReady() to check that: if HashMore() returned FALSE and 
    // IsDigestReady() returns TRUE, it means that digest is available and 
    // everything was fine.
    // Else, if HashMore() returned FALSE and IsDigestReady() returned FALSE, 
    // it means that an error occurred (in fact, hashing was stopped, but there
    // is no digest available)
    BOOL HashMore();

    // Number of bytes currently processed
    __int64 GetProcessedBytes() const;

    // Is MD5 digest available ?
    BOOL IsDigestReady() const;



    // *** IMPLEMENTATION ***
private:

    // Was initialization successful?
    // (If this flag is FALSE, it means that constructor failed, 
    // so the object can't be used)
    BOOL m_initOK;


    // Was MD5 digest computed successfully ?
    BOOL m_digestReady;

    CString m_fileName;
    __int64 m_fileSize; // in bytes

    // Number of bytes currently processed
    __int64 m_processedBytes;

    // At the end of the computation, will store the MD5 digest
    CString m_digest;


    // Input file handle
    HANDLE m_hFile;

    // Cryptographic Service Provider (CSP) handler
    HCRYPTPROV m_hCryptProv;

    // CSP Hash object handler
    HCRYPTHASH m_hCryptHash;

    // Buffer for partial file reading for MD5 hashing
    BYTE * m_readBuffer;

    // Size of read buffer, in bytes
    DWORD m_readBufferSize;



    // Safely cleanup handles
    // (called by destructor, or by constructor if initialization fails).
    void Cleanup();

    // Given an MD5 digest (made by 16 bytes), returns a string with
    // an hex representation of that digest
    CString ConvertMD5DigestToString( const BYTE * hashValue );
};



//=============================================================================
//                     INLINE METHOD IMPLEMENTATIONS
//=============================================================================

inline BOOL CMD5FileCalculator::IsInitOK() const
{
    return m_initOK;
}

inline CString CMD5FileCalculator::GetFileName() const
{
    return m_fileName;
}

inline CString CMD5FileCalculator::GetMD5Digest() const
{
    ATLASSERT( m_initOK );
    ATLASSERT( m_digestReady );
    return m_digest;
}

inline __int64 CMD5FileCalculator::GetFileSize() const
{
    ATLASSERT( m_initOK );
    return m_fileSize;
}

inline BOOL CMD5FileCalculator::IsDigestReady() const
{
    ATLASSERT( m_initOK );
    return m_digestReady;
}

inline __int64 CMD5FileCalculator::GetProcessedBytes() const
{
    ATLASSERT( m_initOK );
    return m_processedBytes;
}


//*****************************************************************************
// FILE: MD5FileCalculator.cpp
// DESC: Header for CMD5FileCalculator class.
//
// By Giovanni Dicanio <giovanni.dicanio@gmail.com>
// 2008, September 26th
//*****************************************************************************



//=============================================================================
//                              INCLUDES
//=============================================================================

#include "stdafx.h"
#include "MD5FileCalculator.h"      // Module header



//=============================================================================
//                          METHOD IMPLEMENTATIONS
//=============================================================================


CMD5FileCalculator::CMD5FileCalculator( LPCTSTR filename, DWORD chunkSize )
    : m_initOK( FALSE ),            // initialization not done yet
    m_digestReady( FALSE ),         // no MD5 digest calculated yet
    m_fileSize( 0 ),                // no info on file size
    m_hFile( INVALID_HANDLE_VALUE ),// no file open yet
    m_hCryptProv( NULL ),           // no CSP acquired
    m_hCryptHash( NULL ),           // no hash object created
    m_readBuffer( NULL ),           // buffer for hashing
    m_readBufferSize( 0 ),          // no chunk allocated yet
    m_processedBytes( 0 )           // nothing processed yet
{
    // File name must be not null and not empty
    ATLASSERT( filename != NULL );
    ATLASSERT( *filename != TEXT('\0') );

    // Store filename
    m_fileName = filename;

    // Try opening the file
    m_hFile = ::CreateFile( 
        filename,           // file name
        GENERIC_READ,       // request reading access to file
        FILE_SHARE_READ,    // enables subsequent open operations on a file or device to request read access
        NULL,               // security attributes unused
        OPEN_EXISTING,      // opens file only if it exists
        FILE_FLAG_SEQUENTIAL_SCAN, // access is intended to be sequential from beginning to end
        NULL                // no template used
        );
    if ( m_hFile == INVALID_HANDLE_VALUE )
    {
        // Error in opening file
        return;
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if ( ! ::GetFileSizeEx( m_hFile, &fileSize) )
    {
        // Error
        Cleanup();
        return;
    }
    m_fileSize = fileSize.QuadPart;

    // Acquire a handler to a CSP (Cryptographic Service Provider)
    if ( ! ::CryptAcquireContext(
        &m_hCryptProv,  // variable to hold the returned handle
        NULL,           // use default key container
        NULL,           // use default provider 
        PROV_RSA_FULL,  // type of context to acquire
        CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET // good flags for hashing
        ) )
    {
        // Error
        Cleanup();
        return;
    }

    // Acquire handler to hash object
    if ( ! ::CryptCreateHash(
        m_hCryptProv,   // handle to CSP obtained earlier
        CALG_MD5,       // hashing algorithm
        0,              // non-keyed hash
        0,              // should be 0
        &m_hCryptHash   // handle value to be retrieved
        ) )
    {
        // Error
        Cleanup();
        return;
    }

    // Allocate read buffer
    ATLASSERT( chunkSize > 0 );
    m_readBufferSize = chunkSize;
    m_readBuffer = new BYTE[ m_readBufferSize ];

    // Object initialization successful
    m_initOK = TRUE;
}


void CMD5FileCalculator::Cleanup()
{
    // Clear digest string
    m_digestReady = FALSE;
    m_digest.Empty();

    // Clear file size info
    m_fileSize = 0;

    // Nothing processed
    m_processedBytes = 0;



    // Cleanup hash handler
    if ( m_hCryptHash != NULL )
    {
        ::CryptDestroyHash( m_hCryptHash );
        m_hCryptHash = NULL;
    }

    // Cleanup CSP handler
    if ( m_hCryptProv != NULL )
    {
        ::CryptReleaseContext( m_hCryptProv, 0 );
        m_hCryptProv = NULL;
    }

    // Close file
    if ( m_hFile != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }

    // Cleanup read buffer
    if ( m_readBuffer != NULL )
    {
        delete [] m_readBuffer;
        m_readBuffer = NULL;

        m_readBufferSize = 0;
    }


    // Object is no more initialized
    m_initOK = FALSE;
}


CMD5FileCalculator::~CMD5FileCalculator()
{
    Cleanup();
}


BOOL CMD5FileCalculator::HashMore()
{
    // Initialization must be successful
    ATLASSERT( m_initOK );

    // Hash must not be available when this method is called
    ATLASSERT( ! m_digestReady );

    // Read a new chunk from file
    DWORD readBytes = 0;
    BOOL success = ::ReadFile( m_hFile, m_readBuffer, m_readBufferSize, &readBytes, NULL );
    if ( ! success )
    {
        // Stop: Read error
        return FALSE;
    }

    // Read was successful

    // If there is no more data, it means that we can finalize the hash computation
    if ( readBytes == 0 )
    {
        BYTE hashValue[ 128 ]; // 16 bytes are fine for MD5 hash, but just make a bigger buffer...
        static const int md5HashByteCount = 16; // 16 bytes for MD5 hash
        DWORD hashValueSize = md5HashByteCount; 

        // Get hash value
        if ( ! ::CryptGetHashParam(
            m_hCryptHash,       // Handle to hash object
            HP_HASHVAL,         // Get hash value
            hashValue,          // Buffer to hold hash value
            &hashValueSize,     // Length of data
            0) )                // Must be zero
        {
            // Error
            return FALSE;
        }

        // Convert digest bytes to a string (with bytes represented in hex),
        // and save digest string in data member
        m_digest = ConvertMD5DigestToString( hashValue );

        // Digest string ready
        m_digestReady = TRUE;

        // Stop looping - job well done
        return FALSE;
    }

    // We have more bytes to process
    ATLASSERT( readBytes != 0 );

    // Update processed byte count
    m_processedBytes += readBytes;

    success = ::CryptHashData( m_hCryptHash, m_readBuffer, readBytes, 0 );
    return success; // stop looping on error, else continue looping
}


CString CMD5FileCalculator::ConvertMD5DigestToString( const BYTE * hashValue )
{
    // Check input pointer
    ATLASSERT( hashValue!= NULL );

    // Table with hex digits for integers 0,1,2,...15
    static const TCHAR * digits = TEXT("0123456789abcdef");

    // 16 bytes in MD5 digest
    static const int md5Len = 16;

    // Array of TCHAR's storing the MD5 digest string in hex
    // (each MD5 byte is represented with two hex digits, and +1 is for end-of-string)
    TCHAR hexDigest[ (md5Len * 2) + 1 ];

    TCHAR * pCurrHexDigit = hexDigest;
    const BYTE * pCurrDigestByte = hashValue;
    for ( int i = 0; i < md5Len; i++ )
    {
        *pCurrHexDigit++ = digits[ (*pCurrDigestByte) >> 4 ];      // high nibble of current byte
        *pCurrHexDigit++ = digits[ (*pCurrDigestByte) & 0x0F ];    // low nibble of current byte

        // Process next digest byte
        ++pCurrDigestByte;
    }

    // End of string
    *pCurrHexDigit = TEXT('\0');



    // The above code is equivalent to the following code
    // (but it's faster!)
    //
    //    CString strDigest;
    //    CString strHexByte;
    //    for ( int i = 0; i < cbMD5Len; i++ )
    //    {
    //        strHexByte.Format( TEXT("%02X"), pMD5Digest[i] );
    //        strDigest += strHexByte;
    //    }
    //


    // Return a CString instance
    return hexDigest;
}


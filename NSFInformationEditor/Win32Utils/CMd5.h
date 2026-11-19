#pragma once

#include <wtypesbase.h>
#include <string>
#include <fstream>
#include <vector>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

/* MD5 declaration. */
class CMd5 {
public:
    CMd5();
    CMd5(const void* input, size_t length);
    CMd5(const std::string& str);
    CMd5(const std::wstring& str);
    CMd5(std::ifstream& in);
    void Update(const void* input, size_t length);
    void Update(const std::string& str);
    void Update(const std::wstring& str);
    void Update(std::ifstream& in);
    const byte* Digest();
    std::string ToStringA(bool fUpper = true);
    std::wstring ToStringW(bool fUpper = true);
    _tstring ToString(bool fUpper = true);
    void Reset();

private:
    void Update(const byte* input, size_t length);
    void Final();
    void Transform(const byte block[64]);
    void Encode(const uint32_t* input, byte* output, size_t length);
    void Decode(const byte* input, uint32_t* output, size_t length);

    /* class uncopyable */
    CMd5(const CMd5&) = delete;
    CMd5& operator = (const CMd5&) = delete;

private:
    uint32_t _state[4]; /* state (ABCD) */
    uint32_t _count[2]; /* number of bits, modulo 2^64 (low-order word first) */
    byte _buffer[64]; /* input buffer */
    byte _digest[16]; /* message digest */
    bool _finished;   /* calculate finished ? */

    static const byte _padding[64]; /* padding for calculate */
    enum { BUFFER_SIZE = 1024 };
};

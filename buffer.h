
#ifndef _SEENET_BUFFER_H
#define _SEENET_BUFFER_H

#include "seenet.h"
#include "noncopyable.h"
#include "./util/util_endian.h"
#include<algorithm>
#include<vector>

#include<assert.h>
#include<string>
//for ssize_t
#include<unistd.h>

namespace seenet{
    namespace net{
        class Buffer: public NonCopyable
        {
         public:
             static const size_t kCheapPrepend = 8;
             static const size_t kInitialSize = 1024;
             explicit Buffer(size_t initialSize = kInitialSize)
             : m_buff(kCheapPrepend + initialSize),
               m_readerIndex(kCheapPrepend),
               m_wirterIndex(kCheapPrepend)
             {
                 assert(readableBytes() == 0);
                 assert(wirtableBytes() == 0);
                 assert(prependableBytes() == kCheapPrepend);
             }


            void swap(Buffer& rhs)
            {
                m_buff.swap(rhs.m_buff);
                std::swap(m_readerIndex, rhs.m_readerIndex);
                std::swap(m_wirterIndex, rhs.m_wirterIndex);
            }
             size_t readableBytes() const 
             { return m_wirterIndex - m_readerIndex; }

             size_t wirtableBytes() const
             { return m_buff.size() - m_wirterIndex;}

             size_t prependableBytes() const 
             { return m_readerIndex;}

            const char* peek() const
             {
                 return begin() + m_readerIndex;
             }

             const char* findCRLF() const
             { 
                const char* crlf = std::search(peek(), beginWrite(),m_kCRLF, m_kCRLF+2);

                return crlf == beginWrite() ? NULL:crlf;
             }

             const char* findCRLF(const char* start) const
             {
                 assert(peek() <= start);
                 assert(start <= (const char *)beginWrite());
                 //fixme replace with memmom
                 const char* crlf = std::search(start, beginWrite(), m_kCRLF, m_kCRLF+2);
                 return crlf == beginWrite() ? NULL:crlf;
             }

             const char* findEOL() const 
             {
                 const void* eol = memchr(peek(), '\n', readableBytes());
                 return static_cast<const char *>(eol);
             }

             const char* findEOL(const char *start) const
             {
                 assert(peek() == start);
                 assert(start <= beginWrite());
                 const void* eol = memchr(start, '\n', beginWrite() - start);
                 return static_cast<const char *>(eol);
             }

             void retrieve(size_t len)
             {
                 assert(len <= readableBytes());
                 if(len < readableBytes())
                 {
                     m_readerIndex += len;
                 }
                 else
                 {
                     retrieveAll();
                 }
             }

             void retrieveUntil(const char* end)
             {
                 assert(peek() <= end);
                 assert(end <= beginWrite());
                 retrieve(end - peek());
             }

             void retrieveInt64()
             {
                 retrieve(sizeof(int64_t));
             }

             void retrieveInt32()
             {
                 retrieve(sizeof(int32_t));
             }

             void retrieveInt16()
             {
                 retrieve(sizeof(int16_t));
             }

             void retrieveInt8()
             {
                 retrieve(sizeof(int8_t));
             }

             void retrieveAll()
             {
                 m_readerIndex = kCheapPrepend;
                 m_wirterIndex = kCheapPrepend;
             }

             std::string retrieveAllAsString(size_t len)
             {
                 assert(len <= readableBytes());
                 std::string result(peek(), len);
                 retrieve(len);
                 return result;
             }

             void append(std::string_view str)
             {
                 append(str.data(), str.size());
             }

             std::string_view toStringView() const
             { 
                 return std::string_view(peek(), static_cast<int>(readableBytes()));
             }

             void shrink(size_t reserve)
             {
                 Buffer other;
                 other.ensureWritableBytes(readableBytes()+reserve);
                 other.append(toStringView());
                 swap(other);
             }

             void append(const char * data, size_t len)
             {
                 ensureWritableBytes(len);
                 std::copy(data, data+len, beginWrite());
                 hasWritten(len);
             }

             void append(const void *data, size_t len)
             {
                 append(static_cast<const char*>(data), len);
             }

             void ensureWritableBytes(size_t len)
             {
                 if(wirtableBytes() < len)
                 {
                     makeSpace(len);
                 }

                 assert(wirtableBytes()>= len);
             }

             char *beginWrite()
             {
                 return begin() + m_wirterIndex;
             }

             const char *beginWrite() const
             {
                 return begin() + m_wirterIndex;
             }

             void hasWritten(size_t len)
             {
                 assert(len <= wirtableBytes());
                 m_wirterIndex += len;
             }

             void unwrite(size_t len)
             {
                 assert(len <= readableBytes());
                 m_wirterIndex -= len;
             }

             // append
             void appendInt64(int64_t x)
             {
                 int64_t be64 = sockets::hostToNetwork64(x);
                 append((const char*)&be64, sizeof(be64));
             }

             void appendInt32(int32_t x)
             {
                 int32_t be32 = sockets::hostToNetwork32(x);
                 append((const char*)&be32, sizeof(be32));
             }

             void appendInt16(int16_t x)
             {
                 int16_t be16 = sockets::hostToNetwork16(x);
                 append((const char*)&be16, sizeof(be16));
             }

             void appendInt8(int8_t x)
             {
                 append((const char*)&x,sizeof(x));
             }

             //read from network
             int64_t readInt64()
             {
                 int64_t result = peekInt64();
                 retrieveInt64();
                 return result;
             }

             int32_t readInt32()
             {
                 int32_t result = peekInt32();
                 retrieveInt32();
                 return result;
             }

             int16_t readInt16()
             {
                 int16_t result = peekInt16();
                 retrieveInt16();
                 return result;
             }

             int8_t readInt8()
             {
                 int8_t result = peekInt8();
                 retrieveInt8();
             }

             int64_t peekInt64() const 
             {
                 assert(readableBytes() > sizeof(int64_t));
                 int64_t be64 = 0; 
                 ::memcpy(&be64, peek(), sizeof(be64));
                 return sockets::networkToHost64(be64);
             }

             int32_t peekInt32() const 
             { 
                 assert(readableBytes() > sizeof(int32_t));
                 int32_t be32 = 0; 
                 ::memcpy(&be32, peek(), sizeof(be32));
                 return sockets::networkToHost32(be32);
             }

             int16_t peekInt16() const
             { 
                 assert(readableBytes() > sizeof(int16_t));
                 int16_t be16 = 0;
                 ::memcpy(&be16, peek(), sizeof(be16));
                 return sockets::networkToHost16(be16);
             }

             int8_t peekInt8() const
             {
                 assert(readableBytes() > sizeof(int8_t));
                 int8_t be8 = *peek();
                 return be8;
             }

             void prependInt64(int64_t x)
             {
                 int64_t be64 = sockets::hostToNetwork64(x);
                 prepend(&be64, sizeof(be64));
             }

             void prependInt32(int32_t x)
             {
                 int32_t be32 = sockets::hostToNetwork32(x);
                 prepend(&be32, sizeof(be32));
             }

             void prependInt16(int16_t x)
             {
                 int16_t be16 = sockets::hostToNetwork16(x);
                 prepend(&be16, sizeof(be16));
             }

             void prependInt8(int8_t x)
             {
                 prepend(&x,  sizeof(x));
             }

             void prepend(const void* data, size_t len)
             {
                 assert(len <= prependableBytes());
                 m_readerIndex -= len;
                 const char * d = static_cast<const char*>(data);
                 std::copy(d, d+len, begin()+m_readerIndex);
             }
             
             size_t internalCapacity() const 
             { 
                 return m_buff.capacity();
             }
             
             ssize_t readFd(int fd, int *savedErrno);
         private:
            char *begin()
            { return &*m_buff.begin();}
            const char* begin() const
            {return &*m_buff.begin();}
            
            void makeSpace(size_t len)
            { 
                if(wirtableBytes() + prependableBytes() < len + kCheapPrepend)
                { 
                    // fixme: move readable data
                    m_buff.resize(m_wirterIndex + len);
                }
                else
                { 
                    //move readable data to the front
                    assert(kCheapPrepend < m_readerIndex);
                    size_t readable = readableBytes();
                    std::copy(begin() + m_readerIndex, 
                             begin() + m_wirterIndex,
                              begin() + kCheapPrepend);
                    m_readerIndex = kCheapPrepend;
                    m_wirterIndex = m_readerIndex + readable;
                    assert(readable == readableBytes());
                }
            }

             std::vector<char> m_buff;
             size_t m_readerIndex;
             size_t m_wirterIndex;

             static const char m_kCRLF[];
        };
    }
}
#endif
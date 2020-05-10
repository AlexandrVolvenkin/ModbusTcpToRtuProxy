/*
 * $Id$
 *
 * Copyright (c) 2005-2008 Vyacheslav Frolov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * $Log$
 * Revision 1.8  2008/02/22 12:56:42  vfrolov
 * Implemented --connect-dtr option
 *
 * Revision 1.7  2008/02/08 16:40:56  vfrolov
 * Protected SendRaw() and WriteRaw() Protocol's methods
 *
 * Revision 1.6  2007/07/20 09:23:48  vfrolov
 * Added virtual ~Protocol()
 * Thanks to wurfholz
 *
 * Revision 1.5  2006/11/16 12:51:43  vfrolov
 * Added ability to set COM port parameters
 *
 * Revision 1.4  2005/10/03 13:44:17  vfrolov
 * Added Clean() method
 *
 * Revision 1.3  2005/06/10 15:55:10  vfrolov
 * Implemented --terminal option
 *
 * Revision 1.2  2005/06/08 07:40:23  vfrolov
 * Added missing DataStream::busy initialization
 *
 * Revision 1.1  2005/06/06 15:19:02  vfrolov
 * Initial revision
 *
 */

#ifndef _UTILS_H
#define _UTILS_H

///////////////////////////////////////////////////////////////
typedef vector< BYTE, allocator<BYTE> > BYTE_vector;
///////////////////////////////////////////////////////////////
class ChunkStream
{
public:
    ChunkStream() : first(0), last(0) {}

    int write(const void *pBuf, int count);
    int read(void *pBuf, int count);

private:
    char data[256];
    int first;
    int last;

    ChunkStream *pNext;

    friend class ChunkStreamQ;
};
///////////////////////////////////////////////////////////////
class ChunkStreamQ
{
public:
    ChunkStreamQ() : pFirst(NULL), pLast(NULL) {}

    void toQueue(ChunkStream *pChunk);
    ChunkStream *fromQueue();

    ChunkStream *first()
    {
        return pFirst;
    }
    ChunkStream *last()
    {
        return pLast;
    }

private:
    ChunkStream *pFirst;
    ChunkStream *pLast;
};
///////////////////////////////////////////////////////////////
class DataStream
{
public:
    DataStream(int _threshold = 0)
        : busy(0), threshold(_threshold), eof(FALSE) {}
    ~DataStream()
    {
        DataStream::Clean();
    }

    int PutData(const void *pBuf, int count);
    int GetData(void *pBuf, int count);
    void PutEof()
    {
        eof = TRUE;
    }
    BOOL isFull() const
    {
        return threshold && threshold < busy;
    }
    void Clean();

private:
    ChunkStreamQ bufQ;
    int busy;

    int threshold;
    BOOL eof;
};
///////////////////////////////////////////////////////////////
class Protocol
{
public:
    Protocol(int _thresholdSend = 0, int _thresholdWrite = 0)
        : streamCocToSock(_thresholdSend), streamSockToCoc(_thresholdWrite) {}
    virtual ~Protocol() {}

    virtual int Send(const void *pBuf, int count);
    void SendEof()
    {
        streamCocToSock.PutEof();
    }
    BOOL isSendFull() const
    {
        return streamCocToSock.isFull();
    }
    int Read(void *pBuf, int count)
    {
        return streamCocToSock.GetData(pBuf, count);
    }

    virtual int Write(const void *pBuf, int count);
    void WriteEof()
    {
        streamSockToCoc.PutEof();
    }
    BOOL isWriteFull() const
    {
        return streamSockToCoc.isFull();
    }
    int Recv(void *pBuf, int count)
    {
        return streamSockToCoc.GetData(pBuf, count);
    }

    virtual void Clean();

protected:
    int SendRaw(const void *pBuf, int count)
    {
        return streamCocToSock.PutData(pBuf, count);
    }
    int WriteRaw(const void *pBuf, int count)
    {
        return streamSockToCoc.PutData(pBuf, count);
    }

private:
    DataStream streamCocToSock;
    DataStream streamSockToCoc;
};
///////////////////////////////////////////////////////////////
class ComParams
{
public:
    ComParams();

    void SetBaudRate(const char *pBaudRate)
    {
        baudRate = atol(pBaudRate);
    }
    void SetByteSize(const char *pByteSize)
    {
        byteSize = atoi(pByteSize);
    }
    BOOL SetParity(const char *pParity);
    BOOL SetStopBits(const char *pStopBits);
    void SetIgnoreDSR(BOOL val)
    {
        ignoreDSR = val;
    }
    void SetConnectDTR(BOOL val)
    {
        connectDTR = val;
    }

    static const char *ParityStr(int parity);
    static const char *StopBitsStr(int stopBits);

    static const char *BaudRateLst();
    static const char *ByteSizeLst();
    static const char *ParityLst();
    static const char *StopBitsLst();

    long BaudRate() const
    {
        return baudRate;
    }
    int ByteSize() const
    {
        return byteSize;
    }
    int Parity() const
    {
        return parity;
    }
    int StopBits() const
    {
        return stopBits;
    }
    BOOL IgnoreDSR() const
    {
        return ignoreDSR;
    }
    BOOL ConnectDTR() const
    {
        return connectDTR;
    }

private:
    long baudRate;
    int byteSize;
    int parity;
    int stopBits;
    BOOL ignoreDSR;
    BOOL connectDTR;
};
///////////////////////////////////////////////////////////////

#endif  // _UTILS_H

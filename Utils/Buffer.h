#pragma once

#include <Windows.h>
#include <crtdbg.h>

class Lock {
public:

	Lock() : cs(NULL) {};

	Lock(PCRITICAL_SECTION sc) : cs(sc)
	{
		setLock(cs);
	}

	VOID setLock(PCRITICAL_SECTION section)
	{
		cs = section;

		_ASSERTE(NULL != cs);
		useLock();
	}

	VOID useLock() { EnterCriticalSection(cs); }

	VOID unlock()
	{
		_ASSERTE(NULL != cs);
		LeaveCriticalSection(cs);
	}

	~Lock() { (NULL != cs) ? unlock() : (VOID*)0; }

private:
	PCRITICAL_SECTION cs;
};


typedef enum {
	TypeVirtual,
	TypeHeap,
} BufferType;

class IBuffer {
public:
	IBuffer() {};
	virtual ~IBuffer() {};
	virtual const PBYTE getBuffer() = 0;
	virtual SIZE_T size() = 0;
	virtual int resize(SIZE_T) = 0;
	virtual int getError() = 0;
	virtual VOID clear() = 0;
	virtual int copyTo(PVOID src, SIZE_T len) = 0;
	virtual int copyFrom(PVOID dst, SIZE_T len) = 0;
	virtual int compare(PVOID, SIZE_T) = 0;
	virtual const BufferType getCurrentType() = 0;
	virtual int setType(BufferType) = 0;
};


class Buffer : IBuffer {
public:
	Buffer(SIZE_T size, BufferType tp = BufferType::TypeVirtual);
	Buffer(PBYTE buf, SIZE_T size, BufferType tp = BufferType::TypeVirtual);
	virtual ~Buffer();
	virtual const PBYTE getBuffer();
	virtual SIZE_T size();
	virtual int resize(SIZE_T nsize);
	virtual int getError();
	virtual VOID clear();
	virtual int copyTo(PVOID src, SIZE_T len);
	virtual int copyFrom(PVOID dst, SIZE_T len);
	virtual int compare(PVOID buffer, SIZE_T len);
	virtual const BufferType getCurrentType();
	virtual int setType(BufferType nt);
private:
	PBYTE buffer;
	SIZE_T currentSize;
	BufferType btype;
	HANDLE hHeap;
	int error;

	int internalAllocate(SIZE_T size);
	int internalFree(PVOID buffer);
};
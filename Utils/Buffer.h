#pragma once


#include <Windows.h>
#include <memory>
#include <crtdbg.h>



typedef enum {
	TypeVirtual,
	TypeHeap,
	TypeInvalid, // strictly for unit test validation
} BufferType;


/**
* Structure representing buffer attributes.
* * Some notes for use with setAttr:
*	-> setting BufferType to a value other than either 0 or
*	   the current type will result in a changeType() (which implies reallocating the buffer)
*	-> The union members are associated with different BufferTypes, e.g., you will not get a valid result in memProtect with
*	   getAttr if your current Buffer is TypeHeap.
*
*/
typedef struct {
	BufferType type;
	union {
		HANDLE	   hHeap;
		DWORD	   memProtect;
	} u;
} BUFFER_ATTRIBS, *PBUFFER_ATTRIBS;

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
	virtual int setAttribs(PBUFFER_ATTRIBS attribs) = 0;
	virtual const BUFFER_ATTRIBS& getAttribs() = 0;
	virtual int copyToOffset(PVOID src, SIZE_T offset, SIZE_T len) = 0;
	virtual int copyFromOffset(PVOID dst, SIZE_T offset, SIZE_T len) = 0;
	virtual bool isValid() = 0;
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
	virtual int setAttribs(PBUFFER_ATTRIBS attribs);
	virtual const BUFFER_ATTRIBS& getAttribs();
	virtual int copyToOffset(PVOID src, SIZE_T offset, SIZE_T len);
	virtual int copyFromOffset(PVOID dst, SIZE_T offset, SIZE_T len);
	virtual PVOID pointerFromOffset(SIZE_T offset);
	virtual bool isValid();
private:
	std::shared_ptr<BYTE> buffer;
	SIZE_T currentSize;
	BUFFER_ATTRIBS bAttrs;
	int error;

	int internalAllocate(SIZE_T size);
};
#include "Buffer.h"


Buffer::Buffer(SIZE_T size, BufferType bt) : error(ERROR_SUCCESS), currentSize(size)
{
	ZeroMemory(&bAttrs, sizeof(bAttrs));

	bAttrs.type = bt;

	if (BufferType::TypeVirtual == bAttrs.type)
		bAttrs.u.memProtect = PAGE_READWRITE;

	else if (BufferType::TypeHeap == bAttrs.type)
		bAttrs.u.hHeap = GetProcessHeap();

	internalAllocate(size);
}

/**
* Allocate and copy in the provided buffer, assuming all allocations work properly.
*/
Buffer::Buffer(PBYTE buf, SIZE_T len, BufferType bt) : error(ERROR_SUCCESS), currentSize(len), buffer(NULL)
{
	if (NULL == buf || 0 == len) {
		error = ERROR_INVALID_PARAMETER;
		return;
	}

	ZeroMemory(&bAttrs, sizeof(bAttrs));

	bAttrs.type = bt;

	if (BufferType::TypeVirtual == bAttrs.type)
		bAttrs.u.memProtect = PAGE_READWRITE;
	else if (BufferType::TypeHeap == bAttrs.type)
		bAttrs.u.hHeap = GetProcessHeap();


	int status = internalAllocate(len);
	if (ERROR_SUCCESS == status)
		status = copyTo(buf, len);

	error = status;
}

Buffer::~Buffer()
{

}

const PBYTE Buffer::getBuffer()
{
	return const_cast<const PBYTE>(buffer.get());
}

SIZE_T Buffer::size()
{
	return currentSize;
}

/**
* Reallocates the buffer to the provided size.
* This method is stupid-simple currently,
* but eventually should support (at least attempting)
* to grow and shrink the buffer in place, via virtualalloc.
*/
int Buffer::resize(SIZE_T nsize)
{
	int status = ERROR_SUCCESS;

	if (currentSize >= nsize)
		return status;

	if (buffer) {
		buffer.reset();
	}
	
	status = internalAllocate(nsize);
	clear();

	return status;
}

int Buffer::getError()
{
	return error;
}

VOID Buffer::clear()
{
	if (!buffer)
		return;

	ZeroMemory(buffer.get(), currentSize);
}


/**
* Copies the provided buffer into the currently allocated buffer,
* or returns an appropriate error if unable to do so.
*/
int Buffer::copyTo(PVOID src, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == src || NULL == buffer)
		return ERROR_INVALID_PARAMETER;

	if (len > currentSize)
		return ERROR_BUFFER_OVERFLOW;

	clear();
	RtlCopyMemory(buffer.get(), src, len);

	return status;
}

/**
* Copies the current buffer into the one provided in "dst",
* or returns an appropriate error code if unable.
*/
int Buffer::copyFrom(PVOID dst, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == dst || NULL == buffer)
		return ERROR_INVALID_PARAMETER;

	if (len < currentSize)
		return ERROR_BUFFER_OVERFLOW;

	ZeroMemory(dst, len);
	RtlCopyMemory(dst, buffer.get(), currentSize);

	return status;
}


int Buffer::compare(PVOID buf, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == buf || NULL == buffer)
		return ERROR_INVALID_PARAMETER;

	if (len > currentSize)
		return ERROR_BUFFER_OVERFLOW;

	status = memcmp(buffer.get(), buf, len);

	return status;
}

/**
* Copies information from the stored buffer into "dst" from offset "offset".
*/
int Buffer::copyFromOffset(PVOID dst, SIZE_T offset, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == dst || 0 == len)
		return ERROR_INVALID_PARAMETER;

	if ((offset + len) > currentSize)
		return ERROR_BUFFER_OVERFLOW;

	RtlCopyMemory(dst, ((PBYTE)buffer.get() + offset), len);

	return status;
}


/**
* Copies information from "src" into the stored buffer at "offset"
*/
int Buffer::copyToOffset(PVOID src, SIZE_T offset, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == src || 0 == len)
		return ERROR_INVALID_PARAMETER;

	if ((offset + len) > currentSize)
		return ERROR_BUFFER_OVERFLOW;

	RtlCopyMemory(((PBYTE)buffer.get() + offset), src, len);

	return status;
}

/**
* Sets the buffer attributes (memory protections if virtualalloc'd,
* the current HANDLE if heap allocated, )
*/
int Buffer::setAttribs(PBUFFER_ATTRIBS attribs)
{
	int status = ERROR_SUCCESS;
	SIZE_T oldSize = currentSize;

	if (NULL == attribs)
		return ERROR_INVALID_PARAMETER;

	if (BufferType::TypeHeap != attribs->type && BufferType::TypeVirtual != attribs->type)
		return ERROR_INVALID_PARAMETER;

	if (attribs->type != bAttrs.type) {
		buffer.reset();

		bAttrs.type = attribs->type;

		if (BufferType::TypeHeap == attribs->type) {
			bAttrs.u.hHeap = attribs->u.hHeap;
		}
		else if (BufferType::TypeVirtual == attribs->type) {
			bAttrs.u.memProtect = attribs->u.memProtect;
		}

		if (ERROR_SUCCESS != (status = internalAllocate(oldSize))) {
			return status;
		}
	}
	else {
		// This is an important case to handle; we need to free our
		// current buffer before switching handles
		if (BufferType::TypeHeap == bAttrs.type) {
			oldSize = currentSize;

			buffer.reset();

			bAttrs.u.hHeap = attribs->u.hHeap;
			status = internalAllocate(oldSize);
		}
		else if (BufferType::TypeVirtual == bAttrs.type) {
			if (!VirtualProtect(buffer.get(), currentSize, attribs->u.memProtect, &oldSize)) {
				status = GetLastError();
			}
		}
	}

	return status;
}



const BUFFER_ATTRIBS& Buffer::getAttribs()
{
	return bAttrs;
}

int Buffer::internalAllocate(SIZE_T size)
{
	int status = ERROR_SUCCESS;
	PBYTE tmp = nullptr;

	if (BufferType::TypeVirtual == bAttrs.type) {
		if (NULL == (tmp = (PBYTE)VirtualAlloc(NULL, size, MEM_COMMIT, bAttrs.u.memProtect))) {
			error = status = GetLastError();
		}
		buffer = std::shared_ptr<BYTE>(tmp, [&](PVOID p) { VirtualFree(p, currentSize, MEM_RELEASE); });
	}
	else if (BufferType::TypeHeap == bAttrs.type) {
		if (NULL == (tmp = (PBYTE)HeapAlloc(bAttrs.u.hHeap, HEAP_ZERO_MEMORY, size))) {
			error = status = GetLastError();
		}
		buffer = std::shared_ptr<BYTE>(tmp, [&](PVOID p) { HeapFree(bAttrs.u.hHeap, 0, p); });
	}

	currentSize = buffer ? size : 0;

	return status;
}


const BufferType Buffer::getCurrentType()
{
	return const_cast<const BufferType>(bAttrs.type);
}

int Buffer::setType(BufferType bt)
{
	int status = ERROR_SUCCESS;
	SIZE_T oldSize = currentSize;

	if (bt == bAttrs.type)
		return ERROR_SUCCESS;

	if (buffer) {
		buffer.reset();
	}

	if (bt == BufferType::TypeHeap)
		bAttrs.u.hHeap = GetProcessHeap();

	if (bt == BufferType::TypeVirtual)
		bAttrs.u.memProtect = PAGE_READWRITE;


	bAttrs.type = bt;
	status = internalAllocate(oldSize);

	return status;
}


/**
* Returns either a pointer to the requested offset in the buffer, 
* or NULL if the requested index is outside the  current buffer boundaries.
*/
PVOID Buffer::pointerFromOffset(SIZE_T offset)
{
	PVOID os = NULL;
	
	if (offset > currentSize) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return os;
	}

	os = ((PBYTE)buffer.get() + offset);

	return os;
}

/**
* Convenience function to check whether or not
* the current Buffer object is valid.
*/
bool Buffer::isValid()
{
	bool valid = false;

	if (NULL == buffer || 0 == currentSize)
		return valid;

	valid = true;
	return valid;
}

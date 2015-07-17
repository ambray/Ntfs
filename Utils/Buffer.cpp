#include "Buffer.h"


Buffer::Buffer(SIZE_T size, BufferType bt) : error(ERROR_SUCCESS), currentSize(size), buffer(NULL)
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
	if (NULL != buffer)
		internalFree(buffer);

}

const PBYTE Buffer::getBuffer()
{
	return const_cast<const PBYTE>(buffer);
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

	
	if (ERROR_SUCCESS != (status = internalFree(buffer)))
		return status;

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
	if (NULL == buffer)
		return;

	ZeroMemory(buffer, currentSize);
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
	RtlCopyMemory(buffer, src, len);

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
	RtlCopyMemory(dst, buffer, currentSize);

	return status;
}


int Buffer::compare(PVOID buf, SIZE_T len)
{
	int status = ERROR_SUCCESS;

	if (NULL == buf || NULL == buffer)
		return ERROR_INVALID_PARAMETER;

	if (len > currentSize)
		return ERROR_BUFFER_OVERFLOW;

	status = memcmp(buffer, buf, len);

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

	RtlCopyMemory(dst, ((PBYTE)buffer + offset), len);

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

	RtlCopyMemory(((PBYTE)buffer + offset), src, len);

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
		if (ERROR_SUCCESS != (status = internalFree(buffer))) {
			error = status;
			return status;
		}

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
			if (ERROR_SUCCESS != (status = internalFree(buffer)))
				return status;

			bAttrs.u.hHeap = attribs->u.hHeap;
			status = internalAllocate(oldSize);
		}
		else if (BufferType::TypeVirtual == bAttrs.type) {
			if (!VirtualProtect(buffer, currentSize, attribs->u.memProtect, &oldSize)) {
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

	if (BufferType::TypeVirtual == bAttrs.type) {
		if (NULL == (buffer = (PBYTE)VirtualAlloc(NULL, size, MEM_COMMIT, bAttrs.u.memProtect))) {
			error = status = GetLastError();
		}
	}
	else if (BufferType::TypeHeap == bAttrs.type) {
		if (NULL == (buffer = (PBYTE)HeapAlloc(bAttrs.u.hHeap, HEAP_ZERO_MEMORY, size))) {
			error = status = GetLastError();
		}
	}

	currentSize = size;
	if (NULL == buffer)
		currentSize = 0;

	return status;
}

int Buffer::internalFree(PVOID buffer)
{
	int status = ERROR_SUCCESS;

	if (NULL == buffer) {
		error = status = ERROR_INVALID_PARAMETER;
		return status;
	}

	if (BufferType::TypeVirtual == bAttrs.type) {
		if (!VirtualFree(buffer, 0, MEM_RELEASE))
			error = status = GetLastError();
	}
	else if (BufferType::TypeHeap == bAttrs.type) {
		if (!HeapFree(bAttrs.u.hHeap, 0, buffer))
			error = status = GetLastError();
	}

	currentSize = 0;
	this->buffer = NULL;

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

	if (NULL != buffer) {
		if (ERROR_SUCCESS != (status = internalFree(buffer))) {
			return status;
		}
	}

	if (bt == BufferType::TypeHeap)
		bAttrs.u.hHeap = GetProcessHeap();

	if (bt == BufferType::TypeVirtual)
		bAttrs.u.memProtect = PAGE_READWRITE;


	bAttrs.type = bt;
	status = internalAllocate(oldSize);

	return status;
}

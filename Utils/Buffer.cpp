#include "Buffer.h"


Buffer::Buffer(SIZE_T size) : error(ERROR_SUCCESS), currentSize(size), buffer(NULL) 
{
	internalAllocate(size);
}

/**
* Allocate and copy in the provided buffer, assuming all allocations work properly.
*/
Buffer::Buffer(PBYTE buf, SIZE_T len) : error(ERROR_SUCCESS), currentSize(len), buffer(NULL)
{
	if (NULL == buf || 0 == len) {
		error = ERROR_INVALID_PARAMETER;
		return;
	}

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

	status = internalAllocate(nsize);
	clear();

	if (ERROR_SUCCESS == status)
		currentSize = nsize;
	else
		error = status;

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


int Buffer::internalAllocate(SIZE_T size)
{
	int status = ERROR_SUCCESS;

	if (NULL != buffer)
		internalFree(buffer);

	if (NULL == (buffer = (PBYTE)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE))) {
		error = status = GetLastError();
	}

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

	if (!VirtualFree(buffer, 0, MEM_RELEASE))
		error = status = GetLastError();

	if (ERROR_SUCCESS == status)
		currentSize = 0;

	return status;
}



#ifndef _DBUFFER_H
#define _DBUFFER_H

#include "basics.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
	DBuffer vs. DString
	
	The difference is that with a DBuffer, if you AppendString() multiple times,
	you will get null-terminators in between each string. With a DString,
	the strings will be concatenated. You can override this behavior in a DBuffer
	by calling AppendStringNoNull instead of AppendString, but there is no function
	for inserting NULLs into a DString, as that doesn't make sense.
*/

#define DBUFFER_BUILTIN_SIZE			16

class DBuffer
{
public:
	DBuffer();
	~DBuffer();
	
	void SetTo(const uint8_t *data, int length);
	void SetTo(const char *string);
	void SetTo(DBuffer *other);
	void SetTo(DBuffer &other);
	
	void AppendData(const uchar *data, int length);
	void AppendString(const char *str);
	void AppendStringNoNull(const char *str);
	
	void AppendBool(bool value);
	void AppendChar(uchar ch);
	void Append8(uint8_t value);
	void Append16(uint16_t value);
	void Append24(uint32_t value);
	void Append32(uint32_t value);
	
	bool ReadTo(DBuffer *line, uchar ch, bool add_null=true);
	void EnsureAlloc(int min_required);
	
	void ReplaceUnprintableChars();
	
	DBuffer &operator= (const DBuffer &other);
	
	// ---------------------------------------
	
	void Clear();
	uint8_t *Data();
	uint8_t *TakeData();
	char *String();
	int Length();

private:
	uint8_t *fData;
	int fLength;
	int fAllocSize;
	bool fAllocdExternal;
	
	uint8_t fBuiltInData[DBUFFER_BUILTIN_SIZE];
};


inline void DBuffer::EnsureAlloc(int min_required)
{
	if (min_required > fAllocSize)
	{
		fAllocSize = (min_required + (min_required >> 1));
		
		if (fAllocdExternal)
		{
			fData = (uint8_t *)realloc(fData, fAllocSize);
		}
		else
		{
			fData = (uint8_t *)malloc(fAllocSize);
			fAllocdExternal = true;
			
			// compatibility with String() - copy the potential extra null-terminator
			int copysize = (fLength + 1);
			if (copysize > fAllocSize) copysize = fAllocSize;
			memcpy(fData, fBuiltInData, copysize);
		}
	}
}

inline void DBuffer::Clear()
{
	// free any external memory and switch back to builtin
	if (fAllocdExternal)
	{
		free(fData);
		fData = &fBuiltInData[0];
		fAllocSize = DBUFFER_BUILTIN_SIZE;
		fAllocdExternal = false;
	}
	
	fLength = 0;
}

inline void DBuffer::SetTo(const uint8_t *data, int length)
{
	// SetTo from a portion of ourselves
	if (data >= fData && data <= fData + (fLength - 1))
	{
		uint8_t *tempBuffer = (uint8_t *)malloc(length);
		memcpy(tempBuffer, data, length);
		SetTo(tempBuffer, length);
		free(tempBuffer);
		return;
	}
	
	if (fAllocdExternal && length < DBUFFER_BUILTIN_SIZE)
	{
		free(fData);
		fData = &fBuiltInData[0];
		fAllocSize = DBUFFER_BUILTIN_SIZE;
		fAllocdExternal = false;
	}
	else if (length > fAllocSize)
	{	// we are growing, so allocate more memory
		if (fAllocdExternal) free(fData);
		fAllocdExternal = true;
		
		fAllocSize = (length + 16);	// arbitrary, is just space for growing
		fData = (uint8_t *)malloc(fAllocSize);
	}
	
	if (length) memcpy(fData, data, length);
	fLength = length;
}


inline void DBuffer::AppendChar(uchar ch)
{
	AppendData((uchar *)&ch, 1);
}

inline void DBuffer::Append8(uint8_t value)
{
	AppendData((uchar *)&value, 1);
}

#endif

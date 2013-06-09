//------------------------------------------------------------------------------
//	Copyright (c) 2001-2008, Haiku, Inc.
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		List.cpp
//	Author(s):		The Storage kit Team
//					Isaac Yonemoto
//					Rene Gollent
//	Description:	BList class provides storage for pointers.
//					Not thread safe.
//------------------------------------------------------------------------------


// Standard Includes -----------------------------------------------------------
#include "BList.h"

// System Includes -------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// helper function
static inline
void
move_items(void** items, int32 offset, int32 count)
{
	if (count > 0 && offset != 0)
		memmove(items + offset, items, count * sizeof(void*));
}


// constructor
BList::BList(int32 count)
	  :		 fObjectList(NULL),
			 fPhysicalSize(0),
			 fItemCount(0),
			 fBlockSize(count),
			 fResizeThreshold(0)
{
	if (fBlockSize <= 0)
		fBlockSize = 1;
	_ResizeArray(fItemCount);
}


// copy constructor
BList::BList(const BList& anotherList)
	  :		 fObjectList(NULL),
			 fPhysicalSize(0),
			 fItemCount(0),
			 fBlockSize(anotherList.fBlockSize)
{
	*this = anotherList;
}


// destructor
BList::~BList()
{
	free(fObjectList);
}


// =
BList&
BList::operator =(const BList &list)
{
	fBlockSize = list.fBlockSize;
	if (_ResizeArray(list.fItemCount)) {
		fItemCount = list.fItemCount;
		memcpy(fObjectList, list.fObjectList, fItemCount * sizeof(void*));
	}
	
	return *this;
}


// AddItem
bool
BList::AddItem(const void *item, int32 index)
{
	if (index < 0 || index > fItemCount)
		return false;
	
	bool result = true;
	
	if (fItemCount + 1 > fPhysicalSize)
		result = _ResizeArray(fItemCount + 1);
	if (result) {
		++fItemCount;
		move_items(fObjectList + index, 1, fItemCount - index - 1);
		fObjectList[index] = (void *)item;
	}
	return result;
}


// AddItem
bool
BList::AddItem(const void *item)
{
	bool result = true;
	if (fPhysicalSize > fItemCount) {
		fObjectList[fItemCount] = (void *)item;
		++fItemCount;
	} else {
		if ((result = _ResizeArray(fItemCount + 1))) {
			fObjectList[fItemCount] = (void *)item;
			++fItemCount;
		}
	}
	return result;
}


// AddList
bool
BList::AddList(const BList *list, int32 index)
{
	bool result = (list && index >= 0 && index <= fItemCount);
	if (result && list->fItemCount > 0) {
		int32 count = list->fItemCount;
		if (fItemCount + count > fPhysicalSize)
			result = _ResizeArray(fItemCount + count);
		if (result) {
			fItemCount += count;
			move_items(fObjectList + index, count, fItemCount - index - count);
			memcpy(fObjectList + index, list->fObjectList,
				   list->fItemCount * sizeof(void *));
		}
	}
	return result;
}


// AddList
bool
BList::AddList(const BList *list)
{
	bool result = (list != NULL);
	if (result && list->fItemCount > 0) {
		int32 index = fItemCount;
		int32 count = list->fItemCount;
		if (fItemCount + count > fPhysicalSize)
			result = _ResizeArray(fItemCount + count);
		if (result) {
			fItemCount += count;
			memcpy(fObjectList + index, list->fObjectList,
				   list->fItemCount * sizeof(void *));
		}
	}
	return result;
}


// RemoveItem
bool
BList::RemoveItem(void *item)
{
	int32 index = IndexOf(item);
	bool result = (index >= 0);
	if (result)
		RemoveItem(index);
	return result;
}


// RemoveItem
void *
BList::RemoveItem(int32 index)
{
	void *item = NULL;
	if (index >= 0 && index < fItemCount) {
		item = fObjectList[index];
		move_items(fObjectList + index + 1, -1, fItemCount - index - 1);
		--fItemCount;
		if (fItemCount <= fResizeThreshold)
			_ResizeArray(fItemCount);
	}
	return item;
}


// RemoveItems
bool
BList::RemoveItems(int32 index, int32 count)
{
	bool result = (index >= 0 && index <= fItemCount);
	if (result) {
		if (index + count > fItemCount)
			count = fItemCount - index;
		if (count > 0) {
			move_items(fObjectList + index + count, -count,
					   fItemCount - index - count);
			fItemCount -= count;
			if (fItemCount <= fResizeThreshold)
				_ResizeArray(fItemCount);
		} else
			result = false;
	}
	return result;
}


//ReplaceItem
bool
BList::ReplaceItem(int32 index, void *newItem)
{
	bool result = false;
	
	if (index >= 0 && index < fItemCount) {
		fObjectList[index] = newItem;
		result = true;
	}
	return result;
}


// MakeEmpty
void
BList::MakeEmpty()
{
	fItemCount = 0;
	_ResizeArray(0);
}


/* Reordering items. */
// SortItems
void
BList::SortItems(int (*compareFunc)(const void *, const void *))
{
	if (compareFunc)
		qsort(fObjectList, fItemCount, sizeof(void *), compareFunc);
}


//SwapItems
bool
BList::SwapItems(int32 indexA, int32 indexB)
{
	bool result = false;
	
	if (indexA >= 0 && indexA < fItemCount
		&& indexB >= 0 && indexB < fItemCount) {
		
		void *tmpItem = fObjectList[indexA];
		fObjectList[indexA] = fObjectList[indexB];
		fObjectList[indexB] = tmpItem;
		
		result = true;
	}
	
	return result;
}


// MoveItem
// This moves a list item from posititon a to position b, moving the appropriate
// block of list elements to make up for the move. For example, in the array:
//	A B C D E F G H I J
//		Moveing 1(B)->6(G) would result in this:
// A C D E F G B H I J
bool
BList::MoveItem(int32 fromIndex, int32 toIndex)
{
	if ((fromIndex >= fItemCount) || (toIndex >= fItemCount) || (fromIndex < 0)
		|| (toIndex < 0))
		return false;
	
	void * tmpMover = fObjectList[fromIndex];
	if (fromIndex < toIndex) {
		memmove(fObjectList + fromIndex, fObjectList + fromIndex + 1,
			(toIndex - fromIndex) * sizeof(void *));
	} else if (fromIndex > toIndex) {
		memmove(fObjectList + toIndex + 1, fObjectList + toIndex,
			(fromIndex - toIndex) * sizeof(void *));
	};
	fObjectList[toIndex] = tmpMover;
	
	return true;
}


/* Retrieving items. */
// ItemAt
void *
BList::ItemAt(int32 index) const
{
	void *item = NULL;
	if (index >= 0 && index < fItemCount)
		item = fObjectList[index];
	return item;
}


// FirstItem
void *
BList::FirstItem() const
{
	void *item = NULL;
	if (fItemCount > 0)
		item = fObjectList[0];
	return item;
}


// ItemAtFast
void *
BList::ItemAtFast(int32 index) const
{
	return fObjectList[index];
}


// Items
void *
BList::Items() const
{
	return fObjectList;
}


// LastItem
void *
BList::LastItem() const
{
	void *item = NULL;
	if (fItemCount > 0)
		item = fObjectList[fItemCount - 1];
	return item;
}


/* Querying the list. */
// HasItem
bool
BList::HasItem(void *item) const
{
	return (IndexOf(item) >= 0);
}


// IndexOf
int32
BList::IndexOf(void *item) const
{
	for (int32 i = 0; i < fItemCount; i++) {
		if (fObjectList[i] == item)
			return i;
	}
	return -1;
}


// CountItems
int32
BList::CountItems() const
{
	return fItemCount;
}


// IsEmpty
bool
BList::IsEmpty() const
{
	return (fItemCount == 0);
}


/* Iterating over the list. */
//iterate a function over the whole list.  If the function outputs a true
//value, then the process is terminated.

void
BList::DoForEach(bool (*func)(void *))
{
	bool terminate = false; int32 index = 0;		//set terminate condition variables to go.
	if (func != NULL)
	{
		while ((!terminate) && (index < fItemCount))	//check terminate condition.
		{
			terminate = func(fObjectList[index]);			//reset immediate terminator
			index++;									//advance along the list.
		};
	}

}


//same as above, except this function takes an argument.
void
BList::DoForEach(bool (*func)(void *, void*), void * arg)
{
	bool terminate = false; int32 index = 0;
	if (func != NULL)
	{
		while ((!terminate) && (index < fItemCount))
		{
			terminate = func(fObjectList[index], arg);
			index++;
		};
	}
}

#if (__GNUC__ == 2)

// This is somewhat of a hack for backwards compatibility -
// the reason these functions are defined this way rather than simply
// being made private members is that if they are included, then whenever
// gcc encounters someone calling AddList() with a non-const BList pointer,
// it will try to use the private version and fail with a compiler error.

// obsolete AddList(BList* list, int32 index) and AddList(BList* list)
// AddList
extern "C" bool
AddList__5BListP5BListl(BList* self, BList* list, int32 index)
{
	return self->AddList((const BList*)list, index);
}

// AddList
extern "C" bool
AddList__5BListP5BList(BList* self, BList* list)
{
	return self->AddList((const BList *)list);
}
#endif

// Resize
//
// Resizes fObjectList to be large enough to contain count items.
bool
BList::_ResizeArray(int32 count)
{
	bool result = true;
	// calculate the new physical size
	// by doubling the existing size
	// until we can hold at least count items
	int32 newSize = fPhysicalSize > 0 ? fPhysicalSize : fBlockSize;
	int32 targetSize = count;
	if (targetSize <= 0)
		targetSize = fBlockSize;
	if (targetSize > fPhysicalSize) {
		while (newSize < targetSize)
			newSize <<= 1;
	} else if (targetSize <= fResizeThreshold) {
		newSize = fResizeThreshold;
	}
	
	// resize if necessary
	if (newSize != fPhysicalSize) {
		void** newObjectList
			= (void**)realloc(fObjectList, newSize * sizeof(void*));
		if (newObjectList) {
			fObjectList = newObjectList;
			fPhysicalSize = newSize;
			// set our lower bound to either 1/4
			//of the current physical size, or 0
			fResizeThreshold = fPhysicalSize >> 2 >= fBlockSize
				? fPhysicalSize >> 2 : 0;
		} else
			result = false;
	}
	return result;
}


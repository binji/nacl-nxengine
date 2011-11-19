/*
 * Copyright 2001-2007, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BE_LIST_H
#define _BE_LIST_H


#include "SupportDefs.h"


class BList {
	public:
		BList(int32 count = 20);
		BList(const BList& anotherList);
		virtual ~BList();
		
		virtual BList& operator =(const BList &);
		
		/* Adding and removing items. */
		bool AddItem(const void* item, int32 index);
		bool AddItem(const void* item);
		bool AddList(const BList* list, int32 index);
		bool AddList(const BList* list);
		bool RemoveItem(void* item);
		void* RemoveItem(int32 index);
		bool RemoveItems(int32 index, int32 count);
		bool ReplaceItem(int32 index, void* newItem);
		virtual void MakeEmpty();
		
		// Reorder items
		void SortItems(int (*compareFunc)(const void*, const void*));
		bool SwapItems(int32 indexA, int32 indexB);
		bool MoveItem(int32 fromIndex, int32 toIndex);
		
		// Retrieve items
		void* ItemAt(int32 index) const;
		void* FirstItem() const;
		void* ItemAtFast(int32) const;
			// does not check the array bounds!
		
		void* LastItem() const;
		void* Items() const;
		
		// Query
		bool HasItem(void* item) const;
		int32 IndexOf(void* item) const;
		int32 CountItems() const;
		bool IsEmpty() const;
		
		// Iteration
		void DoForEach(bool (*func)(void* item));
		void DoForEach(bool (*func)(void* item, void* arg2), void *arg2);
		
	private:
		bool _ResizeArray(int32 count);
		
		void**	fObjectList;
		int32	fPhysicalSize;
		int32	fItemCount;
		int32	fBlockSize;
		int32	fResizeThreshold;
		uint32	_reserved[1];
};

#endif // _BE_LIST_H

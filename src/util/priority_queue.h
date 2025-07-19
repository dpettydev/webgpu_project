#ifndef __DRG_PRIORITY_QUEUE_H__
#define __DRG_PRIORITY_QUEUE_H__

#include "core/object.h"

/// <summary>
///Priority queue which holds drgObjects.  
///Lowest integer represents highest priority.
///</summary>
class drgPriorityQueue : public drgObjectProp
{
	friend class PQTest;

public:	
	const static drgInt32 INVALID = DRG_INT_MAX; 
	
	drgPriorityQueue(drgInt32 initialPrioritiesCapacity = DEFAULT_PRIORITIES_CAPACITY);
	virtual ~drgPriorityQueue();
			
	/// <returns>The number of elements in this priority queue</returns>
	inline drgInt32 GetCount() { return m_Size; }

	/// <returns>Whether the priority queue is empty</returns>
	inline bool IsEmpty() { return 0 >= m_Size; }
		
	/// <returns>the highest priority in the queue.  returns INVALID if none</returns>
	drgInt32 GetTopPriority();

	/// <param name="priority">the priority to search for</param>
	/// <returns>Whether there are any elements with that priority</returns>
	bool IsPriorityEmpty(drgInt32 priority);

	/// <summary>removes all data from this priority queue</summary>
	bool Clear(drgInt32 initialPrioritiesCapacity = DEFAULT_PRIORITIES_CAPACITY);

	/// <returns>an element with the highest priority, or null if the priority queue is empty</returns>
	drgObjectBase* Peek();

	/// <param name="priority">the priority to search for</param>
	/// <returns>an element with that priority, or NULL if none can be found</returns>
	drgObjectBase* Peek(drgInt32 priority);

	/// <summary>removes and returns the top element of the priority queue</summary>
	/// <returns>an element with the highest priority, or null if the priority queue is empty</returns>
	drgObjectBase* Dequeue();

	/// <summary>removes and returns an element with the selected priority</summary>
	/// <param name="priority">the priority to search for</param>
	/// <returns>an element with that priority, or NULL if none can be found</returns>
	drgObjectBase* Dequeue(drgInt32 priority);

	/// <summary>Adds an element to the priority queue.  Adding the same element twice or to multiple queues or drgObjectLists causes undefined behaviour.</summary>
	/// <param name="priority">the priority of the item</param>
	/// <param name="item">the item to add</param>
	/// <returns>whether the item was successfully added</returns>
	bool Enqueue(drgInt32 priority, drgObjectBase* item);

	/// <summary>Adds all the elements in a drgObjectList to the priority queue.  Adding the same element twice or to multiple queues or drgObjectLists causes undefined behaviour.</summary>
	/// <param name="priority">the priority of the items</param>
	/// <param name="item">the list of items to add</param>
	/// <returns>whether the item was successfully added</returns>
	bool EnqueueRange(drgInt32 priority, drgObjectList* list);

	// *** PUBLIC ALIASES ***	
	
	/// <summary>alias for Peek</summary>
	inline drgObjectBase* GetFirst() { return Peek(); }
	
	/// <summary>alias for Dequeue</summary>
	inline drgObjectBase* Pop() { return Dequeue(); }
	
	/// <summary>alias for Enqueue</summary>
	inline bool Add(drgInt32 priority, drgObjectBase* item) { return Enqueue(priority, item); }
	
	/// <summary>alias for Enqueue</summary>
	inline bool AddTail(drgInt32 priority, drgObjectBase* item) { return Enqueue(priority, item); }
	
	/// <summary>alias for EnqueueRange</summary>
	inline bool AddRange(drgInt32 priority, drgObjectList* list) { return EnqueueRange(priority, list); }

	// ***  PROTECTED ***
protected:
	const static drgInt32 DEFAULT_PRIORITIES_CAPACITY = 4;

	drgInt32 m_PrioritiesCapacity;
	drgInt32 m_NumPriorities;
	drgInt32 m_Size;
	drgInt32 m_HeadIndex;
	bool m_Sorted;
	
	drgInt32* m_pPriorities;
	drgObjectList** m_ppSubqueues;

	bool Initialize(drgInt32 initialPrioritiesCapacity);

	void Uninitialize();

	void ExpandPrioritiesCapacity();

	drgObjectList* GetTopSubqueue();

	drgObjectList* GetSubqueue(drgInt32 priority, bool addSubqueueIfNoneFound);

	drgObjectList* AddSubqueue(drgInt32 priority);

	drgInt32 FindFreeIndex();

	drgInt32 GetIndexOfPriority(drgInt32 priority);

	bool RemoveTopSubqueueIfEmpty();

	bool RemoveSubqueueIfEmpty(drgInt32 priority);

	drgObjectBase* PeekIntoSubqueue(drgObjectList* subqueue);

	void Sort();

	void MergeSort(drgInt32 startPriorityIndex, drgInt32 endPriorityIndex);
	
	//for debug. mutiple PQ threads will garble this.
	void PrintPQ();
};

#endif //__DRG_PRIORITY_QUEUE_H__

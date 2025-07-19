#include "priority_queue.h"

#include "system/debug.h"


#define PRIORITY_QUEUE_NAIVE 1



drgPriorityQueue::drgPriorityQueue(drgInt32 initialPrioritiesCapacity)
{
	Initialize(initialPrioritiesCapacity);
}

drgPriorityQueue::~drgPriorityQueue()
{
	Uninitialize();
}

drgInt32 drgPriorityQueue::GetTopPriority()
{ 	
	assert(m_HeadIndex >= 0 && m_HeadIndex < m_PrioritiesCapacity);
	if(0 >= m_PrioritiesCapacity)
		return INVALID;
	return m_pPriorities[m_HeadIndex]; 
}

bool drgPriorityQueue::IsPriorityEmpty(drgInt32 priority)
{
	if (IsEmpty() == false)
	{
		drgObjectList* subqueue = GetSubqueue(priority, false);
		if(NULL != subqueue)
			return (subqueue->GetCount()) <= 0;
	}

	return true;
}

bool drgPriorityQueue::Clear(drgInt32 initialPrioritiesCapacity)
{
	Uninitialize();
	return Initialize(initialPrioritiesCapacity);
}

drgObjectBase* drgPriorityQueue::Peek()
{
	if (IsEmpty())
	{
		//drgPrintOut("PQ Peek called on empty priority queue \r\n");
		return NULL;
	}
	
	drgObjectList* subqueue = GetTopSubqueue();
	if(NULL != subqueue)
		return  PeekIntoSubqueue(subqueue);

	PrintPQ();
	drgDebug::Error("PQ Peek error \r\n");
	return NULL;
}

drgObjectBase* drgPriorityQueue::Peek(drgInt32 priority)
{
	if (IsEmpty() == false && IsPriorityEmpty(priority) == false)
	{
		drgObjectList* subqueue = GetSubqueue(priority, false);
		if(NULL != subqueue)
			return  PeekIntoSubqueue(subqueue);
	}

	//drgPrintOut("PQ Peek could not find element with that priority \r\n");
	return NULL;
}

drgObjectBase* drgPriorityQueue::Dequeue()
{
	assert(m_HeadIndex >= 0 && m_HeadIndex < m_PrioritiesCapacity);

	if (IsEmpty())
	{
		//drgPrintOut("PQ Dequeue called on empty priority queue \r\n");
		return NULL;
	}

	drgObjectList* subqueue = GetTopSubqueue();

	drgObjectBase* result = NULL;
	if(NULL != subqueue)
	{
		if(0 < subqueue->GetCount())
		{
			result = PeekIntoSubqueue(subqueue);

			if (NULL != result)
			{
				subqueue->Remove(result);
				m_Size--;
				RemoveTopSubqueueIfEmpty();
				return result;
			}
		}
	}

	PrintPQ(); 
	drgDebug::Error("PQ Dequeue error.  subqueueCount: %d, 'PeekIntoSubqueue' result is null: %d \r\n", subqueue->GetCount(), NULL == result);
	return result;
}

drgObjectBase* drgPriorityQueue::Dequeue(drgInt32 priority)
{
	if (IsEmpty() == false && IsPriorityEmpty(priority) == false)
	{
		drgObjectList* subqueue = GetSubqueue(priority, false);
		drgObjectBase* result = PeekIntoSubqueue(subqueue);

		if (NULL != result)
		{
			subqueue->Remove(result);
			m_Size--;
			bool removed = RemoveSubqueueIfEmpty(priority);
			if(removed)
			{
				m_Sorted = false;
			}
		}

		return result;
	}

	//drgPrintOut("PQ Dequeue called on empty priority queue \r\n");
	return NULL;
}

bool drgPriorityQueue::Enqueue(drgInt32 priority, drgObjectBase* item)
{	
	if(INVALID == priority)
	{
		drgPrintOut("%d is an invalid priority.  Please use a different priority. \r\n", INVALID);
		assert("Invalid PQ priority" && 0);
		return false;
	}

	drgObjectList* subqueue = GetSubqueue(priority, true);
	subqueue->AddTail(item);
	m_Size++;
	return true;
}

bool drgPriorityQueue::EnqueueRange(drgInt32 priority, drgObjectList* list)
{
	if(INVALID == priority)
	{
		drgPrintOut("Invalid priority. \r\n");
		return false;
	}

	drgObjectList* subqueue = GetSubqueue(priority, true);

	for (drgObjectBase* vObj = list->GetFirst(); NULL != vObj; vObj = list->GetNext(vObj))
	{
		subqueue->AddTail(vObj);
		m_Size++;
	}

	return true;
}

// ***  PROTECTED FUNCTIONS ***

bool drgPriorityQueue::Initialize(drgInt32 initialPrioritiesCapacity)
{
	m_PrioritiesCapacity = initialPrioritiesCapacity;

	int prioritiesBytes = sizeof(drgInt32) * m_PrioritiesCapacity;
	int subqueuesBytes = sizeof(drgObjectList*) * m_PrioritiesCapacity;
	
	MEM_INFO_SET_NEW;
	m_pPriorities = static_cast<drgInt32*>( drgMemAlloc(prioritiesBytes) );
	MEM_INFO_SET_NEW;
	m_ppSubqueues = static_cast<drgObjectList**>( drgMemAlloc(subqueuesBytes) );

	if(NULL == m_pPriorities || NULL == m_ppSubqueues)
	{
		drgPrintOut("Failed to allocate memory for priority queue");
		return false;
	}

	for (drgInt32 i = 0; i < m_PrioritiesCapacity; ++i)
	{
		m_ppSubqueues[i] = NULL;
		m_pPriorities[i] = INVALID;
	}

	m_HeadIndex = 0;
	m_Size = 0;
	m_NumPriorities = 0;
	m_Sorted = true;

	return true;
}

void drgPriorityQueue::Uninitialize()
{
	for(drgInt32 i = 0; i < m_PrioritiesCapacity; ++i)
	{
		m_pPriorities[i] = INVALID;
		if(NULL != m_ppSubqueues[i])
		{
			delete (m_ppSubqueues[i]);
			m_ppSubqueues[i] = NULL;
		}
	}
	drgMemFree(m_pPriorities);
	m_pPriorities = NULL;
	drgMemFree(m_ppSubqueues);
	m_ppSubqueues = NULL;
	
	m_HeadIndex = INVALID;
	m_Size = INVALID;
	m_PrioritiesCapacity = INVALID;
	m_NumPriorities = INVALID;
	m_Sorted = false;
}

drgObjectList* drgPriorityQueue::GetTopSubqueue()
{ 
	drgObjectList* subqueue = NULL;

#if PRIORITY_QUEUE_NAIVE
	drgInt32 min = DRG_INT_MAX;
	drgInt32 minIndex = INVALID;
	drgInt32 i = INVALID;
	for(i = m_HeadIndex; i < m_PrioritiesCapacity; ++i)
	{
		if(m_pPriorities[i] < min)
		{
			if(NULL != m_ppSubqueues[i] && 0 < m_ppSubqueues[i]->GetCount())
			{
				min = m_pPriorities[i];
				minIndex = i;
			}
		}
	}
	subqueue = m_ppSubqueues[minIndex];
#else  //PRIORITY_QUEUE_NAIVE
	Sort();
	subqueue = m_ppSubqueues[m_HeadIndex];
#endif   //PRIORITY_QUEUE_NAIVE
	return subqueue; 
}

drgObjectList* drgPriorityQueue::GetSubqueue(drgInt32 priority, bool addSubqueueIfNoneFound)
{
	assert(m_HeadIndex >= 0 && m_HeadIndex < m_PrioritiesCapacity);

	drgObjectList* subqueue = NULL;
    for(drgInt32 i = m_HeadIndex; m_PrioritiesCapacity > i; ++i)
	{
		if(m_pPriorities[i] == priority)
		{
			subqueue = m_ppSubqueues[i];
			break;
		}
	}

	if(addSubqueueIfNoneFound && NULL == subqueue)
		return AddSubqueue(priority);
	else
		return subqueue;
}

drgInt32 drgPriorityQueue::GetIndexOfPriority(drgInt32 priority)
{
    for(drgInt32 i = 0; m_PrioritiesCapacity > i; ++i)
	{
		if(m_pPriorities[i] == priority)
			return i;
	}

	return INVALID;
}

drgObjectList* drgPriorityQueue::AddSubqueue(drgInt32 priority)
{
	while ((m_HeadIndex + m_NumPriorities) >= m_PrioritiesCapacity)
		ExpandPrioritiesCapacity();

	drgInt32 indexOfAdd = FindFreeIndex();
	m_pPriorities[indexOfAdd] = priority;

	drgObjectList* subqueue = m_ppSubqueues[indexOfAdd];
	if(NULL == subqueue)
	{
		MEM_INFO_SET_NEW;
		subqueue = new drgObjectList();
		m_ppSubqueues[indexOfAdd] = subqueue;
	}

	++m_NumPriorities;

	if(m_NumPriorities > 1)
	{
		m_Sorted = false;
	}

    return subqueue;
}   

drgInt32 drgPriorityQueue::FindFreeIndex()
{
	drgInt32 freeIndex = INVALID;
	if(m_HeadIndex > 0)
	{
		m_HeadIndex--;
		freeIndex = m_HeadIndex;
	}
	else
	{
		for(int i = m_HeadIndex; i < m_PrioritiesCapacity; ++i)
		{
			if(INVALID == m_pPriorities[i])
			{
				freeIndex = i;
				break;
			}
		}
	}
	assert(INVALID != freeIndex); //could not find free index
	assert(0 <= freeIndex);
	assert(m_PrioritiesCapacity > freeIndex);

	return freeIndex;
}

drgObjectBase* drgPriorityQueue::PeekIntoSubqueue(drgObjectList* subqueue)
{
    if (NULL == subqueue)
		return NULL;
		
	return subqueue->GetFirst();
}   

void drgPriorityQueue::ExpandPrioritiesCapacity()
{
	drgInt32 newCapacity = m_PrioritiesCapacity * 2;

	int prioritiesBytes = sizeof(drgInt32) * newCapacity;
	int subqueuesBytes = sizeof(drgObjectList*) * newCapacity;

	drgInt32* pPrioritiesNew = static_cast<drgInt32*>( drgMemReAlloc(m_pPriorities, prioritiesBytes) );
	m_pPriorities = (NULL == pPrioritiesNew ? m_pPriorities : pPrioritiesNew);

	drgObjectList** ppSubqueuesNew = static_cast<drgObjectList**>( drgMemReAlloc(m_ppSubqueues, subqueuesBytes) );
	m_ppSubqueues = (NULL == ppSubqueuesNew ? m_ppSubqueues : ppSubqueuesNew);

	for(drgInt32 i = m_PrioritiesCapacity; i < newCapacity; ++i)
	{
		m_pPriorities[i] = INVALID;
		m_ppSubqueues[i] = NULL;
	}
		
	m_PrioritiesCapacity = newCapacity;
}

bool drgPriorityQueue::RemoveTopSubqueueIfEmpty()
{
#if !PRIORITY_QUEUE_NAIVE
		Sort();
#endif

	bool removed = RemoveSubqueueIfEmpty(GetTopPriority());

#if !PRIORITY_QUEUE_NAIVE
		if(removed)
			m_HeadIndex++; //ONLY IF SORTED
#endif

	return removed;
}

//returns true if the subqueue (and associated priority) was removed
bool drgPriorityQueue::RemoveSubqueueIfEmpty(drgInt32 priority)
{	
	drgInt32 index = GetIndexOfPriority(priority);
	if(INVALID == index)
		return false;
	else if(INVALID == m_pPriorities[index])
		return false;
	else 
	{
		bool invalidate = false;
		drgObjectList* subqueue = m_ppSubqueues[index];
		if(NULL != subqueue)
		{
			if((subqueue->GetCount()) <= 0)//empty
				invalidate = true;
		}
		else
		{
			invalidate = true;
		}

		if(invalidate)
		{
			m_pPriorities[index] = INVALID;
			--m_NumPriorities;
			return true;
		}
	}
	return false;
}   

void drgPriorityQueue::Sort()
{
	if(1 < m_PrioritiesCapacity)
	{
		if(false == m_Sorted)
		{
			//TestUtils::StartTimer();
			MergeSort(m_HeadIndex, m_PrioritiesCapacity);
			//TestUtils::EndTimer("Sort");
			m_Sorted = true;	
		}
	}
}

//index based merge sort. some of the sort is done in-place.  Modifies m_pPriorities and m_ppSubqueues.
//args: inclusive start, exclusive end
void drgPriorityQueue::MergeSort(drgInt32 startPriorityIndex, drgInt32 endPriorityIndex)
{
	drgInt32 size = endPriorityIndex - startPriorityIndex;

    if (1 >= size)
        return;
	
	//LEFT
	drgInt32 leftSize = size / 2;	

	if (0 != (size % 2))  //odd
		++leftSize;		  //correct for integer division
		
	drgInt32 leftStartIndex = startPriorityIndex;
	drgInt32 leftEndIndex = startPriorityIndex + leftSize;

    MergeSort(leftStartIndex, leftEndIndex);
	
	//RIGHT
	drgInt32 rightSize = size / 2;

	drgInt32 rightStartIndex = leftEndIndex;
	drgInt32 rightEndIndex = leftEndIndex + rightSize;

    MergeSort(rightStartIndex, rightEndIndex);

	//MERGE/COPY TO NEW ARRAYS
	MEM_INFO_SET_NEW;
	drgInt32* mergedPrioritiesTemp = static_cast<drgInt32*>( drgMemAlloc(sizeof(drgInt32) * size) ); 
	MEM_INFO_SET_NEW;
	drgObjectList** mergedSubqueuesTemp = static_cast<drgObjectList**>( drgMemAlloc(sizeof(drgObjectList*) * size) ); 
	drgInt32 leftSizeTemp = leftSize;
	drgInt32 leftStartIndexTemp = leftStartIndex;
	drgInt32 rightSizeTemp = rightSize;
	drgInt32 rightStartIndexTemp = rightStartIndex;
		
	bool useLeft;
	bool useRight;
	for (drgInt32 mergeIndex = 0; mergeIndex < size && (0 < leftSizeTemp || 0 < rightSizeTemp); ++mergeIndex)
	{
		useLeft = false;
		useRight = false;

		if (0 < leftSizeTemp && 0 < rightSizeTemp)
		{
			if (INVALID == m_pPriorities[rightStartIndexTemp])
				useLeft = true;
			else if(INVALID == m_pPriorities[leftStartIndexTemp])
				useRight = true;
            else if(m_pPriorities[leftStartIndexTemp] <= m_pPriorities[rightStartIndexTemp])
				useLeft = true;
			else
				useRight = true;
		}
		else if(0 < leftSizeTemp)
			useLeft = true; 
		else if(0 < rightSizeTemp)
			useRight = true;

		if(useLeft)
		{
			mergedPrioritiesTemp[mergeIndex] = m_pPriorities[leftStartIndexTemp];
			mergedSubqueuesTemp[mergeIndex] = m_ppSubqueues[leftStartIndexTemp];
			++leftStartIndexTemp;
			--leftSizeTemp;
		}
		else if(useRight)
		{
			mergedPrioritiesTemp[mergeIndex] = m_pPriorities[rightStartIndexTemp];
			mergedSubqueuesTemp[mergeIndex] = m_ppSubqueues[rightStartIndexTemp];
			++rightStartIndexTemp;
			--rightSizeTemp;
		}
		else
		{
			PrintPQ();
			drgDebug::Error("PQ Sort error \r\n");
			return;
		}
	}
		
	//COPY BACK
	drgInt32 mergeIndex = 0;
	for(drgInt32 i = startPriorityIndex; endPriorityIndex > i; ++i)
	{
		m_pPriorities[i] = mergedPrioritiesTemp[mergeIndex];
		m_ppSubqueues[i] = mergedSubqueuesTemp[mergeIndex];

		++mergeIndex;
	}

	//DEALLOC TEMPS
	for(drgInt32 i = 0; size > i; ++i)
	{
		mergedPrioritiesTemp[i] = INVALID;
		mergedSubqueuesTemp[i] = NULL;
	}
	delete[](mergedPrioritiesTemp);
	mergedPrioritiesTemp = NULL;
	delete[](mergedSubqueuesTemp);
	mergedSubqueuesTemp = NULL;
}

//mutiple PQ threads will garble this
void drgPriorityQueue::PrintPQ()
{
	drgPrintOut("\r\n PRINT PQ \r\n");

	drgPriorityQueue* q = this;
	if(NULL == q)
	{
		drgPrintOut("null PQ \r\n");
		return;
	}
	
	drgPrintOut("Priorities Capacity: %d\r\n", q->m_PrioritiesCapacity);
	drgPrintOut("Number of Priorities: %d\r\n", q->m_NumPriorities);
	drgPrintOut("GetCount(): %d\r\n", q->GetCount());
	drgPrintOut("Head Index: %d\r\n", q->m_HeadIndex);
	drgPrintOut("Sorted: %d\r\n", q->m_Sorted);

	drgPrintOut("Priorities: ");
	for (int i = 0; i < q->m_PrioritiesCapacity; ++i)
	{
		int p = q->m_pPriorities[i];

		drgPrintOut(" ");
		if(p == drgPriorityQueue::INVALID)
			drgPrintOut("(NONE)");
		else
			drgPrintOut("%d", p);
		drgPrintOut(" ");
	}
	drgPrintOut("\r\n");

	drgPrintOut("Subqueues: \r\n");
	for (int i = 0; i < q->m_PrioritiesCapacity; ++i)
	{
		drgPrintOut("I:%d ", i);

		drgPrintOut("P:%d ", q->m_pPriorities[i]);

		drgPrintOut("C:");
		drgObjectList* subqueue = q->m_ppSubqueues[i];
		if(subqueue != NULL)
		{
			drgPrintOut("%d ", q->m_ppSubqueues[i]->GetCount());
		}
		else
			drgPrintOut(" NULL ");

		drgPrintOut("\r\n");
	}
	drgPrintOut("\r\n");
}
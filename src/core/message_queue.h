#ifndef __DRG_MESSAGEQUEUE_H
#define __DRG_MESSAGEQUEUE_H

#include "core/memmgr.h"


#define MESSAGEQUEUESIZE	1024

class drgMessageRecord;

class drgMessageProcessor
{
public:
	virtual void ProcessMessage(drgMessageRecord* pMessage);
};

class drgMessageQueue
{
public:
	drgMessageQueue()
	{
		m_iReadIndex=0;
		m_iWriteIndex=0;
		m_iClearIndex=0;

		drgMemory::MemSet(m_Queue, 0, sizeof( m_Queue));
	}

	void ProcessRecords(drgMessageProcessor* pProcessor);
	void ClearProcessedRecords();

	void AddRecord(drgMessageRecord* pRec);
	bool HasRecord();

private:
	drgMessageRecord*		m_Queue[MESSAGEQUEUESIZE];
	drgMutex				m_Mutex;

	int						m_iReadIndex;
	int						m_iWriteIndex;
	int						m_iClearIndex;

	friend class MessageRecord;
};

class drgMessageRecord
{
public:
	drgMessageRecord(int dataType, void* dataPtr, bool destructData=true )
		{
			m_DataType = dataType;
			m_Data = dataPtr;
			m_Processed = false;
			m_DestructData = destructData;
		}

						~drgMessageRecord();
	void				ProcessMessage(drgMessageProcessor* pProcessor);

	int					GetDataType()		{ return m_DataType; }
	void*				GetData()			{ return m_Data; }
	bool				GetProcessedFlag()	{ return m_Processed; }
	void				SetProcessedFlag()	{ m_Processed = true; }
													

private:
	int					m_DataType;		// A unique ID to identify what class we are. Should be set by the inheriting class.
											// Used primarily for passing messages between threads since you won't know what class is incoming.
	void*				m_Data;
	bool				m_Processed;		// Marks this record as ready for deletion
	bool				m_DestructData;	// Deletes object pointed to by data when the message is deleted

};


#endif //__DRG_MESSAGEQUEUE_H

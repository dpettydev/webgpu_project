#include "message_queue.h"

void drgMessageQueue::AddRecord(drgMessageRecord* pRec)
{
	m_Mutex.Lock();

	m_Queue[m_iWriteIndex] = pRec;

	m_iWriteIndex++;							// Increment the write index for next time
	if( m_iWriteIndex >= MESSAGEQUEUESIZE)		// Wrap around
		m_iWriteIndex = 0;

	if(m_Queue[m_iWriteIndex])					// We're about to overwrite a record, we've looped the stack
	{
		if( m_Queue[m_iWriteIndex]->GetProcessedFlag())		// It's ok, this records' been processed
		{
			delete m_Queue[m_iWriteIndex];					// so delete it
			m_Queue[m_iWriteIndex] = 0;
		}
		else												// otherwise, something's very wrong
			*(int*)0 = 0;									// blow the app
	}
	m_Mutex.Unlock();
}

bool drgMessageQueue::HasRecord()
{
	m_Mutex.Lock();
	
	int index=0;
	do
	{
		if(m_Queue[index] && !m_Queue[index]->GetProcessedFlag())
		{
			m_Mutex.Unlock();
			return true;
		}

		index++;
		if( index >= MESSAGEQUEUESIZE)		// Wrap around
			index = 0;
	} while(index!=0);

	m_Mutex.Unlock();
	return false;
}

void drgMessageQueue::ProcessRecords(drgMessageProcessor* pProcessor)
{
	m_Mutex.Lock();

	while( m_Queue[m_iReadIndex] )
	{
		m_Queue[m_iReadIndex]->ProcessMessage(pProcessor);
		m_iReadIndex++;
		if( m_iReadIndex >= MESSAGEQUEUESIZE)		// Wrap around
			m_iReadIndex = 0;
	}

	m_Mutex.Unlock();
}

void drgMessageQueue::ClearProcessedRecords()
{
	m_Mutex.Lock();

	while( 	m_Queue[m_iClearIndex] &&
			m_Queue[m_iClearIndex]->GetProcessedFlag())
	{
		delete m_Queue[m_iClearIndex];
		m_Queue[m_iClearIndex] = 0;

		m_iClearIndex++;
		if( m_iClearIndex >= MESSAGEQUEUESIZE)		// Wrap around
			m_iClearIndex = 0;
	}

	m_Mutex.Unlock();
}

void drgMessageProcessor::ProcessMessage( drgMessageRecord* record )
{
	drgPrintError( "This should be derived" );
}



drgMessageRecord::~drgMessageRecord()
{
	if( m_DestructData)
	{
		//delete m_Data;  // TODO: please fix this! no deleting void*
		free( m_Data);
	}
}

void drgMessageRecord::ProcessMessage(drgMessageProcessor* pProcessor)
{
	if( m_Processed) 	// something's wrong, we shouldn't be asked to process this message twice
		*(int*)0 = 0;

	pProcessor->ProcessMessage(this);
	m_Processed = true;
}


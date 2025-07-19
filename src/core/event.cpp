
#include "core/event.h"
#include "system/global.h"

drgEventNode* drgEvent::m_Root[DRG_EVENT_MAX] = {NULL,NULL,NULL,NULL,NULL};

drgEventNode::drgEventNode(drgObjectBase* obj, drgEventNode* next)
{
	m_Object = obj;
	m_Next = next;
	m_Type[0] = '\0';
}

drgEventNode::~drgEventNode()
{
}



drgEvent::drgEvent()
{
}

drgEvent::~drgEvent()
{
}

void drgEvent::Add(DRG_EVENT_TYPE type, drgObjectBase* obj)
{
	if (!Find(type, obj))
	{
		MEM_INFO_SET_NEW;
		m_Root[type] = new drgEventNode(obj, m_Root[type]);
	}
}

void drgEvent::Remove(DRG_EVENT_TYPE type, drgObjectBase* obj)
{
	drgEventNode* itr = m_Root[type];
	drgEventNode* prev = NULL;
	while(itr!=NULL)
	{
		if(itr->m_Object==obj)
		{
			if(prev==NULL)
				m_Root[type] = itr->m_Next;
			else
				prev->m_Next = itr->m_Next;
			delete itr;
			return;
		}
		prev = itr;
		itr = itr->m_Next;
	}
}

void drgEvent::Remove(drgObjectBase* obj)
{
	for(int i=0; i<DRG_EVENT_MAX; ++i)
		Remove((DRG_EVENT_TYPE)i, obj);
}

bool drgEvent::Find(DRG_EVENT_TYPE type, drgObjectBase* obj)
{
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		if(itr->m_Object==obj)
			return true;
		itr = itr->m_Next;
	}
	return false;
}

void drgEvent::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
	DRG_EVENT_TYPE type = DRG_EVENT_KEY_DOWN;
	if(down==false)
		type = DRG_EVENT_KEY_UP;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallKeyClick(winid, key, charval, down);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseDown(unsigned int winid, int button)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_DOWN;
	drgEventNode* itr = m_Root[type];

	while(itr!=NULL)
	{
		itr->m_Object->CallMouseDown(winid, button);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseClick(unsigned int winid, int button)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_CLICK;
	drgEventNode* itr = m_Root[type];

	while(itr!=NULL)
	{
		itr->m_Object->CallMouseClick(winid, button);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseUp(unsigned int winid, int button)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_UP;
	drgEventNode* itr = m_Root[type];

	while (itr != NULL)
	{
		itr->m_Object->CallMouseUp(winid, button);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseWheel(unsigned int winid, float pos, float rel)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_MOVE;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallMouseWheel(winid, pos, rel);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_MOVE;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallMouseMove(winid, pos, rel);
		itr = itr->m_Next;
	}
}

void drgEvent::CallResize(unsigned int winid, drgVec2* size)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_MOVE;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallResize(winid, size);
		itr = itr->m_Next;
	}
}

void drgEvent::CallMouseDrag(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
	DRG_EVENT_TYPE type = DRG_EVENT_MOUSE_DRAG;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallMouseDrag(winid, pos, rel);
		itr = itr->m_Next;
	}
}

void drgEvent::CallPointerDown(unsigned int winid, int pointerIndex)
{
	DRG_EVENT_TYPE type = DRG_EVENT_POINTER_DOWN;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallPointerDown( winid, pointerIndex );
		itr = itr->m_Next;
	}
}

void drgEvent::CallPointerClick(unsigned int winid, int pointerIndex)
{
	DRG_EVENT_TYPE type = DRG_EVENT_POINTER_CLICK;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallPointerClick( winid, pointerIndex );
		itr = itr->m_Next;
	}
}

void drgEvent::CallPointerUp(unsigned int winid, int pointerIndex)
{
	DRG_EVENT_TYPE type = DRG_EVENT_POINTER_UP;
	drgEventNode* itr = m_Root[type];
	while (itr != NULL)
	{
		itr->m_Object->CallPointerUp(winid, pointerIndex);
		itr = itr->m_Next;
	}
}

void drgEvent::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2* pos, drgVec2* rel)
{
	DRG_EVENT_TYPE type = DRG_EVENT_POINTER_DOWN;
	drgEventNode* itr = m_Root[type];
	while(itr!=NULL)
	{
		itr->m_Object->CallPointerMove( winid, pointerIndex, pos, rel );
		itr = itr->m_Next;
	}
}
#ifndef __DRG_EVENT_H__
#define __DRG_EVENT_H__

#include "core/object.h"

class drgEvent;

enum DRG_EVENT_TYPE
{
	DRG_EVENT_KEY_DOWN = 0,
	DRG_EVENT_KEY_UP,
	DRG_EVENT_MOUSE_DOWN,
	DRG_EVENT_MOUSE_CLICK,
	DRG_EVENT_MOUSE_UP,
	DRG_EVENT_MOUSE_MOVE,
	DRG_EVENT_MOUSE_WHEEL,
	DRG_EVENT_MOUSE_DRAG,
	DRG_EVENT_POINTER_DOWN,
	DRG_EVENT_POINTER_CLICK,
	DRG_EVENT_POINTER_UP,
	DRG_EVENT_POINTER_MOVE,
	DRG_EVENT_RESIZE,
	DRG_EVENT_CUSTOM,
	DRG_EVENT_MAX
};

class drgEventNode
{
protected:
	drgEventNode(drgObjectBase* obj, drgEventNode*	next);
	~drgEventNode();

	drgObjectBase*	m_Object;
	char			m_Type[16];
	drgEventNode*	m_Next;

	friend class drgEvent;
};


class drgEvent
{
public:
	static void Add(DRG_EVENT_TYPE type, drgObjectBase* obj);
	static void AddCustom(char* type, drgObjectBase* obj);
	static void Remove(DRG_EVENT_TYPE type, drgObjectBase* obj);
	static void Remove(drgObjectBase* obj);
	static bool Find(DRG_EVENT_TYPE type, drgObjectBase* obj);

	static void CallKeyClick(unsigned int winid, int key, int charval, bool down);
	static void CallMouseClick(unsigned int winid, int button);
	static void CallMouseUp(unsigned int winid, int button);
	static void CallMouseDown(unsigned int winid, int button );
	static void CallMouseWheel(unsigned int winid, float pos, float rel);
	static void CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel);
	static void CallMouseDrag(unsigned int winid, drgVec2* pos, drgVec2* rel);
	static void CallPointerDown(unsigned int winid, int pointerIndex);
	static void CallPointerClick(unsigned int winid, int pointerIndex);
	static void CallPointerUp(unsigned int winid, int pointerIndex);
	static void CallPointerMove(unsigned int winid, int pointerIndex, drgVec2* pos, drgVec2* rel);
	static void CallResize(unsigned int winid, drgVec2* size);
	static void CallCustom(unsigned int winid, char* type, char* data);

protected:
	drgEvent();
	~drgEvent();

	static drgEventNode*	m_Root[DRG_EVENT_MAX];
};

#endif // __DRG_EVENT_H__


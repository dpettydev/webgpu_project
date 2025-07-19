
#include "core/input.h"

drgVec2 drgInputMouse::m_CursorPos;
drgVec2 drgInputMouse::m_CursorPosOld;
drgVec2 drgInputMouse::m_CursorPosRel;
drgVec2 drgInputMouse::m_GlobalCursorPosOld;
drgVec2 drgInputMouse::m_GlobalCursorPosRel;
drgVec2 drgInputMouse::m_GlobalCursorPos;

drgInputPointer drgInputTouch::m_Pointers[ DRG_NUM_POINTERS ];

float drgInputMouse::m_WheelPos = 0.0f;
float drgInputMouse::m_WheelPrevPos = 0.0f;
bool drgInputMouse::m_MouseButtonsDown[ DRG_MOUSE_NUM_BUTTONS ];


drgInputKeyboard::drgInputKeyboard()
{
}

drgInputKeyboard::~drgInputKeyboard()
{
}



drgInputMouse::drgInputMouse()
{
}

drgInputMouse::~drgInputMouse()
{
}

bool drgInputMouse::AddPos(drgVec2* pos)
{
	if((DRG_ABS(pos->x-m_CursorPos.x)>1.0f) || (DRG_ABS(pos->y-m_CursorPos.y)>1.0f))
	{
		m_CursorPosOld.x = m_CursorPos.x;
		m_CursorPosOld.y = m_CursorPos.y;
		m_CursorPos.x = pos->x;
		m_CursorPos.y = pos->y;
		m_CursorPosRel.x = m_CursorPos.x-m_CursorPosOld.x;
		m_CursorPosRel.y = m_CursorPos.y-m_CursorPosOld.y;
		return true;
	}
	return false;
}

bool drgInputMouse::AddGlobalPos(drgVec2* pos)
{
	if((DRG_ABS(pos->x-m_CursorPos.x)>1.0f) || (DRG_ABS(pos->y-m_CursorPos.y)>1.0f))
	{
		m_GlobalCursorPosOld.x = m_GlobalCursorPos.x;
		m_GlobalCursorPosOld.y = m_GlobalCursorPos.y;
		m_GlobalCursorPos.x = pos->x;
		m_GlobalCursorPos.y = pos->y;
		m_GlobalCursorPosRel.x = m_GlobalCursorPos.x-m_GlobalCursorPosOld.x;
		m_GlobalCursorPosRel.y = m_GlobalCursorPos.y-m_GlobalCursorPosOld.y;
		return true;
	}
	return false;
}

void drgInputMouse::AddWheel(float rel)
{
	m_WheelPrevPos = m_WheelPos;
	m_WheelPos += rel;
}

drgInputPointer::drgInputPointer()
{
	m_Position.Set( 0.0f, 0.0f );
	m_PreviousPosition.Set( 0.0f, 0.0f );
	m_GlobalPosition.Set( 0.0f, 0.0f );
	m_GlobalPreviousPosition.Set( 0.0f, 0.0f );
	m_RelativeMovement.Set( 0.0f, 0.0f );
	m_Pressure = 0.0f;
	m_Radius = DRG_POINTER_RADIUS;
}

drgInputPointer::~drgInputPointer()
{

}

bool drgInputPointer::AddPosition( drgVec2* position )
{
	if((DRG_ABS(position->x - m_Position.x) > 1.0f) || (DRG_ABS(position->y - m_Position.y) > 1.0f))
	{
		m_PreviousPosition = m_Position;
		m_Position.Set( position->x, position->y );
		m_RelativeMovement = m_Position - m_PreviousPosition;
		return true;
	}

	return false;
}

bool drgInputPointer::AddGlobalPosition( drgVec2* position )
{
	if((DRG_ABS(position->x - m_Position.x) > 1.0f) || (DRG_ABS(position->y - m_Position.y) > 1.0f))
	{
		m_GlobalPreviousPosition = m_GlobalPosition;
		m_GlobalPosition.Set( position->x, position->y );
		m_GlobalRelativeMovement = m_GlobalPosition - m_GlobalPreviousPosition;
		return true;
	}

	return false;
}

bool drgInputPointer::IsPointerInside( drgBBox* box )
{
	assert( box );

	drgVec3 pointerCenter( m_Position.x, m_Position.y, 0.0f );

	// Determine if we are contained by the box
	if( pointerCenter.x <= box->maxv.x && pointerCenter.x >= box->minv.x &&
		pointerCenter.y <= box->maxv.y && pointerCenter.y >= box->minv.y )
	{
		return true;
	}

	// Determine if we intersect the box.
	// Get the closest pointer on the AABB
	drgVec3 closestPoint;
    closestPoint.x = (pointerCenter.x < box->minv.x)? box->minv.x : (pointerCenter.x > box->maxv.x)? box->maxv.x : pointerCenter.x;
    closestPoint.y = (pointerCenter.y < box->minv.y)? box->minv.y : (pointerCenter.y > box->maxv.y)? box->maxv.y : pointerCenter.y;
    closestPoint.z = (pointerCenter.z < box->minv.z)? box->minv.z : (pointerCenter.z > box->maxv.z)? box->maxv.z : pointerCenter.z;

	// Find the seperation
	drgVec3 difference = pointerCenter - closestPoint;
	float distanceSquared = difference.LengthSqr();

	if( distanceSquared > m_Radius * m_Radius )
	{
		return false;
	}
	else
	{
		return true;
	}
}
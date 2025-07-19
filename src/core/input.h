#ifndef __DRG_INPUT_H__
#define __DRG_INPUT_H__

#include "core/event.h"

#define DRG_KEY_UNKNOWN      -1
#define DRG_KEY_SPACE        32
#define DRG_KEY_SPECIAL      256
#define DRG_KEY_ESC          (DRG_KEY_SPECIAL+1)
#define DRG_KEY_F1           (DRG_KEY_SPECIAL+2)
#define DRG_KEY_F2           (DRG_KEY_SPECIAL+3)
#define DRG_KEY_F3           (DRG_KEY_SPECIAL+4)
#define DRG_KEY_F4           (DRG_KEY_SPECIAL+5)
#define DRG_KEY_F5           (DRG_KEY_SPECIAL+6)
#define DRG_KEY_F6           (DRG_KEY_SPECIAL+7)
#define DRG_KEY_F7           (DRG_KEY_SPECIAL+8)
#define DRG_KEY_F8           (DRG_KEY_SPECIAL+9)
#define DRG_KEY_F9           (DRG_KEY_SPECIAL+10)
#define DRG_KEY_F10          (DRG_KEY_SPECIAL+11)
#define DRG_KEY_F11          (DRG_KEY_SPECIAL+12)
#define DRG_KEY_F12          (DRG_KEY_SPECIAL+13)
#define DRG_KEY_F13          (DRG_KEY_SPECIAL+14)
#define DRG_KEY_F14          (DRG_KEY_SPECIAL+15)
#define DRG_KEY_F15          (DRG_KEY_SPECIAL+16)
#define DRG_KEY_F16          (DRG_KEY_SPECIAL+17)
#define DRG_KEY_F17          (DRG_KEY_SPECIAL+18)
#define DRG_KEY_F18          (DRG_KEY_SPECIAL+19)
#define DRG_KEY_F19          (DRG_KEY_SPECIAL+20)
#define DRG_KEY_F20          (DRG_KEY_SPECIAL+21)
#define DRG_KEY_F21          (DRG_KEY_SPECIAL+22)
#define DRG_KEY_F22          (DRG_KEY_SPECIAL+23)
#define DRG_KEY_F23          (DRG_KEY_SPECIAL+24)
#define DRG_KEY_F24          (DRG_KEY_SPECIAL+25)
#define DRG_KEY_F25          (DRG_KEY_SPECIAL+26)
#define DRG_KEY_UP           (DRG_KEY_SPECIAL+27)
#define DRG_KEY_DOWN         (DRG_KEY_SPECIAL+28)
#define DRG_KEY_LEFT         (DRG_KEY_SPECIAL+29)
#define DRG_KEY_RIGHT        (DRG_KEY_SPECIAL+30)
#define DRG_KEY_LSHIFT       (DRG_KEY_SPECIAL+31)
#define DRG_KEY_RSHIFT       (DRG_KEY_SPECIAL+32)
#define DRG_KEY_LCTRL        (DRG_KEY_SPECIAL+33)
#define DRG_KEY_RCTRL        (DRG_KEY_SPECIAL+34)
#define DRG_KEY_LALT         (DRG_KEY_SPECIAL+35)
#define DRG_KEY_RALT         (DRG_KEY_SPECIAL+36)
#define DRG_KEY_TAB          (DRG_KEY_SPECIAL+37)
#define DRG_KEY_ENTER        (DRG_KEY_SPECIAL+38)
#define DRG_KEY_BACKSPACE    (DRG_KEY_SPECIAL+39)
#define DRG_KEY_INSERT       (DRG_KEY_SPECIAL+40)
#define DRG_KEY_DEL          (DRG_KEY_SPECIAL+41)
#define DRG_KEY_PAGEUP       (DRG_KEY_SPECIAL+42)
#define DRG_KEY_PAGEDOWN     (DRG_KEY_SPECIAL+43)
#define DRG_KEY_HOME         (DRG_KEY_SPECIAL+44)
#define DRG_KEY_END          (DRG_KEY_SPECIAL+45)
#define DRG_KEY_KP_0         (DRG_KEY_SPECIAL+46)
#define DRG_KEY_KP_1         (DRG_KEY_SPECIAL+47)
#define DRG_KEY_KP_2         (DRG_KEY_SPECIAL+48)
#define DRG_KEY_KP_3         (DRG_KEY_SPECIAL+49)
#define DRG_KEY_KP_4         (DRG_KEY_SPECIAL+50)
#define DRG_KEY_KP_5         (DRG_KEY_SPECIAL+51)
#define DRG_KEY_KP_6         (DRG_KEY_SPECIAL+52)
#define DRG_KEY_KP_7         (DRG_KEY_SPECIAL+53)
#define DRG_KEY_KP_8         (DRG_KEY_SPECIAL+54)
#define DRG_KEY_KP_9         (DRG_KEY_SPECIAL+55)
#define DRG_KEY_KP_DIVIDE    (DRG_KEY_SPECIAL+56)
#define DRG_KEY_KP_MULTIPLY  (DRG_KEY_SPECIAL+57)
#define DRG_KEY_KP_SUBTRACT  (DRG_KEY_SPECIAL+58)
#define DRG_KEY_KP_ADD       (DRG_KEY_SPECIAL+59)
#define DRG_KEY_KP_DECIMAL   (DRG_KEY_SPECIAL+60)
#define DRG_KEY_KP_EQUAL     (DRG_KEY_SPECIAL+61)
#define DRG_KEY_KP_ENTER     (DRG_KEY_SPECIAL+62)
#define DRG_KEY_KP_NUM_LOCK  (DRG_KEY_SPECIAL+63)
#define DRG_KEY_CAPS_LOCK    (DRG_KEY_SPECIAL+64)
#define DRG_KEY_SCROLL_LOCK  (DRG_KEY_SPECIAL+65)
#define DRG_KEY_PAUSE        (DRG_KEY_SPECIAL+66)
#define DRG_KEY_LSUPER       (DRG_KEY_SPECIAL+67)
#define DRG_KEY_RSUPER       (DRG_KEY_SPECIAL+68)
#define DRG_KEY_MENU         (DRG_KEY_SPECIAL+69)
#define DRG_KEY_LAST         DRG_KEY_MENU

/* Mouse button definitions */
#define DRG_MOUSE_BUTTON_1      0
#define DRG_MOUSE_BUTTON_2      1
#define DRG_MOUSE_BUTTON_3      2
#define DRG_MOUSE_BUTTON_4      3
#define DRG_MOUSE_BUTTON_5      4
#define DRG_MOUSE_BUTTON_6      5
#define DRG_MOUSE_BUTTON_7      6
#define DRG_MOUSE_BUTTON_8      7
#define DRG_MOUSE_BUTTON_LAST   DRG_MOUSE_BUTTON_8
#define DRG_MOUSE_NUM_BUTTONS	DRG_MOUSE_BUTTON_LAST + 1

#define DRG_MOUSE_BUTTON_LEFT   DRG_MOUSE_BUTTON_1
#define DRG_MOUSE_BUTTON_RIGHT  DRG_MOUSE_BUTTON_2
#define DRG_MOUSE_BUTTON_MIDDLE DRG_MOUSE_BUTTON_3

#define DRG_JOYSTICK_1          0
#define DRG_JOYSTICK_2          1
#define DRG_JOYSTICK_3          2
#define DRG_JOYSTICK_4          3
#define DRG_JOYSTICK_5          4
#define DRG_JOYSTICK_6          5
#define DRG_JOYSTICK_7          6
#define DRG_JOYSTICK_8          7
#define DRG_JOYSTICK_9          8
#define DRG_JOYSTICK_10         9
#define DRG_JOYSTICK_11         10
#define DRG_JOYSTICK_12         11
#define DRG_JOYSTICK_13         12
#define DRG_JOYSTICK_14         13
#define DRG_JOYSTICK_15         14
#define DRG_JOYSTICK_16         15
#define DRG_JOYSTICK_LAST       DRG_JOYSTICK_16

#define DRG_PRIMARY_POINTER		0
#define DRG_NUM_POINTERS		10
#define DRG_POINTER_RADIUS		10.0f


class drgInputKeyboard
{
public:

protected:
	drgInputKeyboard();
	~drgInputKeyboard();
};

class drgInputPointer
{
public:
	inline	drgVec2* GetPosition()					{ return &m_Position; }
	inline	drgVec2* GetPreviousPosition()			{ return &m_PreviousPosition; }
	inline	drgVec2* GetRelativeMovement()			{ return &m_RelativeMovement; }
	inline	drgVec2* GetGlobalPosition()				{ return &m_GlobalPosition; }
	inline	drgVec2* GetGlobalPreviousPosition()		{ return &m_GlobalPreviousPosition; }
	inline	drgVec2* GetGlobalRelativeMovement()		{ return &m_GlobalRelativeMovement; }
	inline	float GetPressure()							{ return m_Pressure; }
	inline	float GetRadius()							{ return m_Radius; }

	inline	bool IsDown()								{ return m_IsDown != 0; }
	inline	void SetIsDown( bool isDown )				{ m_IsDown = isDown ? 1 : 0; } 
	
			bool AddPosition( drgVec2* position );
			bool AddGlobalPosition( drgVec2* position );

	inline	void SetPosition( drgVec2* position )
	{
		m_Position.Set( position->x, position->y );
	}

	inline	void SetPreviousPosition( drgVec2* position )
	{
		m_PreviousPosition.Set( position->x, position->y );
	}

	inline	void SetGlobalPosition( drgVec2* position )
	{
		m_GlobalPosition.Set( position->x, position->y );
	}

	inline	void SetPreviousGlobalPosition( drgVec2* position )
	{
		m_GlobalPreviousPosition.Set( position->x, position->y );
	}

	inline	void SetRadius( float radius )
	{
		m_Radius = radius;
	}

	inline	void SetPressure( float pressure )
	{
		m_Pressure = pressure;
	}

	bool IsPointerInside( drgBBox* boundingBox );

protected:
	drgInputPointer();
	~drgInputPointer();

	drgVec2 m_Position;
	drgVec2 m_RelativeMovement;
	drgVec2 m_PreviousPosition;
	drgVec2 m_GlobalPosition;
	drgVec2 m_GlobalPreviousPosition;
	drgVec2 m_GlobalRelativeMovement;
	float m_Pressure;
	float m_Radius;
	int m_IsDown;

	friend class drgInputTouch;
};

class drgInputTouch
{
public:
	static drgInputPointer* GetPointer( int index = 0 )		{ assert( index >= 0 && index < DRG_NUM_POINTERS ); return &m_Pointers[ index ]; }

private:
	static drgInputPointer m_Pointers[ DRG_NUM_POINTERS ];
};


class drgInputMouse
{
public:
	static bool AddPos(drgVec2* pos);
	static bool AddGlobalPos( drgVec2* pos );
	static void AddWheel(float rel);

	static inline bool IsCursorInside(drgBBox* box)
	{
		if((m_CursorPos.x<box->minv.x)||(m_CursorPos.y<box->minv.y)||(m_CursorPos.x>box->maxv.x)||(m_CursorPos.y>box->maxv.y))
			return false;
		return true;
	}

	static inline drgVec2* GetPos()
	{
		return &m_CursorPos;
	}

	static inline drgVec2* GetGlobalPos()
	{
		return &m_GlobalCursorPos;
	}

	static inline drgVec2* GetPosMove()
	{
		return &m_CursorPosRel;
	}

	static inline drgVec2* GetGlobalPosMove()
	{
		return &m_GlobalCursorPosRel;
	}

	static inline float GetWheelPos()
	{
		return m_WheelPos;
	}

	static inline float GetWheelPrevPos()
	{
		return m_WheelPrevPos;
	}

	static inline void SetPos( int x, int y )
	{
		m_CursorPos.x = (float)x;
		m_CursorPos.y = (float)y;
	}

	static inline void SetPrevPos( int x, int y )
	{
		m_CursorPosOld.x = (float)x;
		m_CursorPosOld.y = (float)y;
	}

	static inline void SetMouseMove( int x, int y )
	{
		m_CursorPosRel.x = (float)x;
		m_CursorPosRel.y = (float)y;
	}

	static inline void SetButtonDown( int button )
	{
		assert( button >= 0 && button < DRG_MOUSE_NUM_BUTTONS );
		m_MouseButtonsDown[ button ] = true;
	}

	static inline void SetButtonUp( int button )
	{
		assert( button >= 0 && button < DRG_MOUSE_NUM_BUTTONS );
		m_MouseButtonsDown[ button ] = false;
	}

	static inline bool IsButtonDown( int button )
	{
		assert( button >= 0 && button < DRG_MOUSE_NUM_BUTTONS );
		return m_MouseButtonsDown[ button ];
	}

protected:
	static drgVec2		m_CursorPos;
	static drgVec2		m_CursorPosOld;
	static drgVec2		m_CursorPosRel;
	static drgVec2		m_GlobalCursorPos;
	static drgVec2		m_GlobalCursorPosOld;
	static drgVec2		m_GlobalCursorPosRel;
	static float			m_WheelPos;
	static float			m_WheelPrevPos;
	static bool				m_MouseButtonsDown[ DRG_MOUSE_NUM_BUTTONS ];

	drgInputMouse();
	~drgInputMouse();
};


#endif // __DRG_INPUT_H__


#ifndef __GUI_PROPERTIES_H__
#define __GUI_PROPERTIES_H__

#include "render/gui/gui.h"

class drgProperty;
class GUI_IntProperty;
class GUI_FloatProperty;
class GUI_BooleanProperty;
class GUI_Vec3Property;
class GUI_String16Property;
class GUI_Label;
class GUI_Input;
class GUI_FileDialog;

class GUI_Property : public GUI_Container
{
public:
	virtual ~GUI_Property();

	virtual void Init();
	virtual void Destroy();

	virtual void UpdateSize();

	void SetProperty(drgProperty *prop) { m_Property = prop; }
	drgProperty *GetDisplayProperty() { return m_Property; }

	virtual bool CallMouseDown(unsigned int winid, int button);
	virtual bool CallMouseClick(unsigned int winid, int button);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Property"; }

protected:
	drgProperty *m_Property;
	GUI_Label *m_Label;
	GUI_Input *m_InputBox;
	GUI_Table *m_Table;

	GUI_Property();

	friend class drgProperty;
};

class GUI_IntProperty : public GUI_Property
{
public:
	virtual ~GUI_IntProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_IntProperty"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_IntProperty();

	friend class drgIntProperty;
};

class GUI_FloatProperty : public GUI_Property
{
public:
	virtual ~GUI_FloatProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual void SetValueChangedCallback(drgCallbackObj callback, void *data)
	{
		m_ValueChangedCallback = callback;
		m_ValueChangedData = data;
	}

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_FloatProperty"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_FloatProperty();

	drgCallbackObj m_ValueChangedCallback;
	void *m_ValueChangedData;

	friend class drgFloatProperty;
};

class GUI_BooleanProperty : public GUI_Property
{
public:
	virtual ~GUI_BooleanProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_BooleanProperty"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_BooleanProperty();

	GUI_Button *m_Checkbox;

	friend class drgBooleanProperty;
};

class GUI_Vec3Property : public GUI_Property
{
public:
	virtual ~GUI_Vec3Property();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual void UpdateSize();

	void SetCallback(drgCallbackObj callback, void *data);
	static void OnInputChanged(drgObjectBase *object, void *data);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Vec3Property"; }

protected:
	GUI_Vec3Property();
	void GetText(string16 *xTxtOut, string16 *yTxtOut, string16 *zTxtOut);

	GUI_Input *m_InputBox2;
	GUI_Input *m_InputBox3;

	friend class drgVec3Property;
};

class GUI_Vec2Property : public GUI_Property
{
public:
	virtual ~GUI_Vec2Property();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_Vec2Property"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_Vec2Property();

	GUI_Input *m_InputBox2;

	friend class drgVec2Property;
};

class GUI_String16Property : public GUI_Property
{
public:
	virtual ~GUI_String16Property();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_String16Property"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_String16Property();

	friend class drgString16Property;
};

class GUI_TransformProperty : public GUI_Property
{
public:
	struct GUI_TransformPropInputChangeData
	{
		GUI_TransformProperty *guiTransformProp;
		GUI_Vec3Property *guiVec3Prop;
	};

	virtual ~GUI_TransformProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_TransformProperty"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_TransformProperty();

	GUI_Vec3Property *m_PositionGUIProp;
	GUI_Vec3Property *m_RotationGUIProp;
	GUI_Vec3Property *m_ScaleGUIProp;

	GUI_TransformPropInputChangeData m_PositionInputData;
	GUI_TransformPropInputChangeData m_RotationInputData;
	GUI_TransformPropInputChangeData m_ScaleInputData;

	friend class drgTransformProperty;
};

class GUI_ColorProperty : public GUI_Property
{
public:
	virtual ~GUI_ColorProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	void SetCallback(drgCallbackObj callback, void *data);
	static void OnInputChanged(drgObjectBase *object, void *data);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_ColorProperty"; }

protected:
	GUI_ColorProperty();

	GUI_Input *m_InputBox2;
	GUI_Input *m_InputBox3;
	GUI_Input *m_InputBox4;
	GUI_Container *m_SwathBox;

	friend class drgColorProperty;
};

class GUI_ObjectReferenceProperty : public GUI_Property
{
public:
	GUI_ObjectReferenceProperty();
	virtual ~GUI_ObjectReferenceProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);
	virtual bool CallMouseClick(unsigned int winid, int button);

	virtual void SetChangeObjectCallBack(drgCallbackObj callback, void *data);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_ObjectReferenceProperty"; }

public:
	GUI_Label *m_ObjectTypeLabel;
	GUI_Label *m_ObjectNameLabel;

protected:
	static void HandleSelectButton(drgObjectBase *obj, void *data);
	static void HandleChangeButton(drgObjectBase *obj, void *data);

	GUI_Button *m_ChangeObjectButton;
};

class GUI_EnumProperty : public GUI_Property
{
public:
	virtual ~GUI_EnumProperty();

	virtual void Init();
	virtual void Draw(GUI_Drawing *draw);

	virtual const char *GetScriptClass() { return "Vertigo.GUI.GUI_EnumProperty"; }

protected:
	static void OnInputChanged(drgObjectBase *object, void *data);
	GUI_EnumProperty();

	GUI_DropList *m_ValueNamesList;

	friend class drgEnumProperty;
};

#endif

#ifndef __DRG_PROPERTY_H__
#define __DRG_PROPERTY_H__

#include "util/stringv.h"
#include "util/math_def.h"
#include "util/color.h"
#include "core/callbacks.h"

// Defines
#define DRG_PROPERTY_NAME_MAX_SIZE 32
#define DRG_BOOL_STRING_TRUE "t"
#define DRG_BOOL_STRING_FALSE "f"
#define DRG_PROPERTIES_NODE_NAME "PROPS"
#define DRG_TRANSFORM_PROPERTY_POSITION_NAME "p"
#define DRG_TRANSFORM_PROPERTY_ROTATION_NAME "r"
#define DRG_TRANSFORM_PROPERTY_SCALE_NAME "s"

// Forward Declarations

// typedef drgObjectBase drgObjectProp;
class drgObjectProp;
class GUI_Property;
class drgObjectBase;
class drgAsset;
class drgScene;

// Class Declaration
class drgProperty
{
public: // Methods
	drgProperty();
	virtual ~drgProperty();

	virtual void Init(const char *name, drgObjectProp *object);
	virtual char *GetPropertyName();
	virtual void SetPropertyName(const char *name);
	virtual drgObjectProp *GetParentObject() { return m_ParentObject; }
	virtual const char *GetPropertyType() { return "drgProperty"; }
	virtual const char *GetClassOfReferencedAsset() { return ""; } // override for reference properties

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* name);					//save the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* name);					//load xml and replaces this object's data

	virtual void RemoveAssetReference(drgAsset *asset) {} // to be overridden by assetReferenceProperties

	virtual void SetOnChangedCallback(drgCallbackObj callback);

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetScriptClass() { return "Vertigo.drgProperty"; }

	drgProperty &operator=(const drgProperty &inRef);

public:	   // Members
protected: // Methods
	void RunOnChangedCallback(void *data);

protected: // Members
	char m_Name[DRG_PROPERTY_NAME_MAX_SIZE];
	drgObjectProp *m_ParentObject;
	drgCallbackObj m_OnChangedCallback;
	bool m_RunningCallback;
};

// PropertyList Declaration
class drgPropertyList
{
public:
	drgPropertyList();
	virtual ~drgPropertyList();

	void AddProperty(drgProperty *prop);
	void RemoveProperty(drgProperty *prop);
	void Destroy();
	int GetNumProperties() { return m_NumProperties; }

	drgProperty *GetProperty(int index);
	drgProperty *GetProperty(const char *propertyName);
	drgProperty *GetPropertyWithType(int index, const char *type);
	drgProperty *GetPropertyWithType(const char *propertyName, const char *type);
	const char *GetPropertyName(int index);
	drgPropertyList *GetPropertiesInCommon(drgPropertyList *secondList, drgObjectProp *newParent);

protected:
	int m_NumProperties;
	drgProperty **m_Properties;
};

// Child Class Declarations

// It is not recommended to use this Generic Property if it can be avoided.
// Unless it is an outlying case, it is better to create a new Property class instead.
class drgGenericProperty : public drgProperty
{
public:
	drgGenericProperty();
	virtual ~drgGenericProperty();

	virtual void SetValue(void *value);
	virtual void *GetValue() { return m_Value; }

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* name);  //saves the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* name); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgGenericProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgGenericProperty"; }

protected:
	void *m_Value;
};

class drgIntProperty : public drgProperty
{
public:
	drgIntProperty();
	virtual ~drgIntProperty();

	virtual void SetValue(int value);
	virtual int GetValue() { return m_Value; }

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual const char *GetPropertyType() { return "drgIntProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgIntProperty"; }

	virtual GUI_Property *CreateGUIWidget();

protected:
	int m_Value;
};

class drgFloatProperty : public drgProperty
{
public:
	drgFloatProperty();
	virtual ~drgFloatProperty();

	virtual void SetValue(float value);
	virtual float GetValue() { return m_Value; }

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgFloatProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgFloatProperty"; }

	drgFloatProperty &operator=(const drgFloatProperty &inRef);

protected:
	float m_Value;
};

class drgBooleanProperty : public drgProperty
{
public:
	drgBooleanProperty();
	virtual ~drgBooleanProperty();

	virtual void SetValue(bool value);
	virtual bool GetValue() { return m_Value; }

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgBooleanProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgBooleanProperty"; }

protected:
	bool m_Value;
};

class drgVec3Property : public drgProperty
{
public:
	drgVec3Property();
	virtual ~drgVec3Property();

	virtual void SetValue(drgVec3 value);
	virtual void SetValue(float x, float y, float z);
	virtual void SetValueX(float x);
	virtual void SetValueY(float y);
	virtual void SetValueZ(float z);

	virtual drgVec3 GetValue() { return m_Value; }
	virtual float GetValueX() { return m_Value.x; }
	virtual float GetValueY() { return m_Value.y; }
	virtual float GetValueZ() { return m_Value.z; }

	// virtual			void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual			void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgVec3Property"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgVec3Property"; }

protected:
	drgVec3 m_Value;
};

class drgVec2Property : public drgProperty
{
public:
	drgVec2Property();
	virtual ~drgVec2Property();

	virtual void SetValue(drgVec2 value);
	virtual void SetValue(float x, float y);
	virtual void SetValueX(float x);
	virtual void SetValueY(float y);

	virtual drgVec2 GetValue() { return m_Value; }
	virtual float GetValueX() { return m_Value.x; }
	virtual float GetValueY() { return m_Value.y; }

	// virtual			void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual			void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgVec2Property"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgVec2Property"; }

protected:
	drgVec2 m_Value;
};

class drgString16Property : public drgProperty
{
public:
	drgString16Property();
	virtual ~drgString16Property();

	virtual void SetValue(string16 *value);
	virtual void SetValue(const char *value);
	virtual const char *GetValue()
	{
		return m_Value.c_str();
	}

	// virtual void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgString16Property"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgString16Property"; }

protected:
	string8 m_Value;
};

class drgTransformProperty : public drgProperty
{
public:
	drgTransformProperty();
	virtual ~drgTransformProperty();

	virtual void SetValue(drgVec3 position, drgVec3 rotation, drgVec3 scale);
	virtual void GetValue(drgVec3 *position, drgVec3 *rotation, drgVec3 *scale);

	virtual drgVec3 *GetPosition() { return &m_ValuePosition; }
	virtual drgVec3 *GetRotation() { return &m_ValueRotation; }
	virtual drgVec3 *GetScale() { return &m_ValueScale; }

	virtual void SetPosition(drgVec3 *position);
	virtual void SetPosition(float x, float y, float z);

	virtual void SetScale(drgVec3 *scale);
	virtual void SetScale(float x, float y, float z);

	virtual void SetRotation(drgVec3 *rotation);
	virtual void SetRotation(float x, float y, float z);

	// virtual			void			Save(drgXMLNode* xmlNodeOut, const char* nodeName);  //saves the object data to xmlNodeOut
	// virtual			void			Load(drgXMLNode* xmlNodeIn, const char* nodeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgTransformProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgTransformProperty"; }

	drgTransformProperty &operator=(const drgTransformProperty &inRef);

protected:
	drgVec3 m_ValuePosition;
	drgVec3 m_ValueRotation;
	drgVec3 m_ValueScale;
};

class drgObjectReferenceProperty : public drgProperty
{
public:
	drgObjectReferenceProperty();
	virtual ~drgObjectReferenceProperty();

	virtual void SetValue(drgObjectProp *object);
	virtual drgObjectProp *GetValue();
	virtual string16 GetObjectName() { return m_ObjectName; }

	// virtual			void			Save(drgXMLNode* xmlNodeOut, const char* nodeName);  //saves the object data to xmlNodeOut
	// virtual			void			Load(drgXMLNode* xmlNodeIn, const char* nodeName); //loads xmlNodeIn and replaces this object's data

	virtual void SetScene(drgScene *scene) { m_Scene = scene; }

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgObjectReferenceProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgObjectReferenceProperty"; }

	drgObjectReferenceProperty &operator=(const drgObjectReferenceProperty &inRef);

protected:
	drgObjectProp *m_Object;
	string16 m_ObjectName;
	drgScene *m_Scene;
};

class drgColorProperty : public drgProperty
{
public:
	drgColorProperty();
	virtual ~drgColorProperty();

	virtual void SetValue(drgColor value) { m_Value = value; }
	virtual void SetValue(float r, float g, float b, float a);
	virtual void SetValueRed(float red) { m_Value.r = (unsigned char)(red * 255.0f); }
	virtual void SetValueGreen(float green) { m_Value.g = (unsigned char)(green * 255.0f); }
	virtual void SetValueBlue(float blue) { m_Value.b = (unsigned char)(blue * 255.0f); }
	virtual void SetValueAlpha(float alpha) { m_Value.a = (unsigned char)(alpha * 255.0f); }

	virtual drgColor *GetValue() { return &m_Value; }
	virtual float GetValueRed() { return (float)m_Value.r / 255.0f; }
	virtual float GetValueGreen() { return (float)m_Value.g / 255.0f; }
	virtual float GetValueBlue() { return (float)m_Value.b / 255.0f; }
	virtual float GetValueAlpha() { return (float)m_Value.a / 255.0f; }
	virtual drgVec4 GetValueVec4();

	// virtual			void			Save(drgXMLNode* xmlNodeOut, const char* attributeName);  //saves the object data to xmlNodeOut
	// virtual			void			Load(drgXMLNode* xmlNodeIn, const char* attributeName); //loads xmlNodeIn and replaces this object's data

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgColorProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgColorProperty"; }

	drgColorProperty &operator=(const drgColorProperty &inRef);

protected:
	drgColor m_Value;
};

class drgIntStringPair // used for drgEnumProperty
{
public:
	drgIntStringPair()
	{
		m_Key = 0;
		m_Value = "";
	}
	drgIntStringPair(int key, const char *value)
	{
		m_Key = key;
		m_Value = value;
	}
	virtual ~drgIntStringPair()
	{
		m_Key = 0;
		m_Value.clear();
	}

	int GetKey() { return m_Key; }
	const char *GetValue() { return m_Value.c_str(); }
	void Set(int key, const char *value)
	{
		m_Key = key;
		m_Value = value;
	}

protected:
	int m_Key;
	string8 m_Value;
};

class drgEnumProperty : public drgIntProperty
{
protected:
public:
	drgEnumProperty();
	virtual ~drgEnumProperty();

	// virtual			void			Load(drgXMLNode* nodeIn, const char* attributeName);
	// virtual			void			Save(drgXMLNode* nodeOut, const char* attributeName);

	const char *GetNameOfCurrentValue();
	const char *GetNameOfValue(int val);
	const char *GetNameAtIndex(int idx);
	void SetValueByName(const char *name);

	int GetNumberOfValues() { return m_EnumValueCount; }

	virtual void AddEnum(int value, const char *name);

	virtual GUI_Property *CreateGUIWidget();
	virtual const char *GetPropertyType() { return "drgEnumProperty"; }
	virtual const char *GetScriptClass() { return "Vertigo.drgEnumProperty"; }

protected:
	drgIntStringPair **m_EnumValues;
	int m_EnumValueCount;
};

#endif // __DRG_PROPERTY_H__

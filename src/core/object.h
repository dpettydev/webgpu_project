#ifndef __DRG_OBJECT_H__
#define __DRG_OBJECT_H__

#include "core/object_types.h"
#include "core/callbacks.h"
#include "system/global.h"
#include "util/math_def.h"
#include "util/stringv.h"
#include "core/event.h"
#include "core/property.h"

#define DRG_OBJECT_NAME_PROPERTY_NAME				"name"
#define DRG_OBJECT_OBJECT_ID_ATTRIBUTE_NAME			"guid"
#define DRG_OBJECT_FLAGS_ATTRIBUTE_NAME				"flags"
#define DRG_OBJECT_PROPERTY_TYPE_NAME				"type"
#define DRG_OBJECT_NEXT_OBJECT_ID_ATTRIBUTE_NAME	"nextGuid"
#define DRG_OBJECT_PREV_OBJECT_ID_ATTRIBUTE_NAME	"prevGuid"


#define DRG_OBJECT_FLAG_SCRIPT_CREATED				(1<<0)
#define DRG_OBJECT_FLAG_SCRIPT_USED					(1<<1)
#define DRG_OBJECT_INVALID_ID						(0xBAADF00D)

//typedef drgObjectBase drgObjectProp;
class drgObjectProp;
class drgDrawList;
class drgProperty;
class drgPropertyList;
class drgScene;
class drgAsset;

class drgObjectBase
{
public:
	drgObjectBase();
	virtual ~drgObjectBase();
	static drgObjectProp* CreateObjectOfType(const char* type);

	virtual void Init();
	virtual void Reinit();
	virtual void Destroy();
	virtual void Update( float deltaTime );

	virtual void CallKeyClick(unsigned int winid, int key, int charval, bool down);
	virtual bool CallMouseDown(unsigned int winid, int button);
	virtual bool CallMouseClick(unsigned int winid, int button);
	virtual void CallMouseUp(unsigned int winid, int button);
	virtual bool CallMouseDoubleClick(unsigned int winid, int button);
	virtual void CallMouseWheel(unsigned int winid, float pos, float rel);
	virtual void CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel);
	virtual void CallMouseDrag(unsigned int winid, drgVec2* pos, drgVec2* rel);
	virtual bool CallPointerDown(unsigned int winid, int pointerIndex);
	virtual bool CallPointerClick(unsigned int winid, int pointerIndex);
	virtual void CallPointerUp(unsigned int winid, int pointerIndex);
	virtual void CallPointerMove(unsigned int windid, int pointerIndex, drgVec2* pos, drgVec2* rel);
	virtual void CallResize(unsigned int winid, drgVec2* size);
	virtual void CallCustom(unsigned int winid, char* data);

	inline void SetObjectFlag(unsigned int flag)			{ m_Flags|=flag; }
	inline void RemoveObjectFlag(unsigned int flag)			{ m_Flags&=(~flag); }
	inline bool GetObjectFlag(unsigned int flag)			{ return ((m_Flags&flag)!=0); }

	virtual bool IsDrawObject()						{ return false; }
	virtual bool DestroyInDescendants(drgObjectBase* obj) { return false; } //to be overridden

	virtual string16 GetObjName();
	virtual string8 GetObjName8();

	virtual const char* GetScriptClass() {return "Vertigo.drgObjectBase";}
	void UseAsScriptObject();
	void ReleaseAsScriptObject();

	void DelayedDelete();

	inline void* GetScriptObject() {return m_ScriptObject;}
	inline void SetScriptObject(void* obj) {m_ScriptObject = obj;}

protected:
	void*				m_ScriptObject;
	unsigned int		m_Flags;
	drgObjectBase*		m_Next;
	drgObjectBase*		m_Prev;

	friend class drgObjectList;
};


class drgObjectProp : public drgObjectBase
{
public:
	drgObjectProp();
	virtual ~drgObjectProp();

	virtual void Init();
	virtual void Destroy();
	virtual void Update(float deltaTime);

	// virtual void Save(drgXMLNode* xmlNodeOut);  //saves the object data to xmlNodeOut
	// virtual void Load(drgXMLNode* xmlNodeIn); //loads xmlNodeIn and replaces this object's data

	virtual void RemoveAssetReference(drgAsset* asset, bool removeInDescendants = false);
	virtual drgObjectProp* GetObjectInDescendantsByObjectID(int objectID) { return NULL; } //plz override
	virtual drgObjectProp* GetObjectInDescendantsByName(string16* objectName) { return NULL; } //plz override

	virtual string16	GetTreeName();

	inline drgScene* GetParentScene()
	{
		return m_ParentScene;
	}

	inline void SetParentScene(drgScene* scene)
	{
		m_ParentScene = scene;
	}

	string8 GetSceneFolder();

	virtual string16 GetObjName();
	virtual string8 GetObjName8();

	void SetObjName(const char* name)
	{
		m_Name.SetValue(name);
	}

	void SetObjName(string16* name)
	{
		m_Name.SetValue(name);
	}

	void SetObjectIDToNewUniqueValue();
	inline void	SetObjectID(unsigned int id)
	{
		m_ObjectID = id;
	}

	inline int	GetObjectID()
	{
		return m_ObjectID;
	}

	void					 AddProperty(drgProperty* prop);
	void					 RemoveProperty(drgProperty* prop);

	virtual drgProperty*	 GetProperty(int index);
	virtual drgProperty*	 GetProperty(const char* propertyName);
	virtual const char*		 GetPropertyName(int index);
	virtual int				 GetNumProperties();
	virtual drgPropertyList* GetPropertiesInCommon(drgPropertyList* secondList, drgObjectProp* newParent);
	virtual const char*		 GetObjectType()	{ return m_TypeString.c_str(); }

	inline drgPropertyList*	 GetPropertyList()
	{
		return m_PropertyList;
	}

	virtual const char* GetScriptClass() { return "Vertigo.drgObject"; }

protected:
	int					m_ObjectID; //known as "guid" in xml attributes, or the c# function drgObject.GetGUID
	drgScene*			m_ParentScene;
	drgPropertyList*	m_PropertyList;
	drgString16Property m_Name;
	string8 m_TypeString;

	static int			m_ObjectIDCounter;
};



class drgObjectList : public drgObjectProp
{
public:
	drgObjectList();
	~drgObjectList();

	void AddHead(drgObjectBase* obj);
	void AddTail(drgObjectBase* obj);
	void AddIndex(drgObjectBase* obj, int index);
	void AddAfter(drgObjectBase* obj, drgObjectBase* after_obj);
	void Remove(drgObjectBase* obj);
	bool InList(drgObjectBase* obj);

	inline unsigned int GetCount()
	{
		return m_Count;
	}

	inline drgObjectBase* GetFirst()
	{
		return m_Root;
	}
	
	inline drgObjectBase* GetLast()
	{
		if(m_Root==NULL)
			return NULL;
		drgObjectBase* itr = m_Root;
		while(itr->m_Next != NULL)
			itr = itr->m_Next;
		return itr;
	}
	
	inline drgObjectBase* GetNext(drgObjectBase* obj)
	{
		if(obj==NULL)
			return NULL;
		return obj->m_Next;
	}

	inline drgObjectBase* GetPrev(drgObjectBase* obj)
	{
		if(obj==NULL)
			return NULL;
		return obj->m_Prev;
	}

	virtual		const char*		GetScriptClass() { return "Vertigo.drgObjectList"; }

protected:
	void CleanUp();

	unsigned int	m_Count;
	drgObjectBase*		m_Root;
};

class drgObjectItr
{
public:
	drgObjectItr(drgObjectBase* obj);
	~drgObjectItr();

private:
	drgObjectItr();

	drgObjectBase*			m_Obj;
	drgObjectItr*		m_Next;
	drgObjectItr*		m_Prev;
};

#endif // __DRG_OBJECT_H__


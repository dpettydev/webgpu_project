#include "system/engine.h"
#include "core/object.h"
#include "util/stringn.h"
#include "core/property.h"
// #include "core/objects/model_object.h"
// #include "core/scene.h"
// #include "util/xml/xml.h"
// #include "util/xml/xmlText.h"
// #include "animation/anim.h"
// #include "screen/sprite.h"
// #include "screen/screen_particle.h"
// #include "screen/text.h"
// #include "screen/screen.h"
// #include "screen/canvas.h"
// #include "screen/screen_anim.h"
// #include "screen/scene_proxy.h"
// #include "core/assets/font_asset.h"
// #include "core/assets/model_asset.h"
// #include "core/assets/particle_asset.h"
// #include "core/assets/texture_asset.h"
// #include "core/assets/file_asset.h"
// #include "core/assets/sound_asset.h"

#define DRG_OBJECT_NAME_ATTRIBUTE	"n"
#define DRG_OBJECT_TYPE_ATTRIBUTE	"type"
#define DRG_OBJECT_ID_ATTRIBUTE		"id"

int drgObjectProp::m_ObjectIDCounter = 0;

drgObjectBase::drgObjectBase()
{
	m_Next = NULL;
	m_Prev = NULL;
	m_Flags = 0;
	m_ScriptObject = NULL;
}

drgObjectBase::~drgObjectBase()
{
	Destroy();
}

void drgObjectBase::Destroy()
{
	if (GetObjectFlag(DRG_OBJECT_FLAG_SCRIPT_USED))
		ReleaseAsScriptObject();
}

drgObjectProp* drgObjectBase::CreateObjectOfType(const char* type)
{
	drgObjectProp* result = NULL;

	//drgScreenObjects
	// if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_CANVAS) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgCanvas();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SPRITE) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgSprite();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_TEXT) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgText();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SCREEN_PARTICLE) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgScreenParticle();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SCREEN) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgScreen();
	// }
	// //else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SCREEN_ANIM) == 0)
	// //	result = new drgScreenAnim();
	// //else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SCREEN_OBJECT) == 0)
	// //	result = new drgScreenObject();

	// //drgDrawObjects
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_MODEL_OBJECT) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgModelObject();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_DRAW_OBJECT) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgDrawObject();
	// }
	// else if(drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_SCENE_PROXY) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgSceneProxy();
	// }

	// //drgAssets
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_FONT) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgFontAsset();
	// }
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_MODEL) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgModelAsset();
	// }
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_PARTICLE) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgParticleAsset();
	// }
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_TEXTURE) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgTextureAsset();
	// }
	// else if( drgString::CompareNoCase( type, DRG_SCENE_ASSET_TYPE_SCREEN_ANIM) == 0 )
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgScreenAnim();
	// }
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_SOUND) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgSoundAsset();
	// }
	// else if (drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_FILE) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgFileAsset();
	// }
	// //else if(drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_FOLDER) == 0)
	// //	result = new drgFolder();
	// //else if(drgString::CompareNoCase(type, DRG_SCENE_ASSET_TYPE_VIDEO) == 0)
	// //	result = new drgVideo(); 
	
	// //Other
	// else if (drgString::CompareNoCase(type, DRG_SCENE_OBJECT_TYPE_TRANSFORMABLE_OBJECT) == 0)
	// {
	// 	MEM_INFO_SET_NEW;
	// 	result = new drgTransformableObject();
	// }
	// else
	{
		drgPrintWarning("Tried to create an object of unknown type, is this intended?");
		MEM_INFO_SET_NEW;
		result = new drgObjectProp();
	}

	result->UseAsScriptObject();
	result->Init();

	return result;
}

void drgObjectBase::Init()
{
	Reinit();
}

void drgObjectBase::Reinit()
{
}

void drgObjectBase::Update(float deltaTime)
{
}

string16 drgObjectBase::GetObjName()
{
	return string16("base");
}

string8 drgObjectBase::GetObjName8()
{
	return string8("base");
}


void drgObjectBase::CallKeyClick(unsigned int winid, int key, int charval, bool down)
{
}

bool drgObjectBase::CallMouseDown(unsigned int winid, int button)
{
	return false;
}

void drgObjectBase::CallMouseUp(unsigned int winid, int button)
{
}

bool drgObjectBase::CallMouseClick(unsigned int winid, int button)
{
	return false;
}

bool drgObjectBase::CallMouseDoubleClick(unsigned int winid, int button)
{
	return false;
}

void drgObjectBase::CallMouseWheel(unsigned int winid, float pos, float rel)
{
}

void drgObjectBase::CallMouseMove(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
}

void drgObjectBase::CallMouseDrag(unsigned int winid, drgVec2* pos, drgVec2* rel)
{
}

bool drgObjectBase::CallPointerDown(unsigned int winid, int pointerIndex)
{
	return false;
}

bool drgObjectBase::CallPointerClick(unsigned int winid, int pointerIndex)
{
	return false;
}

void drgObjectBase::CallPointerUp(unsigned int winid, int pointerIndex)
{
}

void drgObjectBase::CallPointerMove(unsigned int winid, int pointerIndex, drgVec2* pos, drgVec2* rel)
{

}

void drgObjectBase::CallResize(unsigned int winid, drgVec2* size)
{
}

void drgObjectBase::CallCustom(unsigned int winid, char* data)
{
}

#if !defined(PNET_SCRIPT) || defined(EMSCRIPTEN)
void drgObjectBase::UseAsScriptObject()
{
}

void drgObjectBase::ReleaseAsScriptObject()
{
}
#endif



void drgObjectBase::DelayedDelete()
{
	drgEngine::DelayDeleteObject( this );
}






drgObjectProp::drgObjectProp()
{
	m_Next = NULL;
	m_Prev = NULL;
	m_Flags = 0;
	m_ScriptObject = NULL;
	m_ParentScene = NULL;

	m_PropertyList = NULL;

	m_ObjectID = DRG_OBJECT_INVALID_ID;

	// Setup our properties here.
	MEM_INFO_SET_NEW;
	m_PropertyList = new drgPropertyList;

	m_Name.Init(DRG_OBJECT_NAME_PROPERTY_NAME, this);
	m_Name.SetValue("UNNAMED_OBJECT");

	m_TypeString = DRG_SCENE_OBJECT_TYPE_OBJECT;
}

drgObjectProp::~drgObjectProp()
{
	Destroy();
}

void drgObjectProp::Init()
{
}

void drgObjectProp::Destroy()
{
	SAFE_DELETE(m_PropertyList);
}

void drgObjectProp::Update(float deltaTime)
{
}

string16 drgObjectProp::GetObjName()
{
	return m_Name.GetValue();
}

string8 drgObjectProp::GetObjName8()
{
	return string8(GetObjName());
}


void drgObjectProp::SetObjectIDToNewUniqueValue()
{
	if (GetParentScene() == NULL)
	{
		assert(0 && "cannot create new object ID");
		return;
	}

	for (bool idAlreadyUsed = true; idAlreadyUsed; drgObjectProp::m_ObjectIDCounter++)
	{
		// drgObjectProp* obj = GetParentScene()->GetObjectByGUID(drgObjectProp::m_ObjectIDCounter);
		// if (obj != NULL)
		// 	continue;
		// drgAsset* asset = GetParentScene()->GetAssetByGUID(drgObjectProp::m_ObjectIDCounter);
		// if (asset != NULL)
		// 	continue;

		SetObjectID(drgObjectProp::m_ObjectIDCounter);
		idAlreadyUsed = false;
	}
}

// //saves the object data to xmlNodeOut
// void drgObjectProp::Save(drgXMLNode* xmlNodeOut)
// {
// 	const int numericBase = 10;

// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if (xmlNode->IsEmpty())
// 	{
// 		drgDebug::Notify("drgObject cannot save to empty node");
// 		return;
// 	}

// 	// save fields as attributes
// 	xmlNode->AddAttribute(DRG_SCENE_OBJECT_TYPE_ATTRIBUTE, m_TypeString.c_str());

// 	char tempString[DRG_PROPERTY_NAME_MAX_SIZE];

// 	drgString::ItoA(GetObjectID(), tempString, numericBase);
// 	xmlNode->AddAttribute(DRG_OBJECT_OBJECT_ID_ATTRIBUTE_NAME, tempString);
// 	drgString::ItoA(m_Flags, tempString, numericBase);
// 	xmlNode->AddAttribute(DRG_OBJECT_FLAGS_ATTRIBUTE_NAME, tempString);

// 	if (m_Next != NULL)
// 	{
// 		drgString::ItoA(((drgObjectProp*)m_Next)->m_ObjectID, tempString, numericBase);
// 		xmlNode->AddAttribute(DRG_OBJECT_NEXT_OBJECT_ID_ATTRIBUTE_NAME, tempString);
// 	}

// 	if (m_Prev != NULL)
// 	{
// 		drgString::ItoA(((drgObjectProp*)m_Prev)->m_ObjectID, tempString, numericBase);
// 		xmlNode->AddAttribute(DRG_OBJECT_PREV_OBJECT_ID_ATTRIBUTE_NAME, tempString);
// 	}

// 	// Save the properties
// 	if (m_PropertyList != NULL)
// 	{
// 		drgXMLNode propertiesNode = xmlNode->AddChild(DRG_PROPERTIES_NODE_NAME);
// 		if (propertiesNode->IsEmpty())
// 		{
// 			drgDebug::Notify("drgObject cannot create properties node");
// 			return;
// 		}

// 		drgString16Property* nameProp = (drgString16Property*)(m_PropertyList->GetProperty(DRG_OBJECT_NAME_PROPERTY_NAME));

// 		nameProp->Save(&propertiesNode, DRG_OBJECT_NAME_ATTRIBUTE);
// 	}
// }

// //loads xmlNodeIn and replaces this object's data
// void drgObjectProp::Load(drgXMLNode* xmlNodeIn)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if (xmlNode->IsEmpty())
// 	{
// 		drgPrintOut("null obj node when loading");
// 		return;
// 	}

// 	//
// 	// Load our object ID.
// 	//

// 	const char* objIDStr = xmlNode->GetAttribute(DRG_OBJECT_OBJECT_ID_ATTRIBUTE_NAME);
// 	assert(objIDStr && "Every object needs an unique ID");
// 	SetObjectID(drgString::AtoI(objIDStr));

// 	//
// 	// Load our object flags.
// 	//

// 	RemoveObjectFlag(-1);//clear all flags
// 	const char* flagsStr = xmlNode->GetAttribute(DRG_OBJECT_FLAGS_ATTRIBUTE_NAME);
// 	SetObjectFlag(drgString::AtoI(flagsStr));

// 	//
// 	// Load Properties
// 	//

// 	if (m_PropertyList == NULL)
// 		m_PropertyList = new drgPropertyList;

// 	drgXMLNode propertiesNode = xmlNode->GetChildNode(DRG_PROPERTIES_NODE_NAME);
// 	if (propertiesNode->IsEmpty())
// 		return;

// 	// Load our object name
// 	drgString16Property* nameProp = (drgString16Property*)GetProperty(DRG_OBJECT_NAME_PROPERTY_NAME);
// 	assert(nameProp);
// 	nameProp->Load(&propertiesNode, DRG_OBJECT_NAME_ATTRIBUTE);

// 	// Load our object type
// 	//m_TypeString.Load(xmlNodeIn, DRG_OBJECT_TYPE_ATTRIBUTE);
// }

void drgObjectProp::RemoveAssetReference(drgAsset* asset, bool unused)
{
	for (int i = 0; i < GetNumProperties(); i++)
	{
		drgProperty* properT = GetProperty(i);
		if (properT != NULL)
		{
			properT->RemoveAssetReference(asset);
		}
	}
}

string8 drgObjectProp::GetSceneFolder()
{
	// if (m_ParentScene != NULL)
	// {
	// 	string8 sceneFolder = m_ParentScene->GetSceneFolder();
	// 	return sceneFolder;
	// }
	// else
	// 	return string8(drgFile::GetResourceFolder());
}

void drgObjectProp::AddProperty(drgProperty* prop)
{
	if (m_PropertyList == NULL)
	{
		m_PropertyList = new drgPropertyList;
	}

	m_PropertyList->AddProperty(prop);
}

void drgObjectProp::RemoveProperty(drgProperty* prop)
{
	if (m_PropertyList == NULL)
		return;

	m_PropertyList->RemoveProperty(prop);
}

drgProperty* drgObjectProp::GetProperty(int index)
{
	if (m_PropertyList == NULL)
		return NULL;

	return m_PropertyList->GetProperty(index);
}

drgProperty* drgObjectProp::GetProperty(const char* propName)
{
	if (m_PropertyList == NULL)
		return NULL;

	return m_PropertyList->GetProperty(propName);
}

const char* drgObjectProp::GetPropertyName(int index)
{
	if (m_PropertyList == NULL)
		return NULL;

	return m_PropertyList->GetPropertyName(index);
}

int drgObjectProp::GetNumProperties()
{
	if (m_PropertyList == NULL)
		return 0;

	return m_PropertyList->GetNumProperties();
}

string16 drgObjectProp::GetTreeName()
{
	return m_Name.GetValue();
}

drgPropertyList* drgObjectProp::GetPropertiesInCommon(drgPropertyList* secondList, drgObjectProp* newParent)
{
	if (m_PropertyList == NULL)
		return NULL;

	return m_PropertyList->GetPropertiesInCommon(secondList, newParent);
}






drgObjectList::drgObjectList()
{
	m_Count = 0;
	m_Root = NULL;
}

drgObjectList::~drgObjectList()
{
	CleanUp();
}

void drgObjectList::CleanUp()
{
	drgObjectBase* del;
	drgObjectBase* itr = m_Root;
	while(itr)
	{
		del = itr;
		itr = itr->m_Next;
		del->m_Prev = NULL;
		del->m_Next = NULL;
		del->DelayedDelete();
	}
	m_Count = 0;
	m_Root = NULL;
}

void drgObjectList::AddHead(drgObjectBase* obj)
{
	if(m_Root != NULL)
		m_Root->m_Prev = obj;
	obj->m_Next = m_Root;
	obj->m_Prev = NULL;
	m_Root = obj;
	m_Count++;
}

void drgObjectList::AddTail(drgObjectBase* obj)
{
	if(m_Root == NULL)
	{
		obj->m_Prev = NULL;
		m_Root = obj;
	}
	else
	{
		drgObjectBase* itr = m_Root;
		while(itr->m_Next != NULL)
			itr = itr->m_Next;
		itr->m_Next = obj;
		obj->m_Prev = itr;
	}
	obj->m_Next = NULL;
	m_Count++;
}

void drgObjectList::AddAfter(drgObjectBase* obj, drgObjectBase* after_obj)
{
	drgObjectBase* itr = m_Root;
	while(itr)
	{
		if(after_obj==itr)
		{
			obj->m_Prev = itr;
			obj->m_Next = itr->m_Next;
			itr->m_Next = obj;
			obj->m_Next->m_Prev = obj;
			m_Count++;
			return;
		}
		itr = itr->m_Next;
	}
	AddTail(obj);
}

void drgObjectList::Remove(drgObjectBase* obj)
{
	if(obj==m_Root)
		m_Root = obj->m_Next;
	if(obj->m_Prev != NULL)
		obj->m_Prev->m_Next = obj->m_Next;
	if(obj->m_Next != NULL)
		obj->m_Next->m_Prev = obj->m_Prev;
	obj->m_Prev = NULL;
	obj->m_Next = NULL;
	m_Count--;
}

bool drgObjectList::InList(drgObjectBase* obj)
{
	drgObjectBase* itr = m_Root;
	while(itr)
	{
		if(obj==itr)
			return true;
		itr = itr->m_Next;
	}
	return false;
}




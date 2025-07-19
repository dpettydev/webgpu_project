
#include "core/property.h"
#include "core/object.h"
#include "util/stringn.h"
#include "util/stringv.h"
#include "render/gui/gui_properties.h"


#define DRG_PROPERTY_VALUE_STR_MAX 256

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgProperty::drgProperty()
{
	m_ParentObject = NULL;
	m_Name[ 0 ] = '\0';
	m_OnChangedCallback = NULL;
	m_RunningCallback = false;
}

drgProperty::~drgProperty()
{
}

drgProperty& drgProperty::operator=( const drgProperty& inRef )
{
	m_ParentObject = inRef.m_ParentObject;
	m_OnChangedCallback = inRef.m_OnChangedCallback;
	m_RunningCallback = inRef.m_RunningCallback;
	drgString::Copy(m_Name, inRef.m_Name);

	return *this;
}

void drgProperty::Init( const char* name, drgObjectProp* object )
{
	// Set our name
	int length = drgString::Length( name );
	assert( length + 1 <= DRG_PROPERTY_NAME_MAX_SIZE );
	drgString::CopySafe( m_Name, name, length + 1 );

	// Add us to our parent's list
	if(object != NULL)
	{
		m_ParentObject = object;
		m_ParentObject->AddProperty( this );
	}
}

void drgProperty::RunOnChangedCallback(void* data)
{
	if (m_OnChangedCallback != NULL)
	{
		if (!m_RunningCallback)
		{
			m_RunningCallback = true;
			m_OnChangedCallback(m_ParentObject, data);
			m_RunningCallback = false;
		}
	}
}

char* drgProperty::GetPropertyName()
{
	return m_Name;
}

void drgProperty::SetPropertyName(const char* name)
{
	drgString::CopySafe(m_Name, name, DRG_PROPERTY_NAME_MAX_SIZE);
}

void drgProperty::SetOnChangedCallback(drgCallbackObj callback)
{
	m_OnChangedCallback = callback;
}
		
// void drgProperty::Save(drgXMLNode* xmlNodeOut, const char* name)
// {
// 	drgDebug::Notify("drgProperty::Save() NOT IMPLEMENTED (Use derived class instead) \n");
// }

// void drgProperty::Load(drgXMLNode* xmlNodeIn, const char* name) 
// {
// 	drgPrintOut("drgProperty::Load() NOT IMPLEMENTED (Use derived class instead) \n");
// }

GUI_Property* drgProperty::CreateGUIWidget()
{ 
	drgPrintOut("drgProperty::CreateGUIWidget() NOT IMPLEMENTED (Use derived class instead) \n");
	return NULL; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgPropertyList
///////////////////////////////////////////////////////////////////////////////////////////////////

drgPropertyList::drgPropertyList()
{
	m_NumProperties = 0;
	m_Properties = NULL;
}

drgPropertyList::~drgPropertyList()
{
	Destroy();
}

void drgPropertyList::AddProperty( drgProperty* prop )
{
	assert( prop );

	drgProperty** prevList = m_Properties;
	m_Properties = (drgProperty**)drgMemAlloc( sizeof(drgProperty*) * ++m_NumProperties );

	for( int currentProp = 0; currentProp < m_NumProperties - 1; ++currentProp )
	{
		m_Properties[ currentProp ] = prevList[ currentProp ];
	}

	m_Properties[ m_NumProperties - 1 ] = prop;
	SAFE_FREE( prevList );
}

void drgPropertyList::RemoveProperty( drgProperty* prop )
{
	assert( prop );

	//
	// Handle the special case of removing the last property.
	//

	if( m_NumProperties - 1 == 0 &&
		m_Properties &&
		m_Properties[ 0 ] == prop )
	{
		m_NumProperties--;
		SAFE_FREE( m_Properties );
	}
	else
	{
		int offset = 0;
		drgProperty** prevList = m_Properties;
		m_Properties = (drgProperty**)drgMemAlloc( sizeof(drgProperty*) * --m_NumProperties );

		for( int currentProp = 0; currentProp < m_NumProperties; ++currentProp )
		{
			if( prevList[ currentProp + offset ] != prop )
			{
				m_Properties[ currentProp ] = prevList[ currentProp + offset ];
			}
			else
			{
				offset++;
			}
		}

		SAFE_FREE( prevList );
	}
}

void drgPropertyList::Destroy()
{
	SAFE_FREE( m_Properties );
	m_NumProperties = 0;
}

drgProperty* drgPropertyList::GetProperty( int index )
{
	assert( index >= 0 && index < m_NumProperties );
	return m_Properties[ index ];
}

drgProperty* drgPropertyList::GetProperty( const char* name )
{
	for( int currentProperty = 0; currentProperty < m_NumProperties; ++currentProperty )
	{
		if( m_Properties[ currentProperty ] )
		{
			if( drgString::CompareNoCase( m_Properties[ currentProperty ]->GetPropertyName(), name ) == 0 )
			{
				return m_Properties[ currentProperty ];
			}
		}
	}

	return NULL;
}
		
drgProperty* drgPropertyList::GetPropertyWithType( int index, const char* type)
{
	assert( index >= 0 && index < m_NumProperties );

	if( drgString::CompareNoCase( m_Properties[index]->GetPropertyType(), type ) == 0 )
		return m_Properties[ index ];
	else
		return NULL;
}
		
drgProperty* drgPropertyList::GetPropertyWithType( const char* propertyName, const char* type)
{
	for( int currentProperty = 0; currentProperty < m_NumProperties; ++currentProperty )
	{
		if( m_Properties[ currentProperty ] )
		{
			if( drgString::CompareNoCase( m_Properties[ currentProperty ]->GetPropertyName(),
				propertyName, drgString::Length( propertyName ) ) == 0 )
			{
				if(drgString::CompareNoCase( m_Properties[ currentProperty ]->GetPropertyType(), type ) == 0)
					return m_Properties[ currentProperty ];
			}
		}
	}

	return NULL;
}

const char* drgPropertyList::GetPropertyName( int index )
{
	assert((index >= 0) && (index < m_NumProperties));
	return m_Properties[index]->GetPropertyName();
}
		
drgPropertyList* drgPropertyList::GetPropertiesInCommon(drgPropertyList* secondList, drgObjectProp* newParent)
{
	drgPrintError("This whole function needs to be revisited, we shouldn't need a new object to store the list.");
	//int nProps = secondList->GetNumProperties();
	//drgProperty* propInCommon = NULL;
	//drgPropertyList* newList = new drgPropertyList;

	//if(nProps == 0)
	//{
	//	return NULL;
	//}

	//for(int i = 0; i < nProps; i++)
	//{
	//	drgProperty* propName = secondList->GetProperty(i);

	//	propInCommon = GetPropertyWithType(propName->GetPropertyName(), propName->GetPropertyType());

	//	if(propInCommon != NULL)
	//	{
	//		const char* type = propInCommon->GetPropertyType();

	//		if( drgString::CompareNoCase( type, drgIntProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgIntProperty* newIntProp = new drgIntProperty;
	//			newIntProp->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newIntProp);
	//		}
	//		else if( drgString::CompareNoCase( type, drgFloatProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgFloatProperty* newFloatProp = new drgFloatProperty;
	//			newFloatProp->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newFloatProp);
	//		}
	//		else if( drgString::CompareNoCase( type, drgBooleanProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgBooleanProperty* newBooleanProp = new drgBooleanProperty;
	//			newBooleanProp->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newBooleanProp);
	//		}
	//		else if( drgString::CompareNoCase( type, drgVec3Property::GetPropertyType() ) == 0 )
	//		{
	//			drgVec3Property* newVec3Prop = new drgVec3Property;
	//			newVec3Prop->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newVec3Prop);
	//		}
	//		else if( drgString::CompareNoCase( type, drgVec2Property::GetPropertyType() ) == 0 )
	//		{
	//			drgVec2Property* newVec2Prop = new drgVec2Property;
	//			newVec2Prop->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newVec2Prop);
	//		}
	//		else if( drgString::CompareNoCase( type, drgString16Property::GetPropertyType() ) == 0 )
	//		{
	//			drgString16Property* newString16Prop = new drgString16Property;
	//			newString16Prop->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newString16Prop);
	//		}
	//		else if( drgString::CompareNoCase( type, drgTransformProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgTransformProperty* newTransformProp = new drgTransformProperty;
	//			newTransformProp->Init(propInCommon->GetPropertyName(), newParent);
	//			newList->AddProperty(newTransformProp);
	//		}
	//		else if( drgString::CompareNoCase( type, drgAssetProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgAssetProperty* newAssetProp = new drgAssetProperty;
	//			newAssetProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newAssetProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgTextureAssetProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgTextureAssetProperty* newTexAssetProp = new drgTextureAssetProperty;
	//			newTexAssetProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newTexAssetProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgFontAssetProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgFontAssetProperty* newFontAssetProp = new drgFontAssetProperty;
	//			newFontAssetProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newFontAssetProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgParticleAssetProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgParticleAssetProperty* newAssetProp = new drgParticleAssetProperty;
	//			newAssetProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newAssetProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgModelAssetProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgModelAssetProperty* newAssetProp = new drgModelAssetProperty;
	//			newAssetProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newAssetProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgAssetReferenceProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgAssetReferenceProperty* newProp = new drgAssetReferenceProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgTextureAssetReferenceProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgTextureAssetReferenceProperty* newProp = new drgTextureAssetReferenceProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgFontAssetReferenceProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgFontAssetReferenceProperty* newProp = new drgFontAssetReferenceProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgParticleAssetReferenceProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgParticleAssetReferenceProperty* newProp = new drgParticleAssetReferenceProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgModelAssetReferenceProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgModelAssetReferenceProperty* newProp = new drgModelAssetReferenceProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else if( drgString::CompareNoCase( type, drgTextureListProperty::GetPropertyType() ) == 0 )
	//		{
	//			drgTextureListProperty* newProp = new drgTextureListProperty;
	//			newProp->Init( propInCommon->GetPropertyName(), newParent );
	//			newList->AddProperty( newProp );
	//		}
	//		else
	//		{
	//			assert( "Provided an unknown property type" && 0 );
	//			break;
	//		}
	//	}
	//}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgGenericProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgGenericProperty::drgGenericProperty()
{
	m_Value = NULL;
}

drgGenericProperty::~drgGenericProperty()
{

}

//serializes the object data to xmlNodeOut
// void drgGenericProperty::Save(drgXMLNode* xmlNodeOut, const char* name) 
// { 
// 	drgDebug::Notify("drgGenericProperty::Save() NOT IMPLEMENTED \n");
// }

// void drgGenericProperty::Load(drgXMLNode* xmlNodeIn, const char* name)
// {
// 	drgPrintOut("drgGenericProperty::Load() NOT IMPLEMENTED \n");
// }

GUI_Property* drgGenericProperty::CreateGUIWidget()
{ 
	drgPrintOut("drgGenericProperty::CreateGUIWidget() NOT IMPLEMENTED \n");
	return NULL; 
}

void drgGenericProperty::SetValue( void* value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value); 
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// drgIntProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgIntProperty::drgIntProperty()
{
	m_Value = 0;
}

drgIntProperty::~drgIntProperty()
{

}

// //serializes the object data to xmlNodeOut
// void drgIntProperty::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	char intStr[DRG_PROPERTY_VALUE_STR_MAX];
// 	drgString::ItoA(m_Value, intStr, 10);
// 	xmlNode->AddAttribute(attributeName, intStr);
// }

// void drgIntProperty::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);
// 	if(!propertyValueStr)
// 		return;
// 	m_Value = drgString::AtoI(propertyValueStr);
// }

GUI_Property* drgIntProperty::CreateGUIWidget()
{ 
	return new GUI_IntProperty; 
}

void drgIntProperty::SetValue( int value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgFloatProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgFloatProperty::drgFloatProperty()
{
	m_Value = 0.0f;
}

drgFloatProperty::~drgFloatProperty()
{

}

drgFloatProperty& drgFloatProperty::operator=( const drgFloatProperty& inRef )
{
	drgProperty::operator=(inRef);
	m_Value = inRef.m_Value;

	return *this;
}

// //serializes the object data to xmlNodeOut
// void drgFloatProperty::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if(xmlNode->IsEmpty())
// 		return;

// 	char floatStr[DRG_PROPERTY_VALUE_STR_MAX];
// 	drgString::FtoA(m_Value, floatStr);

// 	xmlNode->AddAttribute(attributeName, floatStr);
// }

// void drgFloatProperty::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
	
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);

// 	if(!propertyValueStr)
// 		return;

// 	m_Value = drgString::AtoF(propertyValueStr);
// }

GUI_Property* drgFloatProperty::CreateGUIWidget()
{ 
	return new GUI_FloatProperty; 
}

void drgFloatProperty::SetValue( float value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgBooleanProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgBooleanProperty::drgBooleanProperty()
{
	m_Value = false;
}

drgBooleanProperty::~drgBooleanProperty()
{

}

// //serializes the object data to xmlNodeOut
// void drgBooleanProperty::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* boolString = m_Value ? DRG_BOOL_STRING_TRUE : DRG_BOOL_STRING_FALSE;
// 	xmlNode->AddAttribute(attributeName, boolString);
// }

// void drgBooleanProperty::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);
// 	if(!propertyValueStr)
// 		return;
// 	m_Value = drgString::CompareNoCase(propertyValueStr, DRG_BOOL_STRING_FALSE, 1) != 0;
// }

GUI_Property* drgBooleanProperty::CreateGUIWidget()
{ 
	return new GUI_BooleanProperty; 
}

void drgBooleanProperty::SetValue( bool value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgVec3Property
///////////////////////////////////////////////////////////////////////////////////////////////////

drgVec3Property::drgVec3Property()
{
	drgVec3 point(0,0,0);
	m_Value = point;
}

drgVec3Property::~drgVec3Property()
{

}

// //serializes the object data to xmlNodeOut
// void drgVec3Property::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	char pointStr[DRG_PROPERTY_VALUE_STR_MAX];
// 	m_Value.c_str(pointStr);
// 	xmlNode->AddAttribute(attributeName, pointStr);
// }

// void drgVec3Property::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);
// 	if(!propertyValueStr)
// 		return;
// 	drgVec3 p3 = drgVec3();
// 	p3.Parse(propertyValueStr);
// 	m_Value = p3;
// }

GUI_Property* drgVec3Property::CreateGUIWidget()
{ 
	return new GUI_Vec3Property; 
}

void drgVec3Property::SetValue( drgVec3 value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value); 
}

void drgVec3Property::SetValue( float x, float y, float z )
{
	 m_Value.x = x; 
	 m_Value.y = y; 
	 m_Value.z = z;
	RunOnChangedCallback(&m_Value);
}

void drgVec3Property::SetValueX( float x )
{
	 m_Value.x = x; 
	RunOnChangedCallback(&m_Value);
}

void drgVec3Property::SetValueY( float y )
{
	 m_Value.y = y; 
	RunOnChangedCallback(&m_Value);
}

void drgVec3Property::SetValueZ( float z )
{
	 m_Value.z = z;
	RunOnChangedCallback(&m_Value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgVec2Property
///////////////////////////////////////////////////////////////////////////////////////////////////

drgVec2Property::drgVec2Property()
{
	drgVec2 point(0,0);
	m_Value = point;
}

drgVec2Property::~drgVec2Property()
{

}

// //serializes the object data to xmlNodeOut
// void drgVec2Property::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	char pointStr[DRG_PROPERTY_VALUE_STR_MAX];
// 	m_Value.c_str(pointStr);
// 	xmlNode->AddAttribute(attributeName, pointStr);
// }

// void drgVec2Property::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);
// 	if(!propertyValueStr)
// 		return;
// 	drgVec2 p2 = drgVec2();
// 	p2.Parse(propertyValueStr);
// 	m_Value = p2;
// }

GUI_Property* drgVec2Property::CreateGUIWidget()
{ 
	drgPrintOut("drgVec2Property::CreateGUIWidget() NOT IMPLEMENTED \n");
	return NULL; 
}

void drgVec2Property::SetValue( drgVec2 value )
{
	m_Value = value; 
	RunOnChangedCallback(&m_Value);
}

void drgVec2Property::SetValue( float x, float y )
{
	m_Value.x = x; 
	m_Value.y = y; 
	RunOnChangedCallback(&m_Value);
}

void drgVec2Property::SetValueX( float x )
{
	m_Value.x = x; 
	RunOnChangedCallback(&m_Value);
}

void drgVec2Property::SetValueY( float y )
{
	m_Value.y = y; 
	RunOnChangedCallback(&m_Value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgString16Property
///////////////////////////////////////////////////////////////////////////////////////////////////

drgString16Property::drgString16Property()
{
	m_Value = "";
}

drgString16Property::~drgString16Property()
{

}

// //serializes the object data to xmlNodeOut
// void drgString16Property::Save(drgXMLNode* xmlNodeOut, const char* attributeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;	
// 	if(xmlNode->IsEmpty())
// 		return;
// 	xmlNode->AddAttribute(attributeName, string8(m_Value).c_str());
// }

// void drgString16Property::Load(drgXMLNode* xmlNodeIn, const char* attributeName)
// {
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;
// 	const char* propertyValueStr = xmlNode->GetAttribute(attributeName);
// 	if(!propertyValueStr)
// 		return;
// 	m_Value = propertyValueStr;
// }

GUI_Property* drgString16Property::CreateGUIWidget()
{ 
	return new GUI_String16Property; 
}

void drgString16Property::SetValue( string16* value )
{
	m_Value = (*value);
	RunOnChangedCallback(&m_Value); 
}

void drgString16Property::SetValue( const char* value )
{
	m_Value = value;
	RunOnChangedCallback(&m_Value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgTransformProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgTransformProperty::drgTransformProperty()
{
	m_ValuePosition = {0.0f, 0.0f, 0.0f};
	m_ValueRotation = {0.0f, 0.0f, 0.0f};
	m_ValueScale = {1.0f, 1.0f, 1.0f};
}

drgTransformProperty::~drgTransformProperty()
{

}

drgTransformProperty& drgTransformProperty::operator=( const drgTransformProperty& inRef )
{
	drgProperty::operator=( inRef );
	m_ValuePosition = inRef.m_ValuePosition;
	m_ValueRotation = inRef.m_ValueRotation;
	m_ValueScale = inRef.m_ValueScale;

	return *this;
}

void drgTransformProperty::SetValue( drgVec3 position, drgVec3 rotation, drgVec3 scale )
{
	m_ValuePosition = position;
	m_ValueRotation = rotation;
	m_ValueScale = scale;
	RunOnChangedCallback(this);
}

void drgTransformProperty::SetPosition( drgVec3* position )			
{ 
	m_ValuePosition = *position; 
	RunOnChangedCallback(this); 
}

void drgTransformProperty::SetPosition( float x, float y, float z )	
{ 
	m_ValuePosition.Set( x, y, z ); 
	RunOnChangedCallback(this);
}

void drgTransformProperty::SetScale( drgVec3* scale )				
{ 
	m_ValueScale = *scale; 
	RunOnChangedCallback(this); 
}

void drgTransformProperty::SetScale( float x, float y, float z )		
{ 
	m_ValueScale.Set( x, y, z ); 
	RunOnChangedCallback(this);
}

void drgTransformProperty::SetRotation( drgVec3* rotation )			
{ 
	m_ValueRotation = *rotation; 
	RunOnChangedCallback(this);
}

void drgTransformProperty::SetRotation( float x, float y, float z )	
{ 
	m_ValueRotation.Set( x, y, z ); 
	RunOnChangedCallback(this);
}

void drgTransformProperty::GetValue( drgVec3* position, drgVec3* rotation, drgVec3* scale )
{
	assert( position && scale && rotation );

	position->Set( m_ValuePosition.x, m_ValuePosition.y, m_ValuePosition.z );
	rotation->Set( m_ValueRotation.x, m_ValueRotation.y, m_ValueRotation.z );
	scale->Set( m_ValueScale.x, m_ValueScale.y, m_ValueScale.z );
}

// //serializes the object data to xmlNodeOut
// void drgTransformProperty::Save(drgXMLNode* xmlNodeOut, const char* nodeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeOut;	
// 	if(xmlNode->IsEmpty())
// 		return;

// 	drgXMLNode transformNode = xmlNode->AddChild(nodeName);
		
// 	//Position
// 	drgVec3Property tempPositionProp = drgVec3Property();
// 	tempPositionProp.Init("", NULL);
// 	tempPositionProp.SetValue(m_ValuePosition);
// 	tempPositionProp.Save(&transformNode, DRG_TRANSFORM_PROPERTY_POSITION_NAME);
		
// 	//Rotation
// 	drgVec3Property tempRotationProp = drgVec3Property();
// 	tempRotationProp.Init("", NULL);
// 	tempRotationProp.SetValue(m_ValueRotation);
// 	tempRotationProp.Save(&transformNode, DRG_TRANSFORM_PROPERTY_ROTATION_NAME);
		
// 	//Scale
// 	drgVec3Property tempScaleProp = drgVec3Property();
// 	tempScaleProp.Init("", NULL);
// 	tempScaleProp.SetValue(m_ValueScale);
// 	tempScaleProp.Save(&transformNode, DRG_TRANSFORM_PROPERTY_SCALE_NAME);
// }

// void drgTransformProperty::Load(drgXMLNode* xmlNodeIn, const char* nodeName)
// {	
// 	drgXMLNode xmlNode = *xmlNodeIn;
// 	if(xmlNode->IsEmpty())
// 		return;

// 	drgXMLNode transformNode = xmlNode->GetChildNode(nodeName);
		
// 	//Position
// 	drgVec3Property tempPositionProp = drgVec3Property();
// 	tempPositionProp.Init("", NULL);
// 	tempPositionProp.Load(&transformNode, DRG_TRANSFORM_PROPERTY_POSITION_NAME);
// 	drgVec3 pos = tempPositionProp.GetValue();
		
// 	//Rotation
// 	drgVec3Property tempRotationProp = drgVec3Property();
// 	tempRotationProp.Init("", NULL);
// 	tempRotationProp.Load(&transformNode, DRG_TRANSFORM_PROPERTY_ROTATION_NAME);
// 	drgVec3 rot = tempRotationProp.GetValue();
		
// 	//Scale
// 	drgVec3Property tempScaleProp = drgVec3Property();
// 	tempScaleProp.Init("", NULL);
// 	tempScaleProp.Load(&transformNode, DRG_TRANSFORM_PROPERTY_SCALE_NAME);
// 	drgVec3 scale = tempScaleProp.GetValue();

// 	SetValue(pos, rot, scale);
// }

GUI_Property* drgTransformProperty::CreateGUIWidget()
{ 
	return new GUI_TransformProperty; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgObjectReferenceProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgObjectReferenceProperty::drgObjectReferenceProperty()
{
	m_Object = NULL;
	m_Scene = NULL;
	m_ObjectName = "Unknown Object Reference Name";
}

drgObjectReferenceProperty::~drgObjectReferenceProperty()
{

}

drgObjectReferenceProperty& drgObjectReferenceProperty::operator=( const drgObjectReferenceProperty& inRef )
{
	drgProperty::operator=( inRef );
	m_Object = inRef.m_Object;
	m_ObjectName = inRef.m_ObjectName;
	m_Scene = inRef.m_Scene;

	return *this;
}

drgObjectProp* drgObjectReferenceProperty::GetValue()
{
	// if( m_Object == NULL && m_Scene )
	// {
	// 	m_Object = m_Scene->GetObjectByTreeName(&m_ObjectName);
	// }

	//if( m_Object == NULL )
	//{
	//	int sceneIsNULL = m_Scene == NULL ? 1 : 0;
	//	string8 name = m_ObjectName;
	//	drgPrintOut( "Failed to find object from object reference with name: %s, scene is NULL: %d\n", name.c_str(), sceneIsNULL );
	//}

	return m_Object;
}

void drgObjectReferenceProperty::SetValue(drgObjectProp* object)
{
	m_Object = object;
	if( m_Object )
	{
		m_ObjectName = m_Object->GetObjName();
	}
	else
	{
		m_ObjectName = "Unknown Object Reference Name";
	}
}

// void drgObjectReferenceProperty::Load( drgXMLNode* node, const char* nodeName )
// {
// 	assert( m_Scene );

// 	const char* objectName = (*node)->GetAttribute( nodeName );
// 	if( objectName == NULL )
// 	{
// 		return;
// 	}

// 	m_ObjectName = objectName;
// 	m_Object = GetValue();
// }

// void drgObjectReferenceProperty::Save( drgXMLNode* node, const char* nodeName )
// {
// 	if (GetValue() == NULL)
// 		return;

// 	string8 name = m_Object->GetTreeName();
// 	(*node)->AddAttribute(nodeName, name.c_str() );
// }

GUI_Property* drgObjectReferenceProperty::CreateGUIWidget()
{
	return new GUI_ObjectReferenceProperty;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// drgColorProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgColorProperty::drgColorProperty()
	: m_Value( 255, 255, 255, 255 )
{
}

drgColorProperty::~drgColorProperty()
{
}

drgColorProperty& drgColorProperty::operator=( const drgColorProperty& inRef )
{
	drgProperty::operator=( inRef );
	m_Value = inRef.m_Value;

	return *this;
}

void drgColorProperty::SetValue( float r, float g, float b, float a )
{
	m_Value.r = (unsigned char)(r * 255.0f);
	m_Value.g = (unsigned char)(g * 255.0f); 
	m_Value.b = (unsigned char)(b * 255.0f); 
	m_Value.a = (unsigned char)(a * 255.0f); 
}

drgVec4 drgColorProperty::GetValueVec4()
{
	drgVec4 toReturn( (float)m_Value.r / 255.0f,
		(float)m_Value.g / 255.0f,
		(float)m_Value.b / 255.0f,
		(float)m_Value.a / 255.0f );

	return toReturn;
}

// void drgColorProperty::Load( drgXMLNode* nodeIn, const char* attributeName )
// {
// 	assert( nodeIn && nodeIn->GetData()->IsEmpty() == false );

// 	const char* propertyValueStr = nodeIn->GetData()->GetAttribute(attributeName);
// 	if( propertyValueStr )
// 	{
// 		unsigned int value = (unsigned int)drgString::AtoI( propertyValueStr );
// 		m_Value.Set( value );
// 	}
// }

// void drgColorProperty::Save( drgXMLNode* nodeOut, const char* attributeName )
// {
// 	assert( nodeOut );

// 	char value[ 32 ];
// 	drgString::ItoA( (int)m_Value.Get8888(), value, 10 );

// 	nodeOut->GetData()->AddAttribute( attributeName, value );
// }

GUI_Property* drgColorProperty::CreateGUIWidget()
{
	return new GUI_ColorProperty;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// drgEnumProperty
///////////////////////////////////////////////////////////////////////////////////////////////////

drgEnumProperty::drgEnumProperty() 
{
	m_EnumValues = NULL;
	m_EnumValueCount = 0;
}

drgEnumProperty::~drgEnumProperty() 
{
	for (int i = 0; i < m_EnumValueCount; i++)
	{
		delete m_EnumValues[i];
	}
	drgMemFree(m_EnumValues);
	m_EnumValueCount = 0;
}

// void drgEnumProperty::Load( drgXMLNode* nodeIn, const char* attributeName )
// {
// 	assert( nodeIn && attributeName );

// 	const char* text = nodeIn->GetData()->GetAttribute( attributeName );
// 	if( text )
// 	{
// 		SetValue( drgString::AtoI( text ) );
// 	}
// }

// void drgEnumProperty::Save( drgXMLNode* nodeOut, const char* attributeName )
// {
// 	assert( nodeOut && attributeName );

// 	char valueText[ 128 ];
// 	drgString::ItoA( m_Value, valueText, 10 );
// 	nodeOut->GetData()->AddAttribute( attributeName, valueText ); 
// }

const char* drgEnumProperty::GetNameOfCurrentValue()
{
	return GetNameOfValue(m_Value);
}

const char* drgEnumProperty::GetNameOfValue(int val)
{
	for(int i = 0; i < GetNumberOfValues(); i++)
	{
		if(m_EnumValues[i] != NULL)
		{
			drgIntStringPair* enumVal = m_EnumValues[i];
			if(enumVal->GetKey() == val)
				return enumVal->GetValue();
		}
	}
	return NULL;
}

const char* drgEnumProperty::GetNameAtIndex(int idx)
{
	return m_EnumValues[idx]->GetValue();
}

void drgEnumProperty::SetValueByName(const char* name)
{
	for(int i = 0; i < m_EnumValueCount; i++)
	{
		if(drgString::Compare(m_EnumValues[i]->GetValue(), name) == 0)
			m_Value = m_EnumValues[i]->GetKey();
	}
}

void drgEnumProperty::AddEnum( int value, const char* name )
{
	drgIntStringPair** temp = m_EnumValues;
	m_EnumValues = (drgIntStringPair**)drgMemAlloc( sizeof(drgIntStringPair*) * (m_EnumValueCount + 1) );

	if( temp != NULL )
	{
		drgMemory::MemCopy( m_EnumValues, temp, sizeof(drgIntStringPair*) * m_EnumValueCount );
		SAFE_FREE( temp );
	}

	MEM_INFO_SET_NEW;
	drgIntStringPair* enumVal = new drgIntStringPair(value, name);
	m_EnumValues[ m_EnumValueCount ] = enumVal;

	m_EnumValueCount++;
}


GUI_Property* drgEnumProperty::CreateGUIWidget()
{
	return new GUI_EnumProperty;
}
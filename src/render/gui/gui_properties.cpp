
#include "render/gui/gui_properties.h"
#include "render/gui/gui.h"
#include "render/gui/gui_draw.h"
#include "core/property.h"
#include "core/input.h"
#include "util/stringn.h"

#define DRG_GUI_PROP_MIN_HEIGHT 25.0F
#define DRG_GUI_PROP_LABEL_WIDTH 100.0F
#define DRG_GUI_ASSET_REF_LABEL_WIDTH 150.0f
#define DRG_GUI_PROP_NAME_WIDTH 150.0F
#define DRG_GUI_ASSET_REF_BUTTON_WIDTH 150.0F
#define DRG_GUI_IMAGE_HEIGHT 128.0F
#define DRG_GUI_IMAGE_WIDTH 128.0F
#define DRG_GUI_BUTTON_WIDTH 32.0f
#define DRG_GUI_BUTTON_HEIGHT 32.0f

GUI_Property::GUI_Property()
{
	m_Property = NULL;
	m_Label = NULL;
	m_InputBox = NULL;
	m_Table = NULL;

	m_FlagsWidget |= DRG_WIDGET_RESIZE_EXPAND;
	m_WidgetType = GUI_TYPE_PROPERTY;
}

GUI_Property::~GUI_Property()
{
	// We don't actually own the property.
}

void GUI_Property::Init()
{
	assert(m_Property);

	SetMinHeight(DRG_GUI_PROP_MIN_HEIGHT);

	string16 propText = m_Property->GetPropertyName();

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(0.0f, 1.0f, 0.0f, false);
	m_Table->AddCol(DRG_GUI_PROP_LABEL_WIDTH, 0.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, true);

	GUI_Container *cell0 = m_Table->CreateCell(0, 1, 1, 1);
	m_InputBox = cell0->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell1 = m_Table->CreateCell(0, 0, 1, 1);
	m_Label = cell1->CreateChildLabel(propText, DRG_NO_FLAGS);
	m_Label->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	InitCollapseButton();
}

void GUI_Property::Destroy()
{
	SAFE_DELETE(m_Label);
	SAFE_DELETE(m_InputBox);
}

void GUI_Property::UpdateSize()
{
	GUI_Container::UpdateSize();

	if (m_Table != NULL)
	{
		m_Sens.maxv.y = m_Sens.minv.y + m_Table->GetSumOfRowHeights();
	}

	m_View = m_Sens;
}

bool GUI_Property::CallMouseDown(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	RunCallBack(GUI_CB_BUTTON_DOWN);

	return GUI_Container::CallMouseDown(winid, button);
}

bool GUI_Property::CallMouseClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	RunCallBack(GUI_CB_BUTTON_CLICK);

	return GUI_Container::CallMouseClick(winid, button);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_IntProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_IntProperty::GUI_IntProperty()
{
	m_WidgetType = GUI_TYPE_INT_PROPERTY;
}

GUI_IntProperty::~GUI_IntProperty()
{
}

void GUI_IntProperty::Init()
{
	GUI_Property::Init();
	assert(m_Label && m_InputBox);

	m_InputBox->SetCallBack(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_IntProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgIntProperty *prop = (drgIntProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false)
	{
		char text[32];
		drgString::ItoA(prop->GetValue(), text, 10);
		m_InputBox->SetText(text);
	}

	GUI_Container::Draw(draw);
}

void GUI_IntProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_IntProperty *prop = (GUI_IntProperty *)data;
	GUI_Input *input = (GUI_Input *)object;
	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		assert(prop->m_Property);
		string16 *text = prop->m_InputBox->GetText();
		string8 valueText = *text;
		int value = drgString::AtoI(valueText.c_str());

		drgIntProperty *intProp = (drgIntProperty *)prop->m_Property;
		intProp->SetValue(value);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_FloatProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_FloatProperty::GUI_FloatProperty()
{
	m_WidgetType = GUI_TYPE_FLOAT_PROPERTY;
	m_ValueChangedCallback = NULL;
	m_ValueChangedData = NULL;
}

GUI_FloatProperty::~GUI_FloatProperty()
{
}

void GUI_FloatProperty::Init()
{
	GUI_Property::Init();
	assert(m_Label && m_InputBox);

	m_InputBox->SetCallBack(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_FloatProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgFloatProperty *prop = (drgFloatProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false)
	{
		char text[32];
		drgString::FtoA(prop->GetValue(), text);
		m_InputBox->SetText(text);
	}

	GUI_Container::Draw(draw);
}

void GUI_FloatProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_FloatProperty *prop = (GUI_FloatProperty *)data;
	GUI_Input *input = (GUI_Input *)object;
	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		string16 *text = prop->m_InputBox->GetText();
		string8 valueText = *text;
		float value = drgString::AtoF(valueText.c_str());

		drgFloatProperty *floatProp = (drgFloatProperty *)prop->m_Property;
		floatProp->SetValue(value);

		if (prop->m_ValueChangedCallback)
		{
			prop->m_ValueChangedCallback(input, prop->m_ValueChangedData);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_BooleanProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_BooleanProperty::GUI_BooleanProperty()
{
	m_Checkbox = NULL;
	m_WidgetType = GUI_TYPE_BOOLEAN_PROPERTY;
}

GUI_BooleanProperty::~GUI_BooleanProperty()
{
}

void GUI_BooleanProperty::Init()
{
	GUI_Property::Init();
	assert(m_Label && m_InputBox);

	GUI_Container *fieldContainer = m_Table->GetCell(0, 1);
	fieldContainer->RemoveAllChildren();

	m_Checkbox = fieldContainer->CreateChildCheckBox(OnInputChanged, this, DRG_NO_FLAGS);

	InitCollapseButton();
}

void GUI_BooleanProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgBooleanProperty *prop = (drgBooleanProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (prop->GetValue())
	{
		m_Checkbox->SetState(DRG_BUTTON_STATE_DOWN);
	}
	else
	{
		m_Checkbox->SetState(DRG_BUTTON_STATE_UP);
	}

	GUI_Container::Draw(draw);
}

void GUI_BooleanProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_BooleanProperty *prop = (GUI_BooleanProperty *)data;
	GUI_Button *checkbox = (GUI_Button *)object;
	if (prop && checkbox)
	{
		if (checkbox->GetState() == DRG_BUTTON_STATE_DOWN)
		{
			drgBooleanProperty *boolProp = (drgBooleanProperty *)prop->m_Property;
			boolProp->SetValue(true);
		}
		else
		{
			drgBooleanProperty *boolProp = (drgBooleanProperty *)prop->m_Property;
			boolProp->SetValue(false);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_Vec3Property
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_Vec3Property::GUI_Vec3Property()
{
	m_WidgetType = GUI_TYPE_POINT_3_PROPERTY;
}

GUI_Vec3Property::~GUI_Vec3Property()
{
	// SAFE_DELETE( m_InputBox2 );
	// SAFE_DELETE( m_InputBox3 );
}

void GUI_Vec3Property::Init()
{
	// GUI_Property::Init();

	assert(m_Property);

	string16 propText = m_Property->GetPropertyName();

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddCol(DRG_GUI_PROP_LABEL_WIDTH, 0.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 2.0f, true);

	GUI_Container *cell0 = m_Table->CreateCell(0, 1, 1, 1);
	m_InputBox = cell0->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell1 = m_Table->CreateCell(0, 0, 1, 1);
	m_Label = cell1->CreateChildLabel(propText, DRG_NO_FLAGS);

	GUI_Container *cell2 = m_Table->CreateCell(0, 2, 1, 1);
	m_InputBox2 = cell2->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox2->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell3 = m_Table->CreateCell(0, 3, 1, 1);
	m_InputBox3 = cell3->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox3->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	assert(m_Label && m_InputBox && m_InputBox2 && m_InputBox3);

	SetCallback(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_Vec3Property::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgVec3Property *prop = (drgVec3Property *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false && m_InputBox2->IsFocused() == false && m_InputBox3->IsFocused() == false)
	{
		char text[32];
		drgString::FtoA(prop->GetValue().x, text);
		m_InputBox->SetText(text);
		drgString::FtoA(prop->GetValue().y, text);
		m_InputBox2->SetText(text);
		drgString::FtoA(prop->GetValue().z, text);
		m_InputBox3->SetText(text);
	}

	GUI_Container::Draw(draw);
}

void GUI_Vec3Property::UpdateSize()
{
	GUI_Property::UpdateSize();
}

void GUI_Vec3Property::SetCallback(drgCallbackObj callback, void *data)
{
	m_InputBox->SetCallBack(callback, data);
	m_InputBox2->SetCallBack(callback, data);
	m_InputBox3->SetCallBack(callback, data);
}

void GUI_Vec3Property::OnInputChanged(drgObjectBase *object, void *data)
{
	float x;
	float y;
	float z;
	GUI_Vec3Property *prop = (GUI_Vec3Property *)data;
	GUI_Input *input = (GUI_Input *)object;

	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		string16 *text = prop->m_InputBox->GetText();
		string8 valueTextX = *text;
		x = drgString::AtoF(valueTextX.c_str());

		text = prop->m_InputBox2->GetText();
		string8 valueTextY = *text;
		y = drgString::AtoF(valueTextY.c_str());

		text = prop->m_InputBox3->GetText();
		string8 valueTextZ = *text;
		z = drgString::AtoF(valueTextZ.c_str());

		drgVec3 point(x, y, z);

		drgVec3Property *vec3Prop = (drgVec3Property *)prop->m_Property;
		vec3Prop->SetValue(point);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_Vec2Property
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_Vec2Property::GUI_Vec2Property()
{
	m_WidgetType = GUI_TYPE_POINT_2_PROPERTY;
}

GUI_Vec2Property::~GUI_Vec2Property()
{
}

void GUI_Vec2Property::Init()
{
	assert(m_Property);

	string16 propText = m_Property->GetPropertyName();

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(0.0f, 1.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, true);

	GUI_Container *cell0 = m_Table->CreateCell(0, 1, 1, 1);
	m_InputBox = cell0->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell1 = m_Table->CreateCell(0, 0, 1, 1);
	m_Label = cell1->CreateChildLabel(propText, DRG_NO_FLAGS);
	m_Label->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell2 = m_Table->CreateCell(0, 2, 1, 1);
	m_InputBox2 = cell2->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox2->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	assert(m_Label && m_InputBox && m_InputBox2);

	m_InputBox->SetCallBack(OnInputChanged, this);
	m_InputBox2->SetCallBack(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_Vec2Property::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgVec2Property *prop = (drgVec2Property *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false)
	{
		char text[32];
		drgString::FtoA(prop->GetValue().x, text);
		m_InputBox->SetText(text);
		drgString::FtoA(prop->GetValue().y, text);
		m_InputBox2->SetText(text);
	}

	GUI_Container::Draw(draw);
}

void GUI_Vec2Property::OnInputChanged(drgObjectBase *object, void *data)
{
	float x;
	float y;
	GUI_Vec2Property *prop = (GUI_Vec2Property *)data;
	GUI_Input *input = (GUI_Input *)object;

	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		string16 *text = prop->m_InputBox->GetText();
		string8 valueTextX = *text;
		x = drgString::AtoF(valueTextX.c_str());

		text = prop->m_InputBox2->GetText();
		string8 valueTextY = *text;
		y = drgString::AtoF(valueTextY.c_str());

		drgVec2 point(x, y);

		drgVec2Property *point2Prop = (drgVec2Property *)prop->m_Property;
		point2Prop->SetValue(point);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_String16Property
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_String16Property::GUI_String16Property()
{
	m_WidgetType = GUI_TYPE_STRING16_PROPERTY;
}

GUI_String16Property::~GUI_String16Property()
{
}

void GUI_String16Property::Init()
{
	GUI_Property::Init();
	assert(m_Label && m_InputBox);

	m_InputBox->SetCallBack(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_String16Property::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgString16Property *prop = (drgString16Property *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false)
	{
		string16 temp_st = prop->GetValue();
		m_InputBox->SetText(&temp_st);
	}

	GUI_Container::Draw(draw);
}

void GUI_String16Property::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_String16Property *prop = (GUI_String16Property *)data;
	GUI_Input *input = (GUI_Input *)object;

	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		string16 *text = prop->m_InputBox->GetText();
		drgString16Property *str16Prop = (drgString16Property *)prop->m_Property;
		str16Prop->SetValue(text);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_TransformProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_TransformProperty::GUI_TransformProperty()
{
	m_PositionGUIProp = NULL;
	m_RotationGUIProp = NULL;
	m_ScaleGUIProp = NULL;

	m_WidgetType = GUI_TYPE_TRANSFORM_PROPERTY;
}

GUI_TransformProperty::~GUI_TransformProperty()
{
}

void GUI_TransformProperty::Init()
{
	assert(m_Property);

	drgVec3Property *positionProp = new drgVec3Property();
	positionProp->SetPropertyName("Position");
	drgVec3Property *rotationProp = new drgVec3Property();
	rotationProp->SetPropertyName("Rotation");
	drgVec3Property *scaleProp = new drgVec3Property();
	scaleProp->SetPropertyName("Scale");

	string16 propText = m_Property->GetPropertyName();

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, true);

	GUI_Container *labelCellChild = m_Table->CreateCell(0, 0, 1, 1);
	m_Label = labelCellChild->CreateChildLabel(propText, DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *positionCellChild = m_Table->CreateCell(1, 0, 1, 1);
	m_PositionGUIProp = (GUI_Vec3Property *)positionCellChild->CreateChildProperty(positionProp);
	m_PositionGUIProp->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);
	m_PositionGUIProp->SetContainerResizeType(DRG_CONTAINER_RESIZE_VERTICAL_ALIGN);

	GUI_Container *rotationCellChild = m_Table->CreateCell(2, 0, 1, 1);
	m_RotationGUIProp = (GUI_Vec3Property *)rotationCellChild->CreateChildProperty(rotationProp);
	m_RotationGUIProp->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);
	m_RotationGUIProp->SetContainerResizeType(DRG_CONTAINER_RESIZE_VERTICAL_ALIGN);

	GUI_Container *scaleCellChild = m_Table->CreateCell(3, 0, 1, 1);
	m_ScaleGUIProp = (GUI_Vec3Property *)scaleCellChild->CreateChildProperty(scaleProp);
	m_ScaleGUIProp->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);
	m_ScaleGUIProp->SetContainerResizeType(DRG_CONTAINER_RESIZE_VERTICAL_ALIGN);

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	assert(m_Label && m_PositionGUIProp && m_RotationGUIProp && m_ScaleGUIProp);

	m_PositionInputData.guiTransformProp = this;
	m_PositionInputData.guiVec3Prop = m_PositionGUIProp;
	m_PositionGUIProp->SetCallback(OnInputChanged, &m_PositionInputData);

	m_RotationInputData.guiTransformProp = this;
	m_RotationInputData.guiVec3Prop = m_RotationGUIProp;
	m_RotationGUIProp->SetCallback(OnInputChanged, &m_RotationInputData);

	m_ScaleInputData.guiTransformProp = this;
	m_ScaleInputData.guiVec3Prop = m_ScaleGUIProp;
	m_ScaleGUIProp->SetCallback(OnInputChanged, &m_ScaleInputData);

	InitCollapseButton();
}

void GUI_TransformProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgTransformProperty *prop = (drgTransformProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	drgVec3Property *position = (drgVec3Property *)m_PositionGUIProp->GetDisplayProperty();
	drgVec3 positionValue = *(prop->GetPosition());
	position->SetValue(positionValue);

	drgVec3Property *rotation = (drgVec3Property *)m_RotationGUIProp->GetDisplayProperty();
	drgVec3 rotationValue = *(prop->GetRotation());
	rotation->SetValue(rotationValue);

	drgVec3Property *scale = (drgVec3Property *)m_ScaleGUIProp->GetDisplayProperty();
	drgVec3 scaleValue = *(prop->GetScale());
	scale->SetValue(scaleValue);

	GUI_Container::Draw(draw);
}

void GUI_TransformProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_Input *input = (GUI_Input *)object;
	GUI_TransformPropInputChangeData *inputData = (GUI_TransformPropInputChangeData *)data;

	if (inputData && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		drgTransformProperty *transformProp = (drgTransformProperty *)inputData->guiTransformProp->GetDisplayProperty();
		GUI_Vec3Property *guiPropToUpdate = NULL;
		if (inputData->guiVec3Prop == inputData->guiTransformProp->m_PositionGUIProp)
		{
			guiPropToUpdate = inputData->guiTransformProp->m_PositionGUIProp;
			drgVec3Property *posProp = (drgVec3Property *)guiPropToUpdate->GetDisplayProperty();
			guiPropToUpdate->OnInputChanged(input, guiPropToUpdate);
			drgVec3 pos = posProp->GetValue();
			transformProp->SetPosition(&pos);
		}
		else if (inputData->guiVec3Prop == inputData->guiTransformProp->m_RotationGUIProp)
		{
			guiPropToUpdate = inputData->guiTransformProp->m_RotationGUIProp;
			drgVec3Property *rotProp = (drgVec3Property *)guiPropToUpdate->GetDisplayProperty();
			guiPropToUpdate->OnInputChanged(input, guiPropToUpdate);
			drgVec3 rot = rotProp->GetValue();
			transformProp->SetRotation(&rot);
		}
		else if (inputData->guiVec3Prop == inputData->guiTransformProp->m_ScaleGUIProp)
		{
			guiPropToUpdate = inputData->guiTransformProp->m_ScaleGUIProp;
			drgVec3Property *scaleProp = (drgVec3Property *)guiPropToUpdate->GetDisplayProperty();
			guiPropToUpdate->OnInputChanged(input, guiPropToUpdate);
			drgVec3 scale = scaleProp->GetValue();
			transformProp->SetScale(&scale);
		}
		else
			assert(0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_ColorProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_ColorProperty::GUI_ColorProperty()
{
	m_WidgetType = GUI_TYPE_POINT_3_PROPERTY;

	m_InputBox = NULL;
	m_InputBox2 = NULL;
	m_InputBox3 = NULL;
	m_InputBox4 = NULL;
	m_SwathBox = NULL;
}

GUI_ColorProperty::~GUI_ColorProperty()
{
	// Child GUI elements will be cleaned up in the container destructor.
}

void GUI_ColorProperty::Init()
{
	// GUI_Property::Init();

	assert(m_Property);

	string16 propText = m_Property->GetPropertyName();

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 0.0f, false);
	m_Table->AddCol(DRG_GUI_PROP_LABEL_WIDTH, 0.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false); // r
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false); // g
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false); // b
	m_Table->AddCol(0.0f, 1.0f, 2.0f, false); // a
	m_Table->AddCol(0.0f, 1.0f, 2.0f, true);  // swath

	GUI_Container *cell = m_Table->CreateCell(0, 0);
	m_Label = cell->CreateChildLabel(propText, DRG_NO_FLAGS);

	cell = m_Table->CreateCell(0, 1);
	m_InputBox = cell->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	cell = m_Table->CreateCell(0, 2);
	m_InputBox2 = cell->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox2->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	cell = m_Table->CreateCell(0, 3);
	m_InputBox3 = cell->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox3->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	cell = m_Table->CreateCell(0, 4);
	m_InputBox4 = cell->CreateChildInput(DRG_NO_FLAGS);
	m_InputBox4->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	cell = m_Table->CreateCell(0, 5);
	m_SwathBox = cell; // We just need the bounds.

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	assert(m_Label && m_InputBox && m_InputBox2 && m_InputBox3 && m_InputBox4);

	SetCallback(OnInputChanged, this);

	InitCollapseButton();
}

void GUI_ColorProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgColorProperty *prop = (drgColorProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_InputBox->IsFocused() == false && m_InputBox2->IsFocused() == false && m_InputBox3->IsFocused() == false && m_InputBox4->IsFocused() == false)
	{
		char text[32];
		drgString::FtoA(prop->GetValueRed(), text);
		m_InputBox->SetText(text);
		drgString::FtoA(prop->GetValueGreen(), text);
		m_InputBox2->SetText(text);
		drgString::FtoA(prop->GetValueBlue(), text);
		m_InputBox3->SetText(text);
		drgString::FtoA(prop->GetValueAlpha(), text);
		m_InputBox4->SetText(text);
	}

	//
	// Draw the swath
	//

	drgBBox box(*m_SwathBox->GetView());
	m_SwathBox->WidgetOffsetRect(&box);

	// Take a little off the edges
	box.minv.x += 3.0f;
	box.minv.y += 3.0f;
	box.maxv.x -= 3.0f;
	box.maxv.y -= 3.0f;

	drgColor *color = prop->GetValue();
	draw->GetCommandBuffer()->DrawRect(&box, color);

	GUI_Container::Draw(draw); // The swath container just won't draw anything.
}

void GUI_ColorProperty::SetCallback(drgCallbackObj callback, void *data)
{
	m_InputBox->SetCallBack(callback, data);
	m_InputBox2->SetCallBack(callback, data);
	m_InputBox3->SetCallBack(callback, data);
	m_InputBox4->SetCallBack(callback, data);
}

void GUI_ColorProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_ColorProperty *prop = (GUI_ColorProperty *)data;
	GUI_Input *input = (GUI_Input *)object;

	if (prop && input && input->GetTypeCB() == GUI_CB_INPUT_FOCUS_LOST)
	{
		GUI_Input *inputs[4] = {prop->m_InputBox, prop->m_InputBox2, prop->m_InputBox3, prop->m_InputBox4};
		float colorValues[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		int numInputs = 4;

		for (int currentInput = 0; currentInput < numInputs; ++currentInput)
		{
			string16 *text = inputs[currentInput]->GetText();
			string8 valueText = *text;
			colorValues[currentInput] = drgString::AtoF(valueText.c_str());
			colorValues[currentInput] = DRG_CLAMP(colorValues[currentInput], 0.0f, 1.0f);

			//
			// Make sure we show the clamped value.
			//

			char clampedValue[32];
			drgString::FtoA(colorValues[currentInput], clampedValue);
			inputs[currentInput]->SetText(clampedValue);
		}

		drgColor color;
		color.r = (unsigned char)(colorValues[0] * 255.0f);
		color.g = (unsigned char)(colorValues[1] * 255.0f);
		color.b = (unsigned char)(colorValues[2] * 255.0f);
		color.a = (unsigned char)(colorValues[3] * 255.0f);

		drgColorProperty *colorProp = (drgColorProperty *)prop->GetDisplayProperty();
		colorProp->SetValue(color);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_ObjectReferenceProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////

GUI_ObjectReferenceProperty::GUI_ObjectReferenceProperty()
{
	m_WidgetType = GUI_TYPE_OBJECT_REFERENCE_PROPERTY;
}

GUI_ObjectReferenceProperty::~GUI_ObjectReferenceProperty()
{
}

void GUI_ObjectReferenceProperty::Init()
{
	m_Table = CreateChildTable(0);
	assert(m_Table);

	m_Table->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	//
	// Create our table rows and columns
	//

	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 5.0f, false);
	m_Table->AddRow(DRG_GUI_PROP_MIN_HEIGHT, 0.0f, 5.0f, false);
	m_Table->AddCol(DRG_GUI_ASSET_REF_LABEL_WIDTH, 0.0f, 0.0f, false);
	m_Table->AddCol(DRG_GUI_PROP_NAME_WIDTH, 0.0f, 0.0f, true);

	//
	// Create our object type label
	//

	GUI_Container *container = m_Table->CreateCell(0, 0);
	m_ObjectTypeLabel = container->CreateChildLabel(m_Property->GetPropertyName(), 0);
	m_ObjectTypeLabel->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	//
	// Create our object name label
	//

	container = m_Table->CreateCell(0, 1);
	m_ObjectNameLabel = container->CreateChildLabel("Object Name:", 0);
	m_ObjectNameLabel->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	//
	// Create the change asset reference button
	//

	container = m_Table->CreateCell(1, 0);
	m_ChangeObjectButton = container->CreateChildButton(string16("[Change Object]"), (unsigned int)DRG_GUI_ASSET_REF_BUTTON_WIDTH, (unsigned int)DRG_GUI_PROP_MIN_HEIGHT, NULL, NULL, DRG_NO_FLAGS);
	m_ChangeObjectButton->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	drgObjectReferenceProperty *prop = (drgObjectReferenceProperty *)m_Property;
	if (prop)
	{
		string16 name = prop->GetObjectName();
		m_ObjectNameLabel->SetText(&name);
	}
}

void GUI_ObjectReferenceProperty::SetChangeObjectCallBack(drgCallbackObj callback, void *data)
{
	if (m_ChangeObjectButton != NULL)
		m_ChangeObjectButton->SetCallBack(callback, data);
}

void GUI_ObjectReferenceProperty::Draw(GUI_Drawing *draw)
{
	drgObjectReferenceProperty *prop = (drgObjectReferenceProperty *)m_Property;
	if (prop && prop->GetValue())
	{
		m_ObjectNameLabel->SetText(prop->GetValue()->GetObjName8().c_str());
	}

	if (m_ChangeObjectButton != NULL)
	{
		if (m_ChangeObjectButton->HasCallBack())
			m_ChangeObjectButton->Show();
		else
			m_ChangeObjectButton->Hide();
	}

	GUI_Property::Draw(draw);
}

bool GUI_ObjectReferenceProperty::CallMouseClick(unsigned int winid, int button)
{
	if (IsDisabled() || IsHidden())
		return false;

	if (IsCursorInBounds() == false)
		return false;

	drgBBox buttonOffs(*(m_ChangeObjectButton->GetBox()));
	m_ChangeObjectButton->WidgetOffsetRect(&buttonOffs);
	if (!drgInputMouse::IsCursorInside(&buttonOffs))
		RunCallBack(GUI_CB_BUTTON_CLICK);

	return GUI_Container::CallMouseClick(winid, button);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// GUI_EnumProperty
/////////////////////////////////////////////////////////////////////////////////////////////////

GUI_EnumProperty::GUI_EnumProperty()
{
	m_WidgetType = GUI_TYPE_ENUM_PROPERTY;
	m_ValueNamesList = NULL;
}

GUI_EnumProperty::~GUI_EnumProperty()
{
}

void GUI_EnumProperty::Init()
{
	assert(m_Property);

	SetMinHeight(DRG_GUI_PROP_MIN_HEIGHT);

	string16 propText = m_Property->GetPropertyName();

	drgEnumProperty *enumProp = (drgEnumProperty *)m_Property;

	m_Table = CreateChildTable(DRG_WIDGET_RESIZE_EXPAND);
	m_Table->AddRow(0.0f, 1.0f, 0.0f, false);
	m_Table->AddCol(DRG_GUI_PROP_LABEL_WIDTH, 0.0f, 0.0f, false);
	m_Table->AddCol(0.0f, 1.0f, 0.0f, true);

	GUI_Container *cell0 = m_Table->CreateCell(0, 0, 1, 1);
	m_Label = cell0->CreateChildLabel(propText, DRG_NO_FLAGS);
	m_Label->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);

	GUI_Container *cell1 = m_Table->CreateCell(0, 1, 1, 1);
	m_ValueNamesList = cell1->CreateChildDropList(DRG_NO_FLAGS);
	if (m_ValueNamesList != NULL)
	{
		for (int i = 0; i < enumProp->GetNumberOfValues(); i++)
		{
			string16 enumValueName = enumProp->GetNameAtIndex(i);
			m_ValueNamesList->AddItem(enumValueName, DRG_NO_FLAGS);
		}
		m_ValueNamesList->SetSelected(enumProp->GetValue());
		m_ValueNamesList->AddFlagWidget(DRG_WIDGET_RESIZE_EXPAND);
		m_ValueNamesList->SetCallBack(OnInputChanged, this);
	}

	m_ContainerResizeType = DRG_CONTAINER_RESIZE_VERTICAL_ALIGN;

	InitCollapseButton();
}

void GUI_EnumProperty::Draw(GUI_Drawing *draw)
{
	assert(m_Property);
	drgEnumProperty *prop = (drgEnumProperty *)m_Property;
	m_Label->SetText(prop->GetPropertyName());

	if (m_ValueNamesList != NULL)
	{
		if (m_ValueNamesList->IsFocused() == false)
		{
			drgEnumProperty *enumProp = (drgEnumProperty *)m_Property;
			const char *nameOfVal = enumProp->GetNameOfCurrentValue();

			GUI_DropListItem *itr = m_ValueNamesList->GetFirstItem();
			while (itr)
			{
				string8 txt = *(itr->GetText());
				if (drgString::Compare(txt.c_str(), nameOfVal) == 0)
				{
					m_ValueNamesList->SetSelected(itr);
					break;
				}
				itr = m_ValueNamesList->GetNextItem(itr);
			}
		}
	}

	GUI_Container::Draw(draw);
}

void GUI_EnumProperty::OnInputChanged(drgObjectBase *object, void *data)
{
	GUI_EnumProperty *guiProp = (GUI_EnumProperty *)data;
	GUI_DropList *input = (GUI_DropList *)object;
	if (guiProp && input)
	{
		if (input->GetTypeCB() == GUI_CB_LIST_CHANGE)
		{
			assert(guiProp->m_Property);
			GUI_DropListItem *listItem = guiProp->m_ValueNamesList->GetSelected();
			string16 *text = listItem->GetText();
			string8 enumValText = *text;

			drgEnumProperty *enumProp = (drgEnumProperty *)guiProp->m_Property;
			enumProp->SetValueByName(enumValText.c_str());
		}
	}
}

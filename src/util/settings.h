#ifndef __DRG_SETTINGS_H__
#define __DRG_SETTINGS_H__

#include "system/global.h"

class drgSettings
{
public:
	~drgSettings();

	enum DRG_SETTINGS_TYPE
	{
		DRG_SETTINGS_ENGINE=0,
		DRG_SETTINGS_GAME,
		DRG_SETTINGS_PLATFORM,
		DRG_SETTINGS_DEBUG,
		DRG_SETTINGS_REMOTE,
		DRG_SETTINGS_TOTAL
	};

	static inline drgSettings* GetInstance(DRG_SETTINGS_TYPE type) {return &m_Instance[type];};

	void Init( const char* filename );
	void CleanUp();
	char* GetValueString( const char* key, char* defaultval=NULL );
	int GetValueInt(const char* key, int defaultval=0 );
	float GetValueFloat(const char* key, float defaultval=0.0f );
	bool GetValueBool(const char* key, bool defaultval=false );
	bool GetValueExists(const char* key );

	static void CleanUpAll();

private:
	struct drgSettingsNode
	{
		char				m_Name[DRG_MAX_PATH];
		char				m_Value[DRG_MAX_PATH];
		drgSettingsNode*	m_Next;
	};

	drgSettings();

	drgSettingsNode* GetNode( const char* key );

	drgSettingsNode*		m_Head;
	static drgSettings		m_Instance[DRG_SETTINGS_TOTAL];
};


#endif // __DRG_SETTINGS_H__



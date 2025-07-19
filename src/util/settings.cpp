#include "util/settings.h"
#include "util/stringn.h"
#include "core/file/asset_read.h"
#include "system/file.h"


drgSettings drgSettings::m_Instance[DRG_SETTINGS_TOTAL];

drgSettings::drgSettings()
{
	m_Head=NULL;
}

drgSettings::~drgSettings()
{
	CleanUp();
}

void drgSettings::Init( const char* filename )
{
	CleanUp();

	unsigned int endfile=0;
	unsigned int curcount=0;
	bool curget=false;
	drgAssetReader infile;
	if (!infile.OpenFile(filename, true))
		return;

	char* settingbuff = (char*)infile.GetFileMem();
	endfile = infile.GetFileSize();

	drgSettingsNode* curnode=NULL;
	drgSettingsNode* prevnode=NULL;
	char* cursetting = settingbuff;
	while(curcount<=endfile)
	{
		if( (settingbuff[curcount]=='\r') || (settingbuff[curcount]=='\n') || (settingbuff[curcount]=='=') )
		{
			settingbuff[curcount]='\0';
			if(drgString::Length(cursetting))
			{
				if(curget)
				{
					drgString::Copy(curnode->m_Value,cursetting);
				}
				else
				{
					prevnode=curnode;
					MEM_INFO_SET_NEW;
					curnode = new drgSettingsNode();
					curnode->m_Next=NULL;
					drgString::Copy(curnode->m_Name,cursetting);
					drgString::Copy(curnode->m_Value,"");
					if(prevnode)
						prevnode->m_Next=curnode;
					if(m_Head==NULL)
						m_Head=curnode;
				}
				curget=!curget;
			}
			cursetting=&(settingbuff[curcount+1]);
		}
		++curcount;
	}
}

void drgSettings::CleanUp()
{
	drgSettingsNode* curnode=m_Head;
	drgSettingsNode* delnode=NULL;
	while(curnode)
	{
		delnode=curnode;
		curnode=curnode->m_Next;
		delete delnode;
	}
	m_Head=NULL;
}

void drgSettings::CleanUpAll()
{
	for(int idx=0; idx<DRG_SETTINGS_TOTAL; idx++)
		m_Instance[idx].CleanUp();
}

bool drgSettings::GetValueExists( const char* key )
{
	drgSettingsNode* keynode=GetNode( key );
	return !!keynode;
}

char* drgSettings::GetValueString( const char* key, char* defaultval )
{
	drgSettingsNode* keynode=GetNode( key );
	if(keynode)
		return keynode->m_Value;
	return defaultval;
}

int drgSettings::GetValueInt( const char* key, int defaultval )
{
	drgSettingsNode* keynode=GetNode( key );
	if(keynode)
		return drgString::AtoI(keynode->m_Value);
	return defaultval;
}

float drgSettings::GetValueFloat( const char* key, float defaultval )
{
	drgSettingsNode* keynode=GetNode( key );
	if(keynode)
		return drgString::AtoF(keynode->m_Value);
	return defaultval;
}

bool drgSettings::GetValueBool( const char* key, bool defaultval )
{
	drgSettingsNode* keynode=GetNode( key );
	if(keynode)
	{
		if(!drgString::CompareNoCase("true",keynode->m_Value))
			return true;
		else if(!drgString::CompareNoCase("false",keynode->m_Value))
			return false;
		else
			return !!drgString::AtoI(keynode->m_Value);
	}
	return defaultval;
}

drgSettings::drgSettingsNode* drgSettings::GetNode( const char* key )
{
	drgSettingsNode* curnode=m_Head;
	while(curnode)
	{
		if(!drgString::CompareNoCase(key, curnode->m_Name))
			return curnode;
		curnode=curnode->m_Next;
	}
	return NULL;
}








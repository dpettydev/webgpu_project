#ifndef __DRG_ENGINE_H__
#define __DRG_ENGINE_H__

#include "util/stringv.h"
#include "util/stringn.h"
#include "system/global.h"
#include "core/object.h"
#include "core/message_queue.h"

class drgVec2;
class GUI_Window;
class GUI_Manager;
class drgMaterialManager;
class drgMessageRecord;
class drgTexture;

enum DRG_ENGINE_PLATFORM
{
	PLATFORM_PC = 0,
	PLATFORM_PS3 = 1,
	PLATFORM_360 = 2,
	PLATFORM_WII = 3,
	PLATFORM_PSP = 4,
	PLATFORM_LINUX = 5,
	PLATFORM_IPHONE = 6,
	PLATFORM_EMSCRIPT = 7,
	PLATFORM_JS = 8,
	PLATFORM_ANDROID = 9,
};

enum DRG_AD_TYPE
{
	AD_TYPE_NONE = 0,
	AD_TYPE_IMAGE,
	AD_TYPE_VIDEO_1,
	AD_TYPE_VIDEO_2,
	AD_TYPE_MAX
};

enum DRG_THIRD_PARTY_LOGIN_TYPE
{
	THIRD_PARTY_LOGIN_TYPE_FACEBOOK = 0,
	THIRD_PARTY_LOGIN_TYPE_GOOGLE_PLUS,
	THIRD_PARTY_LOGIN_TYPE_AMAZON,
	THIRD_PARTY_LOGIN_TYPE_MAX
};

enum DRG_MERCHANT_TYPE
{
	MERCHANT_TYPE_GOOGLE = 0,
	MERCHANT_TYPE_AMAZON,
	MERCHANT_TYPE_APPLE,
	MERCHANT_TYPE_MAX
};

class drgEngine
{

	class drgDeleteNode : public drgObjectBase
	{
		public:
			int m_NumFramesRemaining;
			drgObjectBase* m_ObjectToDelete;
			drgDeleteNode(drgObjectBase* object);
	};

public:

	class drgEngineMessageProcessor 
		: public drgMessageProcessor
	{
		virtual void ProcessMessage( drgMessageRecord* pMessage );
	};

	static void Init();
	static void CleanUp();
	static void Close();
	static void Update();
	static void Draw();
	static bool IsMultiWindow();
	static bool IsClosing();
	static void InitDrawing();	
	static void LoadingScreenStart();
	static void LoadingScreenEnd();


	static bool NativeWindowCreate(GUI_Window* win, string16* title, drgVec2* pos = NULL, unsigned int windowFlags = 0 );
	static bool NativeContextCreate(GUI_Window* win);
	static void NativeWindowCleaup(GUI_Window* win);
	static void NativeWindowBegin(GUI_Window* win);
	static void NativeWindowEnd(GUI_Window* win);
	static void* NativeContextSetMain();
	static void NativeWindowSetPosition( GUI_Window* win, float x, float y );
	static void NativeWindowActivateBorder( GUI_Window* win );
	static void NativeWindowDeactivateBorder( GUI_Window* win );
	static void NativeMouseCapture( GUI_Window* win );
	static drgVec2 NativeWindowGetPosition( GUI_Window* win );

	static drgInt64 GetClockCount();
	static void ProcessMessages();
	static void ReleaseResources();
	static void RecreateResources();
	static void DelayDeleteObject(drgObjectBase* object);
	static void	UpdateDelayedDeleteList();
	static void ReleaseDelayedDeleteList();

	static void OnMouseDown( unsigned int winGUID, int buttonID );
	static void OnMouseClick( unsigned int winGUID, int buttonID );
	static void OnMouseMove( unsigned int winGUID, drgVec2* localPos, drgVec2* globalPos );
	static void OnMouseWheel( unsigned int winGUID, float delta );
	static void OnPointerDown( unsigned int winGUID, unsigned int pointerIndex, float pressure = 0.0f );
	static void OnPointerClick( unsigned int winGUID, unsigned int pointerIndex );
	static void OnPointerMove( unsigned int winGUID, unsigned int pointerIndex, drgVec2* position, drgVec2* globalPosition );

	static bool SetClipboardText(string16* text);
	static int GetClipboardText(string16* text);

	static void GetScreenSize( int* width, int* height );
	static void EnableScreenTimeout(bool enable);
	static void GetABTestingProjects(char* projects);
	static void GetABTestingProjectValues(char* project, char* values, char* outvalues);
	static void TriggerInsightEvent(char* eventname);
	static void TriggerInsightEvent(char* eventname, char* attributes, char* metrics);
	static void SubmitQueuedInsightEvents();
	static void GetPushNotificationToken(char* token);

	static void GetDeviceID(char device_st[64]);
	static void OpenExternalUrl(char* url);
	static void OpenExternalEmail(char* addr, char* subject, char* text);
	static void OpenAppRating();
	static bool CanShowAd(DRG_AD_TYPE type);
	static bool ShowAd(DRG_AD_TYPE type);
	static long long ShowThirdPartyLogin(DRG_THIRD_PARTY_LOGIN_TYPE type);
	static bool GetThirdPartyLoginReady(DRG_THIRD_PARTY_LOGIN_TYPE type);
	static long long GetThirdPartyLoginId(DRG_THIRD_PARTY_LOGIN_TYPE type);
	static void GetMerchantSkuData(DRG_MERCHANT_TYPE type, char* skulist);
	static void PurchaseMerchantSku(DRG_MERCHANT_TYPE type, char* sku);

	static GUI_Manager*			GetGuiManager()				{ return m_GuiManager; }
	static drgMaterialManager*	GetMaterialManager()		{ return m_MaterialManager; }

	static inline void AddEngineMessage( drgMessageRecord* record )
	{
		if( m_MessageQueue )
		{
			m_MessageQueue->AddRecord( record );
		}
	}

	static inline DRG_ENGINE_PLATFORM GetPlatform()
	{
		return m_AppPlatform;
	}

	static inline const char* GetAppName()
	{
		return m_AppName;
	}

	static inline void SetAppName(const char* name)
	{
		drgString::Copy(m_AppName, name);
	}

	static inline double GetDeltaTime()
	{
		return m_fDeltaTime;
	}

	static inline unsigned int GetFrameNum()
	{
		return m_FrameNum;
	}

	static inline float GetFPS()
	{
		return m_fFPS;
	}

	static inline double GetDeltatTimeScale()
	{
		return m_fDeltaTimeScale;
	}

	static inline void SetDeltatTimeScale(double scale)
	{
		m_fDeltaTimeScale=scale;
	}

	static inline double GetTotalTime()
	{
		return ((m_fTotalTime-m_fStartTime)*100.0);
	}

	static inline void SetMaxFrameRate( double max )
	{
		m_MinFramePeriod=((unsigned int)((1.0f/max)*1000.0f));
	}

private:

	static bool							m_CloseApp;
	static DRG_ENGINE_PLATFORM			m_AppPlatform;
	static char							m_AppName[32];
	static unsigned int					m_FrameNum;
	static float						m_fFPS;
	static double						m_fDeltaTime;
	static double						m_fDeltaTimeScale;
	static double						m_fTotalTime;
	static double						m_fStartTime;
	static unsigned int					m_MinFramePeriod;
	static char							m_strFrameStats[256];
	static GUI_Manager*					m_GuiManager;
	static drgMaterialManager*			m_MaterialManager;
	static drgMessageQueue*				m_MessageQueue;
	static drgEngineMessageProcessor*	m_MessageProcessor;
	static drgObjectList*				m_DelayedDeleteList;

	drgEngine() {};
	~drgEngine() {};

	static void UpdateStats();
};

#endif // __DRG_ENGINE_H__




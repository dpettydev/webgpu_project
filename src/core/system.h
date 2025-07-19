#ifndef __DRG_SYSTEM_H__
#define __DRG_SYSTEM_H__

typedef void (*drgNativeAppInterfaceInit)();
typedef void (*drgNativeAppInterfaceCleanUp)();
typedef void (*drgNativeAppInterfaceUpdate)(double dtime);

class drgSystem
{
public:
	static int Init();
	static int CleanUp();
	static void Update();
	static void Draw();

private:
	drgSystem() {};
	~drgSystem() {};

	static int m_LoadState;
};

class AppInterface
{
public:
	//! Load/Unload
	static void Init();
	static void CleanUp();
	static void Load(const char *szFileName);
	static void Update(double dtime);
	static void ProcessMessage(int type, char *value);
};

class AppInterfaceNative
{
public:
	AppInterfaceNative(drgNativeAppInterfaceInit _init, drgNativeAppInterfaceCleanUp _cleanup, drgNativeAppInterfaceUpdate _update);

protected:
	static drgNativeAppInterfaceInit g_NativeAppInterfaceInit;
	static drgNativeAppInterfaceCleanUp g_NativeAppInterfaceCleanUp;
	static drgNativeAppInterfaceUpdate g_NativeAppInterfaceUpdate;

	friend class AppInterface;
};

#endif // __DRG_SYSTEM_H__

#ifndef __DRG_COMMAND_ARG_H__
#define __DRG_COMMAND_ARG_H__

#include "util/stringv.h"


class drgOption
{
public:
    string8		m_Option;
    string8		m_Argument;
    drgOption*	m_Next;
};


class drgCommandArg
{
public:
	static void InitList(int argc, char *const argv[]);
	static void FreeOptList();
	static unsigned int GetNumArgs();
	static const char* GetArg(unsigned int idx);
	static const char* GetArg(const char* option);
	static bool HasOption(const char* arg);
	static const char* GetOption(unsigned int idx);
	static const char* GetOption(const char* arg);

private:
	drgCommandArg() {};
	~drgCommandArg() {};

	static int MakeOpt(const char* option);

	static drgOption*	m_Head;
	static drgOption*	m_Tail;
};


#endif // __DRG_COMMAND_ARG_H__


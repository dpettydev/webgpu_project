
#include "core/cmdarg.h"
#include "core/memmgr.h"


drgOption* drgCommandArg::m_Head = NULL;
drgOption* drgCommandArg::m_Tail = NULL;

void drgCommandArg::InitList(int argc, char *const argv[])
{
	int nextArg;
	int argIndex;

	nextArg = 1;
	FreeOptList();

	while (nextArg < argc)
	{
		if('-' != argv[nextArg][0])
		{
			drgPrintError("Bad Option: %s\r\n", argv[nextArg]);
		}
		else
		{
			argIndex = 1;
			if((strlen(argv[nextArg]) > argIndex) && ('-' == argv[nextArg][0]))
			{
				argIndex += MakeOpt(&argv[nextArg][argIndex]);
				if(('=' == argv[nextArg][argIndex]))
				{
					argIndex++;
					m_Tail->m_Argument = &(argv[nextArg][argIndex]);
				}
			}
		}
		nextArg++;
	}
}

void drgCommandArg::FreeOptList()
{
    drgOption* del = NULL;
    while (m_Head != NULL)
    {
        del = m_Head->m_Next;
        free(m_Head);
        m_Head = del;
    }
	m_Head = NULL;
	m_Tail = NULL;
}

int drgCommandArg::MakeOpt(const char* option)
{
	int indx = 0;
    drgOption* opt = new drgOption();
	if (NULL == m_Head)
	{
		m_Head = opt;
		m_Tail = m_Head;
	}
	else
	{
		m_Tail->m_Next = opt;
		m_Tail = m_Tail->m_Next;
	}

	opt->m_Option = "";
    opt->m_Argument = "";
    opt->m_Next = NULL;
	while((option[indx]!='\0') && (option[indx]!='='))
	{
		opt->m_Option += option[indx];
		indx++;
	}

    return indx;
}

unsigned int drgCommandArg::GetNumArgs()
{
	unsigned int num = 0;
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		num++;
        itr = itr->m_Next;
    }
	return num;
}

const char* drgCommandArg::GetArg(unsigned int idx)
{
	unsigned int num = 0;
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		if(num==idx)
			return itr->m_Argument.c_str();
		num++;
        itr = itr->m_Next;
    }
	return NULL;
}

const char* drgCommandArg::GetArg(const char* option)
{
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		if(itr->m_Option == option)
			return itr->m_Argument.c_str();
        itr = itr->m_Next;
    }
	return NULL;
}

bool drgCommandArg::HasOption(const char* arg)
{
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		if(itr->m_Argument == arg)
			return true;
        itr = itr->m_Next;
    }
	return false;
}

const char* drgCommandArg::GetOption(unsigned int idx)
{
	unsigned int num = 0;
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		if(num==idx)
			return itr->m_Option.c_str();
		num++;
        itr = itr->m_Next;
    }
	return NULL;
}

const char* drgCommandArg::GetOption(const char* arg)
{
    drgOption* itr = m_Head;
    while (itr != NULL)
    {
		if(itr->m_Argument == arg)
			return itr->m_Option.c_str();
        itr = itr->m_Next;
    }
	return NULL;
}

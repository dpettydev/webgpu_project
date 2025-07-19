#ifndef __DRG_PROFILE_H__
#define __DRG_PROFILE_H__

#include "system/global.h"

#define DRG_PROFILE 0

#if DRG_PROFILE

#define DRG_PROFILE_MAXNMNODES		(4 * 1024)
#define DRG_PROFILE_MAXNMCHILDREN	(512)

class drgProfile
{
public:
	inline drgProfile(const char *file, unsigned int line, const char *name, unsigned int color)
	{
		Push(file,line,name,color);
	}
	inline ~drgProfile(void)
	{
		Pop();
	}

	static void Init();
	static void Frame();
	static void Reset(bool clean=true);
	static void DumpData(bool reset);
	static void DumpData(char* root, bool reset);
	static void Pop();
	static void Push(const char *file, unsigned int line, const char *name, unsigned int color);
	static void CleanUp();

private:

	struct drgProfileNode
	{
		char*			file;
		char			pfid[64];
		unsigned int	line;
		unsigned int	color;
		int				lastCounter0;
		int				lastCounter1;
		drgInt64		totalCounter0;
		drgInt64		totalCounter1;
		drgInt64		totalTime;
		drgInt64		lastReportedTime;
		drgProfileNode*	child[DRG_PROFILE_MAXNMCHILDREN];
		drgProfileNode*	parent;
		int				nmChildren;
		unsigned int	nmHits;
	};

	static drgProfileNode* FindNode(drgProfileNode *node, char* name);
	static void ResetInternal(drgProfileNode *node);
	static void PrintTree(drgProfileNode *tree, int level, drgInt64 parentTotal, drgInt64 overallTotal, drgInt64 count0Total, drgInt64 count1Total);

	static unsigned int		m_ProfileFrame;

	static drgProfileNode	m_Nodes[DRG_PROFILE_MAXNMNODES];
	static int				m_NmNodes;
	static int				m_Level;
	static drgInt64			m_LastTime;
	static drgProfileNode*	m_CurrentNode;
};

#define DRG_PROFILE_INIT()				drgProfile::Init()
#define DRG_PROFILE_FRAME()				drgProfile::Frame()
#define DRG_PROFILE_DUMPDATA(b)			drgProfile::DumpData(b)
#define DRG_PROFILE_DUMPDATAR(r, b)		drgProfile::DumpData(r, b)
#define DRG_PROFILE_POP()				drgProfile::Pop()
#define DRG_PROFILE_PUSH(x,c)			drgProfile::Push(__FILE__, __LINE__, #x, c)
#define DRG_PROFILE_PUSHST(s,c)			drgProfile::Push(__FILE__, __LINE__, s, c)
#define DRG_PROFILE_FUNK(x,c)			drgProfile tmpProfile(__FILE__, __LINE__, #x, c)
#define DRG_PROFILE_FUNKST(s,c)			drgProfile tmpProfile(__FILE__, __LINE__, s, c)

#else

#define DRG_PROFILE_INIT()
#define DRG_PROFILE_FRAME()
#define DRG_PROFILE_DUMPDATA(b)
#define DRG_PROFILE_DUMPDATAR(r, b)
#define DRG_PROFILE_POP()
#define DRG_PROFILE_PUSH(x,c)
#define DRG_PROFILE_PUSHST(s,c)
#define DRG_PROFILE_FUNK(x,c)
#define DRG_PROFILE_FUNKST(s,c)

#endif

#endif // __DRG_PROFILE_H__



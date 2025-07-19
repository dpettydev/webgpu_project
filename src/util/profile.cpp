
#include "util/profile.h"
#include "system/engine.h"

#if DRG_PROFILE

char g_ProfileFileNull[] = "none";
#if defined(_PS2)
	int g_PCControl;
#endif

unsigned int drgProfile::m_ProfileFrame=1;
drgProfile::drgProfileNode drgProfile::m_Nodes[DRG_PROFILE_MAXNMNODES];
int drgProfile::m_NmNodes=0;
int drgProfile::m_Level=0;
drgInt64 drgProfile::m_LastTime=0;
drgProfile::drgProfileNode* drgProfile::m_CurrentNode=NULL;

void drgProfile::Init()
{
	int i;
	m_Level=0;

	for (i=0;i<DRG_PROFILE_MAXNMNODES;i++)
    {
		m_Nodes[i].nmChildren=0;
		m_Nodes[i].lastCounter0=0;
		m_Nodes[i].lastCounter1=0;
		m_Nodes[i].totalCounter0=0;
		m_Nodes[i].totalCounter1=0;
		m_Nodes[i].totalTime=0;
		m_Nodes[i].lastReportedTime=0;
		m_Nodes[i].parent=NULL;
		m_Nodes[i].nmHits=0;
	}

	m_NmNodes=1;
	m_CurrentNode=m_Nodes;
	strcpy(m_CurrentNode->pfid, "root");
	m_CurrentNode->file = g_ProfileFileNull;
	m_CurrentNode->lastReportedTime=drgEngine::GetClockCount();
	m_CurrentNode->nmHits=m_ProfileFrame;
	m_LastTime=drgEngine::GetClockCount();

#if defined(_PS2)
	g_PCControl  = SCE_PC_CTE;

	g_PCControl |= SCE_PC0_ICACHE_MISS | (SCE_PC_U0|SCE_PC_S0|SCE_PC_K0|SCE_PC_EXL0);
	g_PCControl |= SCE_PC1_DCACHE_MISS | (SCE_PC_U1|SCE_PC_S1|SCE_PC_K1|SCE_PC_EXL1);
#endif
}

void drgProfile::Frame()
{
	m_ProfileFrame++;
#if defined(_PS2)
	scePcStart(g_PCControl, 0, 0);
#endif
}

void drgProfile::Reset(bool clean)
{
	ResetInternal(m_Nodes);
	if (clean)
		Init();
}

void drgProfile::ResetInternal(drgProfileNode *node)
{
	int i;
	node->totalTime=0;
	node->nmHits=0;
	for (i=0;i<node->nmChildren;i++)
		ResetInternal(node->child[i]);
}

drgProfile::drgProfileNode* drgProfile::FindNode(drgProfileNode *node, char* name)
{
	if (!strcmp(node->pfid, name))
		return node;

	int i;
	drgProfileNode* retnode = NULL;
	for (i=0;i<node->nmChildren;i++)
	{
		retnode = FindNode(node->child[i], name);
		if(retnode != NULL)
			return retnode;
	}
	return NULL;
}

void drgProfile::DumpData(bool reset)
{
	drgInt64 sum;
	drgPrintOut("\n\n");
	sum=(drgEngine::GetClockCount()-m_Nodes[0].lastReportedTime);
	m_Nodes[0].totalTime=sum;
	int tempHits=m_Nodes[0].nmHits;
	m_Nodes[0].nmHits=m_ProfileFrame-m_Nodes[0].nmHits;
	PrintTree(m_Nodes,0,sum,sum,m_Nodes[0].totalCounter0,m_Nodes[0].totalCounter1);
	m_Nodes[0].nmHits=tempHits;
	if(reset)
	{
		Reset();
		m_Nodes[0].lastReportedTime=drgEngine::GetClockCount();
		m_Nodes[0].nmHits=m_ProfileFrame;
	}
}


void drgProfile::DumpData(char* root, bool reset)
{
	if(root==NULL || root[0]=='\0')
	{
		DumpData(reset);
		return;
	}

	drgProfileNode* DumpNode = FindNode(m_Nodes, root);
	if(DumpNode==NULL)
	{
		drgPrintOut("PROFILE: Failed to find root node %s!!\n", root);
		return;
	}

	drgInt64 sum;
	drgPrintOut("\n\n");
	sum=(drgEngine::GetClockCount()-m_Nodes[0].lastReportedTime);
	m_Nodes[0].totalTime=sum;
	int tempHits=m_Nodes[0].nmHits;
	m_Nodes[0].nmHits=m_ProfileFrame-m_Nodes[0].nmHits;
	PrintTree(DumpNode,0,sum,sum,DumpNode->totalCounter0,DumpNode->totalCounter1);
	m_Nodes[0].nmHits=tempHits;
	if(reset)
	{
		Reset();
		m_Nodes[0].lastReportedTime=drgEngine::GetClockCount();
		m_Nodes[0].nmHits=m_ProfileFrame;
	}
}

void drgProfile::Push(const char *file, unsigned int line, const char *name, unsigned int color)
{
	m_LastTime=drgEngine::GetClockCount();

	int i;
	for (i=0;i<m_CurrentNode->nmChildren;i++)
		if (!strcmp(m_CurrentNode->child[i]->pfid, name))
			break;

	if (i==m_CurrentNode->nmChildren)
    {
		assert(m_NmNodes<DRG_PROFILE_MAXNMNODES);
		assert(m_CurrentNode->nmChildren<DRG_PROFILE_MAXNMCHILDREN);
		m_CurrentNode->child[m_CurrentNode->nmChildren++]=m_Nodes+m_NmNodes;
		m_Nodes[m_NmNodes].parent=m_CurrentNode;
		strcpy(m_Nodes[m_NmNodes].pfid, name);
		m_Nodes[m_NmNodes].pfid[63] = '\0';
		m_Nodes[m_NmNodes].file=(char*)file;
		m_Nodes[m_NmNodes].line=line;
		m_Nodes[m_NmNodes].color=color;
		m_CurrentNode=m_Nodes+m_NmNodes;
		m_NmNodes++;
    }
	else
	{
		m_CurrentNode=m_CurrentNode->child[i];
	}

#if defined(_PS2)
	m_CurrentNode->lastCounter0=scePcGetCounter0();
	m_CurrentNode->lastCounter1=scePcGetCounter1();
#endif

	m_CurrentNode->lastReportedTime=m_LastTime;
	m_CurrentNode->nmHits++;
}

void drgProfile::Pop()
{
	drgInt64 time=drgEngine::GetClockCount();
	m_CurrentNode->totalTime+=(time-m_CurrentNode->lastReportedTime);

#if defined(_PS2)
	int counter = scePcGetCounter0();
	if((m_CurrentNode->lastCounter0>=0) && (counter>m_CurrentNode->lastCounter0))
		m_CurrentNode->totalCounter0+=(counter-m_CurrentNode->lastCounter0);
	counter = scePcGetCounter1();
	if((m_CurrentNode->lastCounter1>=0) && (counter>m_CurrentNode->lastCounter1))
		m_CurrentNode->totalCounter1+=(counter-m_CurrentNode->lastCounter1);
#endif

	m_CurrentNode=m_CurrentNode->parent;
}

void drgProfile::PrintTree(drgProfileNode *tree, int level, drgInt64 parentTotal, drgInt64 overallTotal, drgInt64 count0Total, drgInt64 count1Total)
{
	int i;
	drgInt64 totPercent;
	drgInt64 allPercent;
	drgInt64 sum;
	int		 hits;
	double   time;
	char     spaces[32] = "";
	for (i=0;i<level;i++)
		spaces[i] = ' ';
	spaces[i] = '\0';
	sum=tree->totalTime;
	time=(double)sum/(double)m_Nodes[0].nmHits;
	if (parentTotal<1)
		totPercent=0;
	else
		totPercent=(10000*sum)/parentTotal;

	if (overallTotal<1)
		allPercent=0;
	else
		allPercent=(10000*sum)/overallTotal;

	hits=tree->nmHits;

	//drgPrintOut("(%.2f, %.2f) %.4f - %i : %s F:%s L:%i\n",allPercent*.01f,totPercent*.01f,time,hits,tree->pfid,tree->file,tree->line);
	//drgPrintOut("(%.2f, %.2f) %.4f - %i : %s\n",totPercent*.01f,allPercent*.01f,time,hits,tree->pfid);
#if defined(_PS2)
	drgInt64 count0Percent;
	drgInt64 count1Percent;
	if (count0Total<1)
		count0Percent=0;
	else
		count0Percent=(10000*tree->totalCounter0)/count0Total;
	if (count1Total<1)
		count1Percent=0;
	else
		count1Percent=(10000*tree->totalCounter1)/count1Total;

	drgPrintOut("%s(%*.2f, %*.2f) : %*i - %s (I)%.2f (D)%.2f\n", spaces, 5, totPercent*.01f, 5, allPercent*.01f, 7, hits, tree->pfid, count0Percent*.01f, count1Percent*.01f);
#else
	drgPrintOut("%s(%*.2f, %*.2f) : %*i - %s [%s(%i)]\n", spaces, 5, totPercent*.01f, 5, allPercent*.01f, 7, hits, tree->pfid, tree->file, tree->line);
#endif

	for (i=0;i<tree->nmChildren;i++)
		PrintTree(tree->child[i],level+1,sum,overallTotal,count0Total,count1Total);
		//PrintTree(tree->child[i],level+1,sum,overallTotal,tree->totalCounter0,tree->totalCounter1);

	tree->lastReportedTime=sum;
}

#endif












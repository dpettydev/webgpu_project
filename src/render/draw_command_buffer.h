#ifndef __DRG_DRAW_COMMAND_BUFFER__
#define __DRG_DRAW_COMMAND_BUFFER__

#include "system/global.h"
#include "render/command_nodes.h"

#define DRG_NUM_DRAW_BUFFERS (2)
#if (defined(_ANDROID) || defined(_IPHONE))
#define DRG_MAX_VERT_SIZE (512 * 1024)
#define DRG_MAX_LINE_SIZE (512)
#else
#define DRG_MAX_VERT_SIZE (1024 * 1024)
#define DRG_MAX_LINE_SIZE (512)
// #define DRG_MAX_VERT_SIZE				(2 * 1024 * 1024)
// #define DRG_MAX_LINE_SIZE				(2 * 1024)
#endif
#define DRG_MAX_COMMAND_SIZE (128 * 1024)
#define DRG_MAX_COMMANDS (64 * 1024)

class drgRenderContext;
class drgModel;
class drgRenderTarget;
class drgRenderCache;

enum DRG_DRAW_COMMAND_BUFFER_TYPE
{
	DRG_COMMAND_BUFFER_DEFAULT,
	DRG_COMMAND_BUFFER_DYNAMIC,
};
typedef DRG_DRAW_COMMAND_BUFFER_TYPE DRG_DRAW_COMMAND_BUFFER_TYPE;

class drgDrawCommandBuffer
{
public:
	drgDrawCommandBuffer(drgRenderContext *context, DRG_DRAW_COMMAND_BUFFER_TYPE type = DRG_COMMAND_BUFFER_DEFAULT, drgRenderCache *cache = NULL);
	virtual ~drgDrawCommandBuffer();

	virtual void Init();
	virtual void Destroy();

	virtual void Begin();
	virtual void End();

	virtual void Clear();

	virtual void Draw(bool flush);

	inline drgRenderContext *GetDrawContext()
	{
		return m_Context;
	}

	inline drgRenderCache *GetRenderCache()
	{
		return m_Cache;
	}

	inline DRG_DRAW_COMMAND_BUFFER_TYPE GetCommandBufferType()
	{
		return m_CommandBufferType;
	}

	inline int GetListID()
	{
		return m_ListID;
	}

protected: // Members
	unsigned char *m_DataBuffer[DRG_NUM_DRAW_BUFFERS];
	int m_NumCommands[DRG_NUM_DRAW_BUFFERS];

	int m_CurrentDataOffset;
	drgRenderContext *m_Context;
	drgRenderCache *m_Cache;

	int m_CurrentDrawBuffer;
	DRG_DRAW_COMMAND_BUFFER_TYPE m_CommandBufferType;
	int m_ListID;

protected: // Methods
	virtual void AddNode(DRG_DRAW_COMMAND_TYPE type, void *data, int dataSize);
	virtual void AddUniformNode(DRG_DRAW_COMMAND_TYPE command, DRG_DRAW_COMMAND_UNIFORM_TYPE uniformType, void *data, int dataSize);

	virtual void ProcessCommands();
	virtual void DispatchCommand(DRG_DRAW_COMMAND_TYPE command, int &currentOffset);

	virtual void ProcessClear(int &currentOffset);
	virtual void ProcessSwapBuffers(int &currentOffset);
	virtual void ProcessDrawMesh(int &currentOffset);
	virtual void ProcessDrawModel(int &currentOffset);
	virtual void ProcessDrawScene(int &currentOffset);
	virtual void ProcessEnableScissor(int &currentOffset);
	virtual void ProcessScissorState(int &currentOffset);
	virtual void ProcessClipRect(int &currentOffset);
	virtual void ProcessViewport(int &currentOffset);
	virtual void ProcessColorBlend(int &currentOffset);
	virtual void ProcessAlphaBlend(int &currentOffset);
	virtual void ProcessEnableColorBlend(int &currentOffset);
	virtual void ProcessColorSourceBlend(int &currentOffset);
	virtual void ProcessColorDestBlend(int &currentOffset);
	virtual void ProcessColorOperation(int &currentOffset);
	virtual void ProcessEnableAlphaBlend(int &currentOffset);
	virtual void ProcessAlphaSourceBlend(int &currentOffset);
	virtual void ProcessAlphaDestBlend(int &currentOffset);
	virtual void ProcessAlphaOperation(int &currentOffset);
	virtual void ProcessDepthState(int &currentOffset);
	virtual void ProcessStencilState(int &currentOffset);
	virtual void ProcessEnableDepthTest(int &currentOffset);
	virtual void ProcessEnableDepthWrite(int &currentOffset);
	virtual void ProcessDepthCompareFunc(int &currentOffset);
	virtual void ProcessEnableStencilTest(int &currentOffset);
	virtual void ProcessEnableStencilWrite(int &currentOffset);
	virtual void ProcessStencilCompareFunc(int &currentOffset);
	virtual void ProcessStencilRef(int &currentOffset);
	virtual void ProcessStencilMask(int &currentOffset);
	virtual void ProcessFrontFace(int &currentOffset);
	virtual void ProcessCullMode(int &currentOffset);
	virtual void ProcessFillMode(int &currentOffset);
	virtual void ProcessMakeCurrent(int &currentOffset);
	virtual void ProcessBindRenderTarget(int &currentOffset);
	virtual void ProcessUnbindRenderTarget(int &currentOffset);
	virtual void ProcessUniformCommand(DRG_DRAW_COMMAND_UNIFORM_TYPE command, int &currentOffset);
	virtual void ProcessUpdateSurfaces(int &currentOffset);
	virtual void ProcessRunCommandList(int &currentOffset);

	friend class drgDrawCommandBufferManager;
};

#endif
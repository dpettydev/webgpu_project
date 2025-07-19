#ifndef _GUI_FONT_H_
#define _GUI_FONT_H_

#include "util/color.h"
#include "util/stringv.h"
#include "render/texture.h"
#include "core/object.h"

#define MAX_UNI_CHARS		0xFFFFU
#define MAX_ASCII_CHARS		0xFFU
#define FONT_IS_INITED		(1<<9)
#define MAX_LINE_BREAKS		(32)
#define DRG_FONT_MAX		(32)

class drgDynamicDrawCommandBuffer;

//!Class that handles the loading and drawing of fonts
class drgFont
{
public:
	drgFont();
	~drgFont();
	void CopyFont(drgFont *font);

  	bool Init( const char* filename, const char* fontname );
	void CleanUp();
	int GetTextureCount();
	static void InitDefaultFont();
	static void CleanUpFontHeaders();

	int Print( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, char *string, float scale=1.0f, int count=100000 );
	void Printf( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, float scale, char *fmt,... );
	int StringSize( char *string, float scale=1.0f, int count=100000 );
	int PrintCentered( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, char *string, float scale=1.0f, int count=10000 );
	int PrintWrap( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, char *string, int width, float scale=1.0f, int alignment=0, int clipLow=-100, int clipHigh=580, int lineWidth=21);
    int PrintWrapAlignment(drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, char *string, int width, int height, float scale=1.0f, int clipLow=-100, int clipHigh=580, int lineWidth=21, int alignmentV=0, int alignmentH=0);
	bool CalculateDimensions(char *string, int width, int height);
	int GetNumLines(char *string, int width, int height);
	int GetTextWidth(char *string, int width, int height);
    int GetTextHeight(char *string, int width, int height);
 
	// UNICODE
	int StringSize( const unsigned short *string, float scale=1.0f, int count=100000 );
	int Print( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, const unsigned short *string,  float scale=1.0f, int count=100000 );
	int PrintCentered( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, const unsigned short *string,  float scale=1.0f, int count=100000);
	int PrintRight( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, const unsigned short *string,  float scale=1.0f, int count=100000);
	int PrintWrap( drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, unsigned short *string, int width,  float scale=1.0f, int alignment=0, int clipLow=-100, int clipHigh=580, int lineWidth=21);
    int PrintWrapAlignment(drgDynamicDrawCommandBuffer* draw, drgColor color, int x, int y, unsigned short *string, int width, int height, float scale=1.0f, int clipLow=-100, int clipHigh=580, int lineWidth=21, int alignmentV=0, int alignmentH=0);
	bool CalculateDimensions(unsigned short *string, int width, int height);
	int GetNumLines(unsigned short *string, int width, int height);
    int GetTextWidth(unsigned short *string, int width, int height);
    int GetTextHeight(unsigned short *string, int width, int height);
	int GetNumMissing(unsigned short *string);
    
	inline int GetFontSize()
	{
		return m_FontSize;
	}

	inline char* GetFontName()
	{
		return m_FontHeaderRec->fontname;
	}

	//!Function that returns the default font.
	//!\returns the default font
	static inline drgFont* GetDefaultFont()
	{
		return &m_DefaultFont;
	};

	struct _chars
	{
		unsigned short unicode;
		short u1,u2,v1,v2,firstKern;
		short advance;
		short yoff;
		short xoff;
		unsigned char texindex;
	};

	struct _kern
	{
		unsigned short code1,code2;
		short spacing;
	};

	//!Structure containing font header information
	struct FontHeader
	{
		//!Number of characters contained in the font
		short nmChar;
		//!Width of a space for the font
		short spaceWidth;
		short m_Flags;
		unsigned short m_MaxChars;
		int nmKerning;
		char pad[4];
		//!Structure containing information about font characters
		drgUInt64 charptr;  // pointer to _chars
		//!Structure containing information about font kerning
		drgUInt64 kernptr;
		drgUInt64 textureptr[10];
		//!ascii map for the font
		short asciiMap[MAX_UNI_CHARS];
	};

	struct FontHeaderRec
	{
		char*		header;
		char		fontname[256];
		int			ref_count;
	};

	inline FontHeader*		GetFontHeader(){ return m_FontHeader;};
	inline FontHeaderRec*	GetFontHeaderRec(){ return m_FontHeaderRec;};

private:
	void LoadRaw(char* rawfile, int total);
	int LoadInternal(int index, unsigned int headersize, const char* fontname);

	FontHeader*		        m_FontHeader;
	FontHeaderRec*			m_FontHeaderRec;

	//!Name of the font
	int						m_FontSize;
	int						m_SpaceWidth;							// width of the space character in the current font
	static drgFont	        m_DefaultFont;
	static int		        m_FontHeaderCount;
	static FontHeaderRec	m_FontHeaderList[DRG_FONT_MAX];

	inline bool				isWhitespace(char checkChar){ return ((checkChar == ' ') || (checkChar == '\t') || (checkChar == '\r'));};
	inline bool				isNewline(char checkChar){ return ((checkChar == '\n'));};
	inline bool				isWhitespace(unsigned short checkChar){ return ((checkChar == ' ') || (checkChar == '\t') || (checkChar == '\r'));};
	inline bool				isNewline(unsigned short checkChar){ return ((checkChar == '\n'));};
	bool					canUseAsLineEnd(unsigned short checkChar);
	bool					canUseAsLineBegin(unsigned short checkChar);
};


class drgFontObj : public drgObjectProp
{
public:
	drgFontObj();
	virtual ~drgFontObj();
	virtual void Reinit();
	virtual void Destroy();
	void Init(const char* filename);
	const char* GetFileName();
	drgFont* GetFont();
	void SetFont( drgFont* font )	{ m_Font = font; }

private:
	drgFont*	m_Font;
	string8		m_FileName;
};

#endif // _GUI_FONT_H_



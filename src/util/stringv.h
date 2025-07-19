#ifndef __DRG_NEW_STRING_H__
#define __DRG_NEW_STRING_H__

#include "core/memmgr.h"

#define STR_ERR					(-1)
#define STR_OK					(0)
#define STR_BUFF_LENGTH_GET		(0)

#define STATICBLKPARMS(q) ((void *)("" q "")), ((int) sizeof(q)-1)

extern void InitVStringBlocks(unsigned int max_blocks);

class string16;

class string8
{
public:
    string8();
	string8(char c);
	string8(unsigned char c);
	string8(const char *s);
	string8(int len, const char *s);
	string8(int len, const short *s);
	string8(const string8& b);
	string8(const string16& b);
	string8(char c, int len);
	string8(const void * blk, int len);
	~string8();

	// = operator
	const string8& operator = (char c);
	const string8& operator = (unsigned char c);
	const string8& operator = (const char *s);
	const string8& operator = (const string8& b);
	const string8& operator = (const string16& b);

	// += operator
	const string8& operator += (char c);
	const string8& operator += (unsigned char c);
	const string8& operator += (const char *s);
	const string8& operator += (const string8& b);

	// *= operator
	inline const string8& operator *= (int count)
	{
		this->repeat (count);
		return *this;
	}

	// + operator
	const string8 operator + (char c) const;
	const string8 operator + (unsigned char c) const;
	const string8 operator + (const unsigned char *s) const;
	const string8 operator + (const char *s) const;
	const string8 operator + (const string8& b) const;

	// * operator
	inline const string8 operator * (int count) const
	{
		string8 retval (*this);
		retval.repeat (count);
		return retval;
	}

	// Comparison operators
	bool operator == (const string8& b) const;
	bool operator == (const char * s) const;
	bool operator == (const unsigned char * s) const;
	bool operator != (const string8& b) const;
	bool operator != (const char * s) const;
	bool operator != (const unsigned char * s) const;
	bool operator <  (const string8& b) const;
	bool operator <  (const char * s) const;
	bool operator <  (const unsigned char * s) const;
	bool operator <= (const string8& b) const;
	bool operator <= (const char * s) const;
	bool operator <= (const unsigned char * s) const;
	bool operator >  (const string8& b) const;
	bool operator >  (const char * s) const;
	bool operator >  (const unsigned char * s) const;
	bool operator >= (const string8& b) const;
	bool operator >= (const char * s) const;
	bool operator >= (const unsigned char * s) const;

	string8& operator << (const char *val);
	string8& operator << (char val);
	string8& operator << (unsigned char val);
	string8& operator << (int val);
	string8& operator << (unsigned int val);
	string8& operator << (float val);
	string8& operator << (double val);
	string8& operator << (string8 val);

	string8& operator >> (int& val);
	string8& operator >> (unsigned int& val);
	string8& operator >> (float& val);
	string8& operator >> (double& val);
	string8& operator >> (string8& val);

	// Casts
	operator double () const;
	operator float () const;
	operator int () const;
	operator unsigned int () const;

	// Accessors
	inline const char* c_str() const
	{
		return (const char *)data;
	}

	inline unsigned int length() const
	{
		return slen;
	}

	inline bool empty() const
	{
		return (slen==0);
	}

	inline unsigned char character (int i) const
	{
		if (((unsigned) i) >= (unsigned) slen)
		{
			drgPrintError("character idx out of bounds\r\n");
			return '\0';
		}
		return data[i];
	}

	inline void set (int i, char val) const
	{
		if (((unsigned) i) < (unsigned) slen)
			data[i] = val;
		drgPrintError("character idx out of bounds\r\n");
	}

	inline unsigned char operator [] (int i) const
	{
		return character(i);
	}

	// Space allocation hint method.
	void alloc(int length);
	void clear();

	// Search methods.
	int caselessEqual(const string8& b) const;
	int caselessCmp(const string8& b) const;
	int find(const string8& b, int pos = 0) const;
	int find(const char * b, int pos = 0) const;
	int caselessfind(const string8& b, int pos = 0) const;
	int caselessfind(const char * b, int pos = 0) const;
	int find(char c, int pos = 0) const;
	int reversefind(const string8& b, int pos) const;
	int reversefind(const char * b, int pos) const;
	int caselessreversefind(const string8& b, int pos) const;
	int caselessreversefind(const char * b, int pos) const;
	int reversefind(char c, int pos) const;
	int findchr(const string8& b, int pos = 0) const;
	int findchr(const char * s, int pos = 0) const;
	int reversefindchr(const string8& b, int pos) const;
	int reversefindchr(const char * s, int pos) const;
	int nfindchr(const string8& b, int pos = 0) const;
	int nfindchr(const char * b, int pos = 0) const;
	int nreversefindchr(const string8& b, int pos) const;
	int nreversefindchr(const char * b, int pos) const;
	int find_first_of(const char * b, int pos = 0) const;
	int find_last_of(const char * b, int pos = 0) const;
	int find_first_not_of(const char * b, int pos = 0) const;
	int find_last_not_of(const char * b, int pos = 0) const;

	// Search and substitute methods.
	void findreplace(const string8& find, const string8& repl, int pos = 0);
	void findreplace(const string8& find, const char * repl, int pos = 0);
	void findreplace(const char * find, const string8& repl, int pos = 0);
	void findreplace(const char * find, const char * repl, int pos = 0);
	void findreplacecaseless(const string8& find, const string8& repl, int pos = 0);
	void findreplacecaseless(const string8& find, const char * repl, int pos = 0);
	void findreplacecaseless(const char * find, const string8& repl, int pos = 0);
	void findreplacecaseless(const char * find, const char * repl, int pos = 0);

	// Extraction method.
	const string8 substr(int pos = 0, int len = npos) const;

	// Standard manipulation methods.
	void setsubstr(int pos, const string8& b, unsigned char fill = ' ');
	void setsubstr(int pos, const char * b, unsigned char fill = ' ');
	void insert(int pos, const string8& b, unsigned char fill = ' ');
	void insert(int pos, const char * b, unsigned char fill = ' ');
	void insertchrs(int pos, int len, unsigned char fill = ' ');
	void replace(int pos, int len, const string8& b, unsigned char fill = ' ');
	void replace(int pos, int len, const char * s, unsigned char fill = ' ');
	void remove(int pos = 0, int len = npos);
	void trunc(int len);

	// Miscellaneous methods.
	void format(const char * fmt, ...);
	void formata(const char * fmt, ...);
	void fill(int length, unsigned char fill = ' ');
	void repeat(int count);
	void caseupper();
	void caselower();
	void ltrim (const char* trim=" \t\v\f\r\n");
	void rtrim (const char* trim=" \t\v\f\r\n");
	inline void trim(const char* trim=" \t\v\f\r\n")
	{
		rtrim(trim);
		ltrim(trim);
	}

	int get();
	const string8 readline(const char* terminator = "\r\n");

	int copy(const string8& b);
	int concat(const string8& b);
	int catblk(const void * s, int len);
	int catcstr(const char * s);
	int conchar(char c);
	int iseq(const string8& b) const;
	int iseqcstr(const char * s) const;
	int compare(const string8& b) const;
	int pattern(int len);
	int setstr (int pos, unsigned char fill);
	int setstr (int pos, const string8& b, unsigned char fill, bool truncate = false);

	static const int npos = 0x7FFF;

private:
	static int snapupsize(int i);

	inline bool isoneof(char ch, const char* check_st) {
		unsigned int pos = 0;
		while (check_st[pos] != '\0') {
			if (ch == check_st[pos]) {
				return true;
			}
			pos++;
		}
		return false;
	}

	unsigned char * data;
	int slen;
	int mlen;
};



class string16
{
public:
    string16();
	string16(short c);
	string16(unsigned short c);
	string16(const char *s);
	string16(int len, const char *s);
	string16(const string16& b);
	string16(char c, int len);
	string16(const void * blk, int len);
	~string16();

	// = operator
	const string16& operator = (char c);
	const string16& operator = (unsigned char c);
	const string16& operator = (const char *s);
	const string16& operator = (const short *s);
	const string16& operator = (const string8& c);
	const string16& operator = (const string16& b);

	// += operator
	const string16& operator += (char c);
	const string16& operator += (unsigned char c);
	const string16& operator += (short c);
	const string16& operator += (unsigned short c);
	const string16& operator += (const char *s);
	const string16& operator += (const short *s);
	const string16& operator += (const string16& b);

	// *= operator
	inline const string16& operator *= (int count)
	{
		this->repeat (count);
		return *this;
	}

	// + operator
	const string16 operator + (char c) const;
	const string16 operator + (unsigned char c) const;
	const string16 operator + (const unsigned char *s) const;
	const string16 operator + (const char *s) const;
	const string16 operator + (const unsigned short *s) const;
	const string16 operator + (const short *s) const;
	const string16 operator + (const string16& b) const;

	// * operator
	inline const string16 operator * (int count) const
	{
		string16 retval (*this);
		retval.repeat (count);
		return retval;
	}

	// Comparison operators
	bool operator == (const string16& b) const;
	bool operator == (const char * s) const;
	bool operator == (const unsigned char * s) const;
	bool operator != (const string16& b) const;
	bool operator != (const char * s) const;
	bool operator != (const unsigned char * s) const;
	bool operator <  (const string16& b) const;
	bool operator <  (const char * s) const;
	bool operator <  (const unsigned char * s) const;
	bool operator <= (const string16& b) const;
	bool operator <= (const char * s) const;
	bool operator <= (const unsigned char * s) const;
	bool operator >  (const string16& b) const;
	bool operator >  (const char * s) const;
	bool operator >  (const unsigned char * s) const;
	bool operator >= (const string16& b) const;
	bool operator >= (const char * s) const;
	bool operator >= (const unsigned char * s) const;

	string16& operator << (const char *val);
	string16& operator << (char val);
	string16& operator << (unsigned char val);
	string16& operator << (int val);
	string16& operator << (unsigned int val);
	string16& operator << (float val);
	string16& operator << (double val);
	string16& operator << (string16 val);

	string16& operator >> (int& val);
	string16& operator >> (unsigned int& val);
	string16& operator >> (float& val);
	string16& operator >> (double& val);
	string16& operator >> (string16& val);

	// Casts
	operator double () const;
	operator float () const;
	operator int () const;
	operator unsigned int () const;

	inline const unsigned short* str() const
	{
		return (unsigned short*)data;
	}

	inline unsigned int length() const
	{
		return slen;
	}

	inline bool empty() const
	{
		return (slen==0);
	}

	inline unsigned short character (int i) const
	{
		if (((unsigned) i) >= (unsigned) slen)
		{
			drgPrintError("character idx out of bounds\r\n");
			return '\0';
		}
		return data[i];
	}

	inline void set (int i, unsigned short val) const
	{
		if (((unsigned) i) >= (unsigned) slen)
		{
			drgPrintError("character idx out of bounds\r\n");
			return;
		}
		data[i] = val;
	}

	inline unsigned short operator [] (int i) const
	{
		return character(i);
	}

	// Space allocation hint method.
	void alloc(int length);
	void clear();

	// Search methods.
	int caselessEqual(const string16& b) const;
	int caselessCmp(const string16& b) const;
	int find(const string16& b, int pos = 0) const;
	int find(const char * b, int pos = 0) const;
	int caselessfind(const string16& b, int pos = 0) const;
	int caselessfind(const char * b, int pos = 0) const;
	int find(char c, int pos = 0) const;
	int reversefind(const string16& b, int pos) const;
	int reversefind(const char * b, int pos) const;
	int caselessreversefind(const string16& b, int pos) const;
	int caselessreversefind(const char * b, int pos) const;
	int reversefind(char c, int pos) const;
	int findchr(const string16& b, int pos = 0) const;
	int findchr(const char * s, int pos = 0) const;
	int reversefindchr(const string16& b, int pos) const;
	int reversefindchr(const char * s, int pos) const;
	int nfindchr(const string16& b, int pos = 0) const;
	int nfindchr(const char * b, int pos = 0) const;
	int nreversefindchr(const string16& b, int pos) const;
	int nreversefindchr(const char * b, int pos) const;
	int find_first_of(const char * b, int pos = 0) const;
	int find_last_of(const char * b, int pos = 0) const;
	int find_first_not_of(const char * b, int pos = 0) const;
	int find_last_not_of(const char * b, int pos = 0) const;

	// Search and substitute methods.
	void findreplace(const string16& find, const string16& repl, int pos = 0);
	void findreplace(const string16& find, const char * repl, int pos = 0);
	void findreplace(const char * find, const string16& repl, int pos = 0);
	void findreplace(const char * find, const char * repl, int pos = 0);
	void findreplacecaseless(const string16& find, const string16& repl, int pos = 0);
	void findreplacecaseless(const string16& find, const char * repl, int pos = 0);
	void findreplacecaseless(const char * find, const string16& repl, int pos = 0);
	void findreplacecaseless(const char * find, const char * repl, int pos = 0);

	// Extraction method.
	const string16 substr(int pos = 0, int len = npos) const;

	// Standard manipulation methods.
	void setsubstr(int pos, const string16& b, unsigned short fill = ' ');
	void setsubstr(int pos, const char * b, unsigned short fill = ' ');
	void insert(int pos, const string16& b, unsigned short fill = ' ');
	void insert(int pos, short c, unsigned short fill = ' ');
	void insertchrs(int pos, int len, unsigned short fill = ' ');
	void replace(int pos, int len, const string16& b, unsigned short fill = ' ');
	void replace(int pos, int len, const char * s, unsigned short fill = ' ');
	void remove(int pos = 0, int len = npos);
	void trunc(int len);

	// Miscellaneous methods.
	void format(const char * fmt, ...);
	void formata(const char * fmt, ...);
	void fill(int length, unsigned char fill = ' ');
	void repeat(int count);
	void caseupper();
	void caselower();
	void ltrim (const char* trim=" \t\v\f\r\n");
	void rtrim (const char* trim=" \t\v\f\r\n");
	inline void trim(const char* trim=" \t\v\f\r\n")
	{
		rtrim(trim);
		ltrim(trim);
	}

	int get();
	const string16 readline(const char* terminator);

	int copy(const string16& b);
	int concat(const string16& b);
	int catblk(const void * s, int len);
	int catcstr(const char * s);
	int conchar(short c);
	int iseq(const string16& b) const;
	int iseqcstr(const char * s) const;
	int compare(const string16& b) const;
	int pattern(int len);
	int setstr (int pos, unsigned short fill);
	int setstr (int pos, const string16& b, unsigned short fill, bool truncate = false);

	static const int npos = 0x7FFF;

private:
	static int snapupsize(int i);

#if defined(_WINDOWS)
	wchar_t* data;
#else
	unsigned short * data;
#endif
	int slen;
	int mlen;

	friend class string8;
};


#endif // __DRG_NEW_STRING_H__


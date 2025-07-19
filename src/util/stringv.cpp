
#include "core/memmgr.h"
#include "util/stringv.h"
#include "util/stringn.h"
#include "util/math_def.h"
#include "system/global.h"
#include <wchar.h>

#define USE_STRINGV_BLOCK 0

#define STRINGV_BLOCK_SIZE	47
#define STRINGV_SPACE_SIZE	(STRINGV_BLOCK_SIZE+1)
#if defined(_DEBUG)
static unsigned int stringv_highmem = 0;
static unsigned int stringv_totalbig = 0;
#endif

static unsigned int stringv_maxmem = 0;
static unsigned int stringv_totalmem = 0;
static unsigned int stringv_memptr = 0;
static char* stringv_memblocks = NULL;
drgMutex stringv_memmutex;

void* stringv_getmem(int* size)
{
	if ((*size) > STRINGV_BLOCK_SIZE)
	{
#if defined(_DEBUG)
		stringv_totalbig++;
#endif
		return malloc(*size);
	}

	stringv_totalmem++;
#if !USE_STRINGV_BLOCK
	return malloc(*size);
#else
	if (stringv_totalmem >= stringv_maxmem)
		drgPrintError("vstring max block size needs to increase(%u:%u)!\n", stringv_totalmem, stringv_maxmem);
#if defined(_DEBUG)
	stringv_highmem = DRG_MAX(stringv_highmem, stringv_totalmem);
#endif

	stringv_memmutex.Lock();
	while (stringv_memblocks[stringv_memptr*STRINGV_SPACE_SIZE] != 0)
		stringv_memptr = (stringv_memptr + 1) % stringv_maxmem;
	stringv_memmutex.Unlock();

	(*size) = STRINGV_BLOCK_SIZE;
	stringv_memblocks[stringv_memptr*STRINGV_SPACE_SIZE] = 123;
	return (void*)&stringv_memblocks[(stringv_memptr*STRINGV_SPACE_SIZE)+1];
#endif
}

void stringv_killmem(void* mem, int size)
{
	if (size > STRINGV_BLOCK_SIZE)
	{
#if defined(_DEBUG)
		stringv_totalbig--;
#endif
		free(mem);
	}
	else
	{
		stringv_totalmem--;
#if USE_STRINGV_BLOCK
		char* mem_val = (char*)mem;
		assert(mem_val[-1] == 123);
		mem_val[-1] = 0;
#else
		free(mem);
#endif
	}
}

void InitVStringBlocks(unsigned int max_blocks)
{
#if USE_STRINGV_BLOCK
	stringv_maxmem = max_blocks;
	stringv_memblocks = (char*)drgMemAlloc(stringv_maxmem*STRINGV_SPACE_SIZE);
	memset(stringv_memblocks, 0, stringv_maxmem*STRINGV_SPACE_SIZE);
#endif
}




#if defined(_WINDOWS)
#define UNI_STRING_TYPE wchar_t
#else
#define UNI_STRING_TYPE unsigned short
#endif

string16::string16()
{
	slen = 0;
	mlen = 8 * sizeof(unsigned short);
	data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
	mlen /= sizeof(unsigned short);
	assert(data);
	data[0] = '\0';
}

string16::string16(const void * blk, int len)
{ 
	data = NULL;
	if (len >= 0)
	{
		mlen = (len + 1)*sizeof(unsigned short);
		slen = len;
		data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
		mlen /= sizeof(unsigned short);
	}
	assert(data);
	if (slen > 0)
		memcpy(data, blk, slen*sizeof(unsigned short));
	data[slen] = '\0';
}

string16::string16(char c, int len)
{
	data = NULL;
	if (len >= 0)
	{
		mlen = (len + 1)*sizeof(unsigned short);
		slen = len;
		data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
		mlen /= sizeof(unsigned short);
	}
	assert(data);
	if (slen > 0)
	{
		for(int itr=0; itr<slen; ++itr)
			data[itr] = (unsigned short)c;
	}
	data[slen] = '\0';
}

string16::string16 (short c)
{
	mlen = 2 * sizeof(unsigned short);
	slen = 1;
	data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
	mlen /= sizeof(unsigned short);
	assert(data);
	data[0] = (unsigned short) c;
	data[1] = '\0';
}

string16::string16 (unsigned short c)
{
	mlen = 2 * sizeof(unsigned short);
	slen = 1;
	data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
	mlen /= sizeof(unsigned short);
	assert(data);
	data[0] = c;
	data[1] = '\0';
}

string16::string16 (const char *s)
{
	assert(s);
	unsigned int itr=0;
	unsigned int sslen = drgString::Length(s);
	assert(sslen < DRG_INT_MAX);
	slen = (int) sslen;
	mlen = (slen + 1)*sizeof(unsigned short);
	data = (UNI_STRING_TYPE *)stringv_getmem(&mlen);
	mlen /= sizeof(unsigned short);
	assert(data);
	for(itr=0; itr<sslen; ++itr)
		data[itr] = ((unsigned char*)s)[itr];
	data[slen] = '\0';
}

string16::string16 (int len, const char *s)
{
	assert(s);
	unsigned int itr=0;
	unsigned int sslen = drgString::Length (s);
	assert(sslen < DRG_INT_MAX);
	if (len < (int)sslen)
		sslen = len;
	slen = (int) sslen;
	mlen = slen + 1;
	if (mlen < len)
		mlen = len;
	mlen *= sizeof(unsigned short);
	data = (UNI_STRING_TYPE*)stringv_getmem(&mlen);
	mlen /= sizeof(unsigned short);
	assert(data);
	for(itr=0; itr<sslen; ++itr)
		data[itr] = ((unsigned char*)s)[itr];
	data[slen] = '\0';
}

string16::string16 (const string16& b)
{
	slen = b.slen;
	mlen = slen + 1;
	data = NULL;
	if (mlen > 0)
	{
		mlen *= sizeof(unsigned short);
		data = (UNI_STRING_TYPE*)stringv_getmem(&mlen);
		mlen /= sizeof(unsigned short);
	}
	assert(data);
	memcpy(data, b.data, slen*sizeof(unsigned short));
	data[slen] = '\0';
}

// Destructor.
string16::~string16 ()
{
	if (data != NULL)
	{
		stringv_killmem(data, mlen*sizeof(unsigned short));
		data = NULL;
	}
	mlen = 0;
	slen = -__LINE__;
}

// = operator.
const string16& string16::operator = (char c)
{
	assert(mlen > 0);
	if (2 >= mlen)
		alloc(2);
	assert(data);
	slen = 1;
	data[0] = (unsigned char) c;
	data[1] = '\0';
	return *this;
}

const string16& string16::operator = (unsigned char c)
{
	assert(mlen > 0);
	if (2 >= mlen)
		alloc (2);
	assert(data);
	slen = 1;
	data[0] = c;
	data[1] = '\0';
	return *this;
}

const string16& string16::operator = (const char *s)
{
	unsigned int itr=0;
	unsigned int tmpSlen;
	assert(mlen > 0);
	if (NULL == s)
		s = "";
	if ((tmpSlen = drgString::Length (s)) >= (size_t) mlen)
	{
		assert(tmpSlen < DRG_INT_MAX-1);
		alloc ((int) tmpSlen);
	}

	assert(data);
	slen = (int) tmpSlen;
	tmpSlen++;
	for(itr=0; itr<tmpSlen; ++itr)
		data[itr] = s[itr];
	return *this;
}

const string16& string16::operator = (const string8& b)
{
	return *this = b.c_str();
}

const string16& string16::operator = (const string16& b)
{
	assert(mlen > 0);
	if (b.slen >= mlen)
		alloc (b.slen);

	slen = b.slen;
	assert(data);
	memcpy(data, b.data, slen*sizeof(unsigned short));
	data[slen] = '\0';
	return *this;
}

const string16& string16::operator += (const string16& b)
{
	if (STR_ERR == concat(b))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16& string16::operator += (const char *s)
{
	short * d;
	int i, l;

	assert(mlen > 0);

	l = mlen - slen;
	d = (short *) &data[slen];
	for (i=0; i < l; i++)
	{
		if ((*d++ = *s++) == '\0')
		{
			slen += i;
			return *this;
		}
	}
	slen += i;

	if (STR_ERR == catcstr(s))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16& string16::operator += (char c)
{
	if (STR_ERR == conchar(c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16& string16::operator += (unsigned char c)
{
	if (STR_ERR == conchar((char) c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16& string16::operator += (short c)
{
	if (STR_ERR == conchar(c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16& string16::operator += (unsigned short c)
{
	if (STR_ERR == conchar((char) c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string16 string16::operator + (char c) const
{
	string16 retval(*this);
	retval += c;
	return retval;
}

const string16 string16::operator + (unsigned char c) const
{
	string16 retval (*this);
	retval += c;
	return retval;
}

const string16 string16::operator + (const string16& b) const
{
	string16 retval (*this);
	retval += b;
	return retval;
}

const string16 string16::operator + (const char *s) const
{
	assert(s);
	string16 retval (*this);
	retval += s;
	return retval;
}

const string16 string16::operator + (const unsigned char *s) const
{
	assert(s);
	string16 retval (*this);
	retval += (const char *) s;
	return retval;
}

bool string16::operator == (const string16& b) const
{
	int retval = iseq(b);
	if (STR_ERR == retval) 
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string16::operator == (const char * s) const
{
	assert(s);
	int retval = iseqcstr(s);
	if (STR_ERR == retval)
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string16::operator == (const unsigned char * s) const
{
	assert(s);
	int retval = iseqcstr((const char*)s);
	if (STR_ERR == retval)
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string16::operator != (const string16& b) const
{
	return ! ((*this) == b);
}

bool string16::operator != (const char * s) const
{
	return ! ((*this) == s);
}

bool string16::operator != (const unsigned char * s) const
{
	return ! ((*this) == s);
}

bool string16::operator < (const string16& b) const
{
	int retval = compare(b);
	if (DRG_SHRT_MIN == retval)
		drgPrintError("Failure in compare (<)");
	return retval < 0;
}

bool string16::operator < (const char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, s) < 0;
}

bool string16::operator < (const unsigned char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, (const char *)s) < 0;
}

bool string16::operator <= (const string16& b) const
{
	int retval = compare(b);
	if (DRG_SHRT_MIN == retval)
		drgPrintError("Failure in compare (<=)");
	return retval <= 0;
}

bool string16::operator <= (const char * s) const
{
	assert(s);
	return drgString::Compare((const char *)this->data, s) <= 0;
}

bool string16::operator <= (const unsigned char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, (const char *)s) <= 0;
}

bool string16::operator > (const string16& b) const
{
	return ! ((*this) <= b);
}

bool string16::operator > (const char * s) const
{
	return ! ((*this) <= s);
}

bool string16::operator > (const unsigned char * s) const
{
	return ! ((*this) <= s);
}

bool string16::operator >= (const string16& b) const
{
	return ! ((*this) < b);
}

bool string16::operator >= (const char * s) const
{
	return ! ((*this) < s);
}

bool string16::operator >= (const unsigned char * s) const
{
	return ! ((*this) < s);
}

string16& string16::operator << (const char *val)
{
	(*this) += val;
	return (*this);
}

string16& string16::operator << (char val)
{
	(*this) += val;
	return (*this);
}

string16& string16::operator << (unsigned char val)
{
	(*this) += val;
	return (*this);
}

string16& string16::operator << (int val)
{
	char outval[16];
	sprintf(outval, "%i", val);
	(*this) += outval;
	return (*this);
}

string16& string16::operator << (unsigned int val)
{
	char outval[16];
	sprintf(outval, "%u", val);
	(*this) += outval;
	return (*this);
}

string16& string16::operator << (float val)
{
	char outval[16];
	sprintf(outval, "%f", val);
	(*this) += outval;
	return (*this);
}

string16& string16::operator << (double val)
{
	char outval[16];
	sprintf(outval, "%f", val);
	(*this) += outval;
	return (*this);
}

string16& string16::operator << (string16 val)
{
	(*this) += val;
	return (*this);
}

string16& string16::operator >> (int& val)
{
	int consumed;
	int res = swscanf((wchar_t*)data, L"%i%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string16& string16::operator >> (unsigned int& val)
{
	int consumed;
	int res = swscanf((wchar_t*)data, L"%u%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string16& string16::operator >> (float& val)
{
	int consumed;
	int res = swscanf((wchar_t*)data, L"%f%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string16& string16::operator >> (double& val)
{
	int consumed;
	int res = swscanf((wchar_t*)data, L"%lf%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string16& string16::operator >> (string16& val)
{
	val = (*this);
	clear();
	return (*this);
}



string16::operator double () const
{
	double d = 0;
	if (1 != swscanf ((wchar_t*)this->data, L"%lf", &d))
		drgPrintError("Unable to convert to a double");
	return d;
}

string16::operator float () const
{
	float d = 0;
	if (1 != swscanf ((wchar_t*)this->data, L"%f", &d))
		drgPrintError("Unable to convert to a float");
	return d;
}

string16::operator int () const
{
	int d = 0;
	if (1 != swscanf ((wchar_t*)this->data, L"%d", &d))
		drgPrintError("Unable to convert to an int");
	return d;
}

string16::operator unsigned int () const
{
	unsigned int d = 0;
	if (1 != swscanf ((wchar_t*)this->data, L"%u", &d))
		drgPrintError("Unable to convert to an unsigned int");
	return d;
}

int string16::find(const string16& b, int pos) const
{
	int j, ii, ll, lf;
	UNI_STRING_TYPE * d0;
	unsigned short c0;
	UNI_STRING_TYPE * d1;
	unsigned short c1;
	int i;

	if (data == NULL || slen < 0 || b.data == NULL || b.slen < 0)
		return npos;
	if (slen == pos)
		return (b.slen == 0)?pos:npos;
	if (slen < pos || pos < 0)
		return npos;
	if (b.slen == 0)
		return pos;

	if ((lf = slen - b.slen + 1) <= pos)
		return npos;

	if (data == b.data && pos == 0)
		return 0;

	i = pos;

	d0 = b.data;
	d1 = data;
	ll = b.slen;

	c0 = d0[0];
	if (1 == ll)
	{
		for (;i < lf; i++) if (c0 == d1[i])
			return i;
		return npos;
	}

	c1 = c0;
	j = 0;
	lf = slen - 1;

	ii = -1;
	if (i < lf) do
	{
		if (c1 != d1[i])
		{
			if (c1 != d1[1+i])
			{
				i += 2;
				continue;
			}
			i++;
		}
		
		if (0 == j)
			ii = i;

		j++;
		i++;

		if (j < ll)
		{
			c1 = d0[j];
			continue;
		}

N0:;
		if (i == ii+j)
			return ii;

		i -= j;
		j  = 0;
		c1 = c0;
	} while (i < lf);

	if (i == lf && ll == j+1 && c1 == d1[i])
		goto N0;
	return npos;
}


int string16::find(const char * b, int pos) const
{
	int ii, j;
	unsigned short c0;
	int i, l;
	unsigned short cx;
	UNI_STRING_TYPE * pdata;

	if (NULL == b)
		return npos;

	if ((unsigned int) pos > (unsigned int) slen)
		return npos;
	if ('\0' == b[0])
		return pos;
	if (pos == slen)
		return npos;
	if ('\0' == b[1])
		return find(b[0], pos);
	cx = c0 = (unsigned short) b[0];
	l = slen - 1;
	pdata = data;
	for (ii = -1, i = pos, j = 0; i < l;)
	{
		if (cx != pdata[i])
		{
			if (cx != pdata[1+i])
			{
				i += 2;
				continue;
			}
			i++;
		}

		if (0 == j)
			ii = i;
		j++;
		i++;
		if ('\0' != (cx = b[j]))
			continue;
N0:;
		if (i == ii+j)
			return ii;
		i -= j;
		j = 0;
		cx = c0;
	}

	if (i == l && cx == pdata[i] && '\0' == b[j+1])
		goto N0;
	return npos;
}

int string16::find (char c, int pos) const
{
	if (pos < 0)
		return npos;
	for(;pos < slen; pos++)
	{
		if (data[pos] == (unsigned char) c)
			return pos;
	}
	return npos;
}


#if 0
int string16::caselessfind (const string16& b, int pos) const
{
	return binstrcaseless ((bstring) this, pos, (bstring) &b);
}

int string16::caselessfind (const char * b, int pos) const
{
	struct tagbstring t;

	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::caselessfind NULL.");
#else
		return STR_ERR;
#endif
	}

	if ((unsigned int) pos > (unsigned int) slen) return STR_ERR;
	if ('\0' == b[0]) return pos;
	if (pos == slen) return STR_ERR;

	btfromcstr (t, b);
	return binstrcaseless ((bstring) this, pos, (bstring) &t);
}

int string16::reversefind (const string16& b, int pos) const {
	return binstrr ((bstring) this, pos, (bstring) &b);
}

int string16::reversefind (const char * b, int pos) const {
	struct tagbstring t;
	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::reversefind NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, b);
	return binstrr ((bstring) this, pos, &t);
}

int string16::caselessreversefind (const string16& b, int pos) const {
	return binstrrcaseless ((bstring) this, pos, (bstring) &b);
}

int string16::caselessreversefind (const char * b, int pos) const {
	struct tagbstring t;

	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::caselessreversefind NULL.");
#else
		return STR_ERR;
#endif
	}

	if ((unsigned int) pos > (unsigned int) slen) return STR_ERR;
	if ('\0' == b[0]) return pos;
	if (pos == slen) return STR_ERR;

	btfromcstr (t, b);
	return binstrrcaseless ((bstring) this, pos, (bstring) &t);
}

int string16::reversefind (char c, int pos) const {
	if (pos > slen) return STR_ERR;
	if (pos == slen) pos--;
	for (;pos >= 0; pos--) {
		if (data[pos] == (unsigned char) c) return pos;
	}
	return STR_ERR;
}

int string16::findchr (const string16& b, int pos) const {
	return binchr ((bstring) this, pos, (bstring) &b);
}

int string16::findchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::findchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return binchr ((bstring) this, pos, (bstring) &t);
}

int string16::nfindchr (const string16& b, int pos) const {
	return bninchr ((bstring) this, pos, (bstring) &b);
}

int string16::nfindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::nfindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return bninchr ((bstring) this, pos, &t);
}

int string16::reversefindchr (const string16& b, int pos) const {
	return binchrr ((bstring) this, pos, (bstring) &b);
}

int string16::reversefindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::reversefindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return binchrr ((bstring) this, pos, &t);
}

int string16::nreversefindchr (const string16& b, int pos) const {
	return bninchrr ((bstring) this, pos, (bstring) &b);
}

int string16::nreversefindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string16::nreversefindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return bninchrr ((bstring) this, pos, &t);
}
#endif

int string16::find_first_of(const char * b, int pos) const
{
	assert(b);
	int count = 0;
	int temp = 0;
	int maxcount = drgString::Length(b);
	int retval = npos;
	for( ; count<maxcount; ++count)
	{
		temp = find(b[count], pos);
		if((temp!=STR_ERR) && (temp<retval))
			retval = temp;
	}
	return retval;
}

int string16::find_last_of(const char * b, int pos) const
{
	assert(b);
	int count = 0;
	int temp = 0;
	int maxcount = drgString::Length(b);
	int retval = -1;
	for( ; count<maxcount; ++count)
	{
		temp = find(b[count], pos);
		if((temp!=STR_ERR) && (temp>retval))
			retval = temp;
	}
	if(retval<0)
		retval = npos;
	return retval;
}

int string16::find_first_not_of(const char * b, int pos) const
{
	assert(b);
	int count1 = pos;
	int count2 = 0;
	bool found = false;
	int maxcount = drgString::Length(b);
	for( ; count1<slen; ++count1)
	{
		found = false;
		for(count2=0; count2<maxcount; ++count2)
		{
			if(b[count2] == data[count1])
			{
				found = true;
				break;
			}
		}
		if(found == false)
			return count1;
	}
	return npos;
}

int string16::find_last_not_of(const char * b, int pos) const
{
	assert(b);
	int count1 = slen-1;
	int count2 = 0;
	bool found = false;
	int maxcount = drgString::Length(b);
	for( ; count1>=pos; --count1)
	{
		found = false;
		for(count2=0; count2<maxcount; ++count2)
		{
			if(b[count2] == data[count1])
			{
				found = true;
				break;
			}
		}
		if(found == false)
			return count1;
	}
	return npos;
}

const string16 string16::substr(int pos, int len) const
{
	if (pos < 0)
	{
		len += pos;
		pos = 0;
	}
	if (len > slen - pos)
		len = slen - pos;
	if (len <= 0)
		return string16("");
	return string16((data + pos), len);
}

void string16::alloc(int olen)
{
	int len;
	if (data == NULL || slen < 0 || mlen <= 0 ||  mlen < slen || olen <= 0)
		drgPrintError("string16::alloc");

	if (olen >= mlen)
	{
		UNI_STRING_TYPE * x;
		len = snapupsize(olen);
		if (len <= mlen)
			return;

		int newlen = len*sizeof(unsigned short);
		x = (UNI_STRING_TYPE*)stringv_getmem(&newlen);
		if (slen)
			memcpy((short *) x, (short *) data, (size_t) slen*sizeof(unsigned short));
		stringv_killmem(data, (size_t)mlen*sizeof(unsigned short));

		data = x;
		mlen = newlen/sizeof(unsigned short);
		data[slen] = (unsigned short) '\0';
	}
}

void string16::clear()
{
	slen = 0;
	data[0] = '\0';
}


#if 0
void string16::fill(int len, unsigned char cfill)
{
	slen = 0;
	if (STR_ERR == bsetstr (this, len, NULL, cfill))
		drgPrintError("Failure in fill");
}

void string16::setsubstr (int pos, const string16& b, unsigned char cfill)
{
	if (STR_ERR == bsetstr (this, pos, (bstring) &b, cfill))
		drgPrintError("Failure in setsubstr");
}

void string16::setsubstr (int pos, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("setsubstr NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, s);
	if (STR_ERR == bsetstr (this, pos, &t, cfill))
		drgPrintError("Failure in setsubstr");
}

void string16::insert (int pos, const string16& b, unsigned char cfill)
{
	if (STR_ERR == binsert(this, pos, (bstring) &b, cfill))
		drgPrintError("Failure in insert");
}

void string16::insert(int pos, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	if (NULL == s)
	{
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("insert NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, s);
	if (STR_ERR == binsert (this, pos, &t, cfill))
		drgPrintError("Failure in insert");
}

void string16::insertchrs(int pos, int len, unsigned char cfill)
{
	if (STR_ERR == binsertch (this, pos, len, cfill))
		drgPrintError("Failure in insertchrs");
}

void string16::replace(int pos, int len, const string16& b, unsigned char cfill)
{
	if (STR_ERR == breplace (this, pos, len, (bstring) &b, cfill))
		drgPrintError("Failure in replace");
}

void string16::replace (int pos, int len, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	size_t q;

	if (mlen <= 0) drgPrintError("Write protection error");
	if (NULL == s || (pos|len) < 0) {
		drgPrintError("Failure in replace");
	} else {
		if (pos + len >= slen) {
			cstr2tbstr (t, s);
			if (STR_ERR == bsetstr (this, pos, &t, cfill)) {
				drgPrintError("Failure in replace");
			} else if (pos + t.slen < slen) {
				slen = pos + t.slen;
				data[slen] = '\0';
			}
		} else {

			/* Aliasing case */
			if ((unsigned int) (data - (unsigned char *) s) < (unsigned int) slen) {
				replace (pos, len, string16(s), cfill);
				return;
			}

			if ((q = drgString::Length (s)) > (size_t) len || len < 0) {
				if (slen + q - len >= INT_MAX) drgPrintError("Failure in replace, result too long.");
				alloc ((int) (slen + q - len));
				if (NULL == data) return;
			}
			if ((int) q != len) bstr__memmove (data + pos + q, data + pos + len, slen - (pos + len));
			memcpy(data + pos, s, q);
			slen += ((int) q) - len;
			data[slen] = '\0';
		}
	}
}

void string16::findreplace (const string16& sfind, const string16& repl, int pos) {
	if (STR_ERR == bfindreplace (this, (bstring) &sfind, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string16::findreplace (const string16& sfind, const char * repl, int pos) {
	struct tagbstring t;
	if (NULL == repl) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, repl);
	if (STR_ERR == bfindreplace (this, (bstring) &sfind, (bstring) &t, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string16::findreplace (const char * sfind, const string16& repl, int pos) {
	struct tagbstring t;
	if (NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	if (STR_ERR == bfindreplace (this, (bstring) &t, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string16::findreplace (const char * sfind, const char * repl, int pos) {
	struct tagbstring t, u;
	if (NULL == repl || NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	cstr2tbstr (u, repl);
	if (STR_ERR == bfindreplace (this, (bstring) &t, (bstring) &u, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string16::findreplacecaseless (const string16& sfind, const string16& repl, int pos) {
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &sfind, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string16::findreplacecaseless (const string16& sfind, const char * repl, int pos) {
	struct tagbstring t;
	if (NULL == repl) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, repl);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &sfind, (bstring) &t, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string16::findreplacecaseless (const char * sfind, const string16& repl, int pos) {
	struct tagbstring t;
	if (NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &t, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string16::findreplacecaseless (const char * sfind, const char * repl, int pos) {
	struct tagbstring t, u;
	if (NULL == repl || NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	cstr2tbstr (u, repl);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &t, (bstring) &u, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}
#endif

void string16::remove(int pos, int len)
{
	if (pos < 0) {
		len += pos;
		pos = 0;
	}
	if (len < 0 || data == NULL || slen < 0 || mlen < slen || mlen <= 0) {
		return;
	}
	if (len > 0 && pos < slen) {
		if (pos + len >= slen) {
			slen = pos;
		}
		else {
			if ((slen - (pos + len)) > 0) {
				memmove((data + pos), (data + pos + len), (slen - (pos + len)) * sizeof(unsigned short));
			}
			slen -= len;
		}
		data[slen] = (unsigned char) '\0';
	}
}

void string16::trunc(int len)
{
	if (len < 0) {
		drgPrintError("Failure in trunc");
	}
	if (len < slen) {
		slen = len;
		data[len] = '\0';
	}
}

void string16::ltrim(const char* trim)
{
	remove(0, find_first_not_of(trim));
}

void string16::rtrim(const char* trim)
{
	remove(find_last_not_of(trim) + 1);
}

void string16::caseupper()
{
	int i, len;
	if(data == NULL || mlen < slen || slen < 0 || mlen <= 0)
		return;
	for(i=0, len = slen; i < len; i++)
		data[i] = (unsigned char)toupper(data[i]);
}

void string16::caselower()
{
	int i, len;
	if(data == NULL || mlen < slen || slen < 0 || mlen <= 0)
		return;
	for(i=0, len = slen; i < len; i++)
		data[i] = (unsigned char)tolower(data[i]);
}

void string16::repeat (int count)
{
	count *= slen;
	if (count == 0)
	{
		trunc(0);
		return;
	}
	if (count < 0 || STR_ERR == pattern (count))
		drgPrintError("Failure in repeat");
}

int string16::get()
{
	if(slen<=0)
		return -1;
	int retval = data[0];
	remove(0, 1);
	return retval;
}

const string16 string16::readline(const char* terminator)
{
	int count;
	int termlen = drgString::Length(terminator);
	string16 retval;
	char getch;
	while(slen > 0)
	{
		getch = get();
		retval += getch;
		for(count=0; count<termlen; ++count)
		{
			if(terminator[count]==getch)
				break;
		}
		if(count<termlen)
			break;
	}
	return retval;
}

int string16::copy(const string16& b)
{
	int i,j;

	if (b.slen < 0 || b.data == NULL)
		return STR_ERR;

	i = b.slen;
	j = snapupsize(i + 1);

	alloc(j);
	mlen = j;
	slen = i;

	if (i)
		memcpy((char *)data, (char *) b.data, i*sizeof(unsigned char));
	data[slen] = (unsigned char) '\0';
	return STR_OK;
}

int string16::concat(const string16& b)
{
	int len, d;

	if (data == NULL || b.data == NULL)
		return STR_ERR;

	d = slen;
	len = b.slen;
	if ((d | (mlen - d) | len | (d + len)) < 0)
		return STR_ERR;

	if (mlen <= d + len + 1)
	{
		long long pd = b.data - data;
		alloc(d + len + 1);
	}

	if (len > 0)
		memmove(&data[d], &b.data[0], len*sizeof(unsigned short));
	data[d + len] = (unsigned char) '\0';
	slen = d + len;
	return STR_OK;
}

int string16::catcstr(const char * s)
{
	short * d;
	int itr, nl;

	if (data == NULL || slen < 0 || mlen < slen	|| mlen <= 0 || s == NULL)
		return STR_ERR;

	nl = mlen - slen;
	d = (short *)&data[slen];
	for (itr=0; itr < nl; itr++)
	{
		if ((*d++ = *s++) == '\0')
		{
			slen += itr;
			return STR_OK;
		}
	}
	slen += itr;

	int len = (int)drgString::Length(s);
	if (0 > (nl = slen + len))
		return STR_ERR;
	if (mlen <= nl)
		alloc(nl + 1);

	itr=0;
	while(itr<len)
		data[slen++] = s[itr++];
	slen = nl;
	data[nl] = (unsigned char) '\0';
	return STR_OK;
}

int string16::catblk(const void * s, int len)
{
	int nl;
	if (data == NULL || slen < 0 || mlen < slen || mlen <= 0 || s == NULL || len < 0)
		return STR_ERR;

	if (0 > (nl = slen + len))
		return STR_ERR;
	if (mlen <= nl)
		alloc(nl + 1);

	if (len > 0)
		memmove(&data[slen], s, len);
	slen = nl;
	data[nl] = (unsigned char) '\0';
	return STR_OK;
}

int string16::conchar(short c)
{
	int d;
	d = slen;
	if((d | (mlen - d)) < 0)
		return STR_ERR;
	alloc(d + 2);
	data[d] = (unsigned short) c;
	data[d + 1] = (unsigned short) '\0';
	slen++;
	return STR_OK;
}

int string16::iseq(const string16& b) const
{
	if (data == NULL || b.data == NULL || slen < 0 || b.slen < 0)
		return STR_ERR;
	if (slen != b.slen)
		return STR_OK;
	if (data == b.data || slen == 0)
		return 1;
	return !memcmp(data, b.data, slen*sizeof(unsigned short));
}

int string16::iseqcstr(const char * s) const
{
	int i;
	if(s == NULL || data == NULL || slen < 0)
		return STR_ERR;
	for (i=0; i < slen; i++)
	{
		if (s[i] == '\0' || data[i] != (unsigned short) s[i])
			return STR_OK;
	}
	return s[i] == '\0';
}

int string16::compare(const string16& b) const
{
	int i, v, n;
	if(data == NULL || b.data == NULL || slen < 0 || b.slen < 0)
		return DRG_SHRT_MIN;
	n = slen; if (n > b.slen) n = b.slen;
	if (slen == b.slen && (data == b.data || slen == 0))
		return STR_OK;

	for (i = 0; i < n; i ++)
	{
		v = ((short) data[i]) - ((short) b.data[i]);
		if (v != 0)
			return v;
		if (data[i] == (unsigned char) '\0')
			return STR_OK;
	}

	if (slen > n)
		return 1;
	if (b.slen > n)
		return -1;
	return STR_OK;
}

int string16::pattern(int len)
{
	int i, d;
	d = length();
	if (d <= 0 || len < 0)
		return STR_ERR;
	alloc(len + 1);
	if (len > 0)
	{
		if (d == 1)
			return setstr(len, data[0]);
		for (i = d; i < len; i++)
			data[i] = data[i - d];
	}
	data[len] = (unsigned char) '\0';
	slen = len;
	return STR_OK;
}

int string16::setstr(int pos, unsigned short fill)
{
	int d, newlen;
	if (pos < 0 || slen < 0 || NULL == data ||  mlen < slen || mlen <= 0)
		return STR_ERR;

	d = pos;
	alloc(d + 1);
	newlen = slen;
	if (pos > newlen)
	{
		memset(data + slen, (int) fill, (size_t) (pos - slen));
		newlen = pos;
	}

	if (d > newlen)
		newlen = d;

	slen = newlen;
	data[newlen] = (unsigned char) '\0';

	return STR_OK;
}

//Sets the data in this string to 'b'.
// if 'truncate' is true, then this string will be shortened if the new string is shorter than the original string.
// 'pos' the starting position of this string at which to copy the string 'b'.  
// if 'pos' is past the end of this string, then 'fill' will be coppied into all characters between the end of this string and 'pos'
int string16::setstr(int pos, const string16& b, unsigned short fill, bool truncate)
{
	int d, newlen;
	if (pos < 0 || slen < 0 || NULL == data ||  mlen < slen || mlen <= 0)
		return STR_ERR;
	if (b.slen < 0 || b.data == NULL)
		return STR_ERR;

	d = pos;
	d += b.slen;

	alloc(d + 1);
	newlen = slen;
	if (pos > newlen)
	{
		memset(data + slen, (int) fill, (size_t) (pos - slen));
		newlen = pos;
	}

	if (b.slen > 0)
		memmove((data + pos), b.data, b.slen*sizeof(unsigned short));

	if (truncate || d > newlen)
		newlen = d;

	slen = newlen;
	data[newlen] = (unsigned char) '\0';

	return STR_OK;
}

void string16::insert(int pos, short c, unsigned short fill)
{
	insert(pos, string16(c), fill);
}

void string16::insert(int pos, const string16& b, unsigned short fill)
{
	int d, l;

	if (pos < 0 || b.slen < 0 || mlen <= 0)
		return;

	d = slen + b.slen;
	l = pos + b.slen;
	if ((d|l) < 0)
		return;

	if (l > d)
	{
		alloc(l + 1);
		memset(data + slen, (int)fill, (size_t) (pos - slen));
		slen = l;
	}
	else
	{
		alloc(d + 1);
		if ((d - l) > 0)
			memmove(data + l, data + pos, (d - l)*sizeof(unsigned short));
		slen = d;
	}
	if (b.slen > 0)
		memmove(data + pos, b.data, b.slen*sizeof(unsigned short));
	data[slen] = (unsigned char) '\0';
}


int string16::snapupsize(int i)
{
	if (i < 8)
	{
		i = 8;
	}
	else
	{
		unsigned int j;
		j = (unsigned int) i;
		j |= (j >>  1);
		j |= (j >>  2);
		j |= (j >>  4);
		j |= (j >>  8);		/* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xffff)
		j |= (j >> 16);		/* For 32 bit int systems */
#if (UINT_MAX > 0xffffffffUL)
		j |= (j >> 32);		/* For 64 bit int systems */
#endif
#endif
		j++;
		if ((int) j >= i)
			i = (int) j;
	}
	return i;
}


const string16 operator + (const char *a, const string16& b)
{
	return string16(a) + b;
}

const string16 operator + (const unsigned char *a, const string16& b)
{
	return string16((const char *)a) + b;
}

const string16 operator + (char c, const string16& b)
{
	return string16((short)c) + b;
}

const string16 operator + (unsigned char c, const string16& b)
{
	return string16((short)c) + b;
}












#include "stringv.h"



string8::string8()
{
	slen = 0;
	mlen = 8;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	data[0] = '\0';
}

string8::string8(const void * blk, int len)
{ 
	data = NULL;
	if (len >= 0)
	{
		mlen = len + 1;
		slen = len;
		data = (unsigned char *)stringv_getmem(&mlen);
	}
	assert(data);
	if (slen > 0)
		memcpy(data, blk, slen);
	data[slen] = '\0';
}

string8::string8(char c, int len)
{
	data = NULL;
	if (len >= 0)
	{
		mlen = len + 1;
		slen = len;
		data = (unsigned char *)stringv_getmem(&mlen);
	}
	assert(data);
	if (slen > 0)
		memset(data, c, slen);
	data[slen] = '\0';
}

string8::string8 (char c)
{
	mlen = 2;
	slen = 1;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	data[0] = (unsigned char) c;
	data[1] = '\0';
}

string8::string8 (unsigned char c)
{
	mlen = 2;
	slen = 1;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	data[0] = c;
	data[1] = '\0';
}

string8::string8 (const char *s)
{
	assert(s);
	size_t sslen = drgString::Length (s);
	assert(sslen < DRG_INT_MAX);
	slen = (int) sslen;
	mlen = slen + 1;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	memcpy(data, s, mlen);
	data[slen] = '\0';
}

string8::string8 (int len, const char *s)
{
	assert(s);
	size_t sslen = drgString::Length (s);
	assert(sslen < DRG_INT_MAX);
	if (len < (int)sslen)
		sslen = len;
	slen = (int) sslen;
	mlen = slen + 1;
	if (mlen < len)
		mlen = len;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	memcpy(data, s, slen + 1);
	data[slen] = '\0';
}

string8::string8 (int len, const short *s)
{
	assert(s);
	size_t sslen = drgStringUni::Length(s);
	assert(sslen < DRG_INT_MAX);
	if (len < (int)sslen)
		sslen = len;
	slen = (int) sslen;
	mlen = slen + 1;
	if (mlen < len)
		mlen = len;
	data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	int itr=0;
	while(itr<slen)
    {
		data[itr] = (unsigned char)s[itr];
        itr++;
    }
	data[slen] = '\0';
}

string8::string8 (const string8& b)
{
	slen = b.slen;
	mlen = slen + 1;
	data = NULL;
	if (mlen > 0)
		data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	memcpy(data, b.data, slen);
	data[slen] = '\0';
}

string8::string8 (const string16& b)
{
	slen = b.slen;
	mlen = slen + 1;
	data = NULL;
	if (mlen > 0)
		data = (unsigned char *)stringv_getmem(&mlen);
	assert(data);
	int itr=0;
	while(itr<slen)
    {
		data[itr] = (unsigned char)b.data[itr];
        itr++;
    }
	data[slen] = '\0';
}

// Destructor.
string8::~string8 ()
{
	if (data != NULL)
	{
		stringv_killmem(data, mlen);
		data = NULL;
	}
	mlen = 0;
	slen = -__LINE__;
}

// = operator.
const string8& string8::operator = (char c)
{
	assert(mlen > 0);
	if (2 >= mlen)
		alloc(2);
	assert(data);
	slen = 1;
	data[0] = (unsigned char) c;
	data[1] = '\0';
	return *this;
}

const string8& string8::operator = (unsigned char c)
{
	assert(mlen > 0);
	if (2 >= mlen)
		alloc (2);
	assert(data);
	slen = 1;
	data[0] = c;
	data[1] = '\0';
	return *this;
}

const string8& string8::operator = (const char *s)
{
	size_t tmpSlen;
	assert(mlen > 0);
	if (NULL == s)
		s = "";
	if ((tmpSlen = drgString::Length (s)) >= (size_t) mlen)
	{
		assert(tmpSlen < DRG_INT_MAX-1);
		alloc ((int) tmpSlen);
	}

	assert(data);
	slen = (int) tmpSlen;
	memcpy(data, s, tmpSlen + 1);
	return *this;
}

const string8& string8::operator = (const string8& b)
{
	assert(mlen > 0);
	if (b.slen >= mlen)
		alloc (b.slen);

	slen = b.slen;
	assert(data);
	memcpy(data, b.data, slen);
	data[slen] = '\0';
	return *this;
}

const string8& string8::operator = (const string16& b)
{
	assert(mlen > 0);
	if (b.slen >= mlen)
		alloc (b.slen);

	slen = b.slen;
	assert(data);
	int itr = 0;
	while (itr<slen)
    {
		data[itr] = (unsigned char)b.data[itr];
        itr++;
    }
	data[slen] = '\0';
	return *this;
}

const string8& string8::operator += (const string8& b)
{
	if (STR_ERR == concat(b))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string8& string8::operator += (const char *s)
{
	char * d;
	int i, l;

	assert(mlen > 0);

	l = mlen - slen;
	d = (char *) &data[slen];
	for (i=0; i < l; i++)
	{
		if ((*d++ = *s++) == '\0')
		{
			slen += i;
			return *this;
		}
	}
	slen += i;

	if (STR_ERR == catcstr(s))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string8& string8::operator += (char c)
{
	if (STR_ERR == conchar(c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string8& string8::operator += (unsigned char c)
{
	if (STR_ERR == conchar((char) c))
		drgPrintError("Failure in concatenate");
	return *this;
}

const string8 string8::operator + (char c) const
{
	string8 retval(*this);
	retval += c;
	return retval;
}

const string8 string8::operator + (unsigned char c) const
{
	string8 retval (*this);
	retval += c;
	return retval;
}

const string8 string8::operator + (const string8& b) const
{
	string8 retval (*this);
	retval += b;
	return retval;
}

const string8 string8::operator + (const char *s) const
{
	assert(s);
	string8 retval (*this);
	retval += s;
	return retval;
}

const string8 string8::operator + (const unsigned char *s) const
{
	assert(s);
	string8 retval (*this);
	retval += (const char *) s;
	return retval;
}

bool string8::operator == (const string8& b) const
{
	int retval = iseq(b);
	if (STR_ERR == retval) 
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string8::operator == (const char * s) const
{
	assert(s);
	int retval = iseqcstr(s);
	if (STR_ERR == retval)
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string8::operator == (const unsigned char * s) const
{
	assert(s);
	int retval = iseqcstr((const char*)s);
	if (STR_ERR == retval)
		drgPrintError("Failure in compare (==)");
	return retval > 0;
}

bool string8::operator != (const string8& b) const
{
	return ! ((*this) == b);
}

bool string8::operator != (const char * s) const
{
	return ! ((*this) == s);
}

bool string8::operator != (const unsigned char * s) const
{
	return ! ((*this) == s);
}

bool string8::operator < (const string8& b) const
{
	int retval = compare(b);
	if (DRG_SHRT_MIN == retval)
		drgPrintError("Failure in compare (<)");
	return retval < 0;
}

bool string8::operator < (const char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, s) < 0;
}

bool string8::operator < (const unsigned char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, (const char *)s) < 0;
}

bool string8::operator <= (const string8& b) const
{
	int retval = compare(b);
	if (DRG_SHRT_MIN == retval)
		drgPrintError("Failure in compare (<=)");
	return retval <= 0;
}

bool string8::operator <= (const char * s) const
{
	assert(s);
	return drgString::Compare((const char *)this->data, s) <= 0;
}

bool string8::operator <= (const unsigned char * s) const
{
	assert(s);
	return drgString::Compare ((const char *)this->data, (const char *)s) <= 0;
}

bool string8::operator > (const string8& b) const
{
	return ! ((*this) <= b);
}

bool string8::operator > (const char * s) const
{
	return ! ((*this) <= s);
}

bool string8::operator > (const unsigned char * s) const
{
	return ! ((*this) <= s);
}

bool string8::operator >= (const string8& b) const
{
	return ! ((*this) < b);
}

bool string8::operator >= (const char * s) const
{
	return ! ((*this) < s);
}

bool string8::operator >= (const unsigned char * s) const
{
	return ! ((*this) < s);
}

string8& string8::operator << (const char *val)
{
	(*this) += val;
	return (*this);
}

string8& string8::operator << (char val)
{
	(*this) += val;
	return (*this);
}

string8& string8::operator << (unsigned char val)
{
	(*this) += val;
	return (*this);
}

string8& string8::operator << (int val)
{
	char outval[16];
	drgString::ItoA(val, outval, 10);
	(*this) += outval;
	return (*this);
}

string8& string8::operator << (unsigned int val)
{
	char outval[16];
	drgString::ItoA(val, outval, 10);
	(*this) += outval;
	return (*this);
}

string8& string8::operator << (float val)
{
	char outval[16];
	sprintf(outval, "%f", val);
	(*this) += outval;
	return (*this);
}

string8& string8::operator << (double val)
{
	char outval[16];
	sprintf(outval, "%f", val);
	(*this) += outval;
	return (*this);
}

string8& string8::operator << (string8 val)
{
	(*this) += val;
	return (*this);
}

string8& string8::operator >> (int& val)
{
	int consumed;
	int res = sscanf((char*)data, "%i%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string8& string8::operator >> (unsigned int& val)
{
	int consumed;
	int res = sscanf((char*)data, "%u%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string8& string8::operator >> (float& val)
{
	int consumed;
	int res = sscanf((char*)data, "%f%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string8& string8::operator >> (double& val)
{
	int consumed;
	int res = sscanf((char*)data, "%lf%n", &val, &consumed);
	if(res>=1)
		remove(0, consumed);
	return (*this);
}

string8& string8::operator >> (string8& val)
{
	val = (*this);
	clear();
	return (*this);
}



string8::operator double () const
{
	double d = 0;
	if (1 != sscanf ((const char *)this->data, "%lf", &d))
		drgPrintError("Unable to convert to a double");
	return d;
}

string8::operator float () const
{
	float d = 0;
	if (1 != sscanf ((const char *)this->data, "%f", &d))
		drgPrintError("Unable to convert to a float");
	return d;
}

string8::operator int () const
{
	int d = 0;
	if (1 != sscanf ((const char *)this->data, "%d", &d))
		drgPrintError("Unable to convert to an int");
	return d;
}

string8::operator unsigned int () const
{
	unsigned int d = 0;
	if (1 != sscanf ((const char *)this->data, "%u", &d))
		drgPrintError("Unable to convert to an unsigned int");
	return d;
}


#if 0
void string8::format (const char * fmt, ...)
{
	bstring b;
	va_list arglist;
	int r, n;

	if (mlen <= 0) drgPrintError("Write protection error");
	if (fmt == NULL) {
		*this = "<NULL>";
		drgPrintError("string8::format (NULL, ...) is erroneous.");
	} else {

		if ((b = bfromcstr ("")) == NULL) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
			drgPrintError("string8::format out of memory.");
#else
			*this = "<NULL>";
#endif
		} else {
			if ((n = (int) (2 * (drgString::Length) (fmt))) < START_VSNBUFF) n = START_VSNBUFF;
			for (;;) {
				if (STR_OK != balloc (b, n + 2)) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
					drgPrintError("string8::format out of memory.");
#else
					b = bformat ("<NULL>");
					break;
#endif
				}

				va_start (arglist, fmt);
				exvsnprintf (r, (char *) b->data, n + 1, fmt, arglist);
				va_end (arglist);

				b->data[n] = '\0';
				b->slen = (int) (drgString::Length) ((char *) b->data);

				if (b->slen < n) break;
				if (r > n) n = r; else n += n;
			}
			*this = *b;
			bdestroy (b);
		}
	}
}

void string8::formata (const char * fmt, ...) {
	bstring b;
	va_list arglist;
	int r, n;

	if (mlen <= 0) drgPrintError("Write protection error");
	if (fmt == NULL) {
		*this += "<NULL>";
		drgPrintError("string8::formata (NULL, ...) is erroneous.");
	} else {

		if ((b = bfromcstr ("")) == NULL) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
			drgPrintError("string8::format out of memory.");
#else
			*this += "<NULL>";
#endif
		} else {
			if ((n = (int) (2 * (drgString::Length) (fmt))) < START_VSNBUFF) n = START_VSNBUFF;
			for (;;) {
				if (STR_OK != balloc (b, n + 2)) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
					drgPrintError("string8::format out of memory.");
#else
					b = bformat ("<NULL>");
					break;
#endif
				}

				va_start (arglist, fmt);
				exvsnprintf (r, (char *) b->data, n + 1, fmt, arglist);
				va_end (arglist);

				b->data[n] = '\0';
				b->slen = (int) (drgString::Length) ((char *) b->data);

				if (b->slen < n) break;
				if (r > n) n = r; else n += n;
			}
			*this += *b;
			bdestroy (b);
		}
	}
}

int string8::caselessEqual (const string8& b) const {
	int ret;
	if (STR_ERR == (ret = biseqcaseless ((bstring) this, (bstring) &b))) {
		drgPrintError("string8::caselessEqual Unable to compare");
	}
	return ret;
}

int string8::caselessCmp (const string8& b) const {
	int ret;
	if (SHRT_MIN == (ret = bstricmp ((bstring) this, (bstring) &b))) {
		drgPrintError("string8::caselessCmp Unable to compare");
	}
	return ret;
}
#endif


int string8::find(const string8& b, int pos) const
{
	int j, ii, ll, lf;
	unsigned char * d0;
	unsigned char c0;
	unsigned char * d1;
	unsigned char c1;
	int i;

	if (data == NULL || slen < 0 || b.data == NULL || b.slen < 0)
		return npos;
	if (slen == pos)
		return (b.slen == 0)?pos:npos;
	if (slen < pos || pos < 0)
		return npos;
	if (b.slen == 0)
		return pos;

	if ((lf = slen - b.slen + 1) <= pos)
		return npos;

	if (data == b.data && pos == 0)
		return 0;

	i = pos;

	d0 = b.data;
	d1 = data;
	ll = b.slen;

	c0 = d0[0];
	if (1 == ll)
	{
		for (;i < lf; i++) if (c0 == d1[i])
			return i;
		return npos;
	}

	c1 = c0;
	j = 0;
	lf = slen - 1;

	ii = -1;
	if (i < lf) do
	{
		if (c1 != d1[i])
		{
			if (c1 != d1[1+i])
			{
				i += 2;
				continue;
			}
			i++;
		}
		
		if (0 == j)
			ii = i;

		j++;
		i++;

		if (j < ll)
		{
			c1 = d0[j];
			continue;
		}

N0:;
		if (i == ii+j)
			return ii;

		i -= j;
		j  = 0;
		c1 = c0;
	} while (i < lf);

	if (i == lf && ll == j+1 && c1 == d1[i])
		goto N0;
	return npos;
}


int string8::find(const char * b, int pos) const
{
	int ii, j;
	unsigned char c0;
	int i, l;
	unsigned char cx;
	unsigned char * pdata;

	if (NULL == b)
		return npos;

	if ((unsigned int) pos > (unsigned int) slen)
		return npos;
	if ('\0' == b[0])
		return pos;
	if (pos == slen)
		return npos;
	if ('\0' == b[1])
		return find(b[0], pos);
	cx = c0 = (unsigned char) b[0];
	l = slen - 1;
	pdata = data;
	for (ii = -1, i = pos, j = 0; i < l;)
	{
		if (cx != pdata[i])
		{
			if (cx != pdata[1+i])
			{
				i += 2;
				continue;
			}
			i++;
		}

		if (0 == j)
			ii = i;
		j++;
		i++;
		if ('\0' != (cx = b[j]))
			continue;
N0:;
		if (i == ii+j)
			return ii;
		i -= j;
		j = 0;
		cx = c0;
	}

	if (i == l && cx == pdata[i] && '\0' == b[j+1])
		goto N0;
	return npos;
}

int string8::find (char c, int pos) const
{
	if (pos < 0)
		return npos;
	for(;pos < slen; pos++)
	{
		if (data[pos] == (unsigned char) c)
			return pos;
	}
	return npos;
}


#if 0
int string8::caselessfind (const string8& b, int pos) const
{
	return binstrcaseless ((bstring) this, pos, (bstring) &b);
}

int string8::caselessfind (const char * b, int pos) const
{
	struct tagbstring t;

	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::caselessfind NULL.");
#else
		return STR_ERR;
#endif
	}

	if ((unsigned int) pos > (unsigned int) slen) return STR_ERR;
	if ('\0' == b[0]) return pos;
	if (pos == slen) return STR_ERR;

	btfromcstr (t, b);
	return binstrcaseless ((bstring) this, pos, (bstring) &t);
}

int string8::reversefind (const string8& b, int pos) const {
	return binstrr ((bstring) this, pos, (bstring) &b);
}

int string8::reversefind (const char * b, int pos) const {
	struct tagbstring t;
	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::reversefind NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, b);
	return binstrr ((bstring) this, pos, &t);
}

int string8::caselessreversefind (const string8& b, int pos) const {
	return binstrrcaseless ((bstring) this, pos, (bstring) &b);
}

int string8::caselessreversefind (const char * b, int pos) const {
	struct tagbstring t;

	if (NULL == b) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::caselessreversefind NULL.");
#else
		return STR_ERR;
#endif
	}

	if ((unsigned int) pos > (unsigned int) slen) return STR_ERR;
	if ('\0' == b[0]) return pos;
	if (pos == slen) return STR_ERR;

	btfromcstr (t, b);
	return binstrrcaseless ((bstring) this, pos, (bstring) &t);
}

int string8::reversefind (char c, int pos) const {
	if (pos > slen) return STR_ERR;
	if (pos == slen) pos--;
	for (;pos >= 0; pos--) {
		if (data[pos] == (unsigned char) c) return pos;
	}
	return STR_ERR;
}

int string8::findchr (const string8& b, int pos) const {
	return binchr ((bstring) this, pos, (bstring) &b);
}

int string8::findchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::findchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return binchr ((bstring) this, pos, (bstring) &t);
}

int string8::nfindchr (const string8& b, int pos) const {
	return bninchr ((bstring) this, pos, (bstring) &b);
}

int string8::nfindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::nfindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return bninchr ((bstring) this, pos, &t);
}

int string8::reversefindchr (const string8& b, int pos) const {
	return binchrr ((bstring) this, pos, (bstring) &b);
}

int string8::reversefindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::reversefindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return binchrr ((bstring) this, pos, &t);
}

int string8::nreversefindchr (const string8& b, int pos) const {
	return bninchrr ((bstring) this, pos, (bstring) &b);
}

int string8::nreversefindchr (const char * s, int pos) const {
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("string8::nreversefindchr NULL.");
#else
		return STR_ERR;
#endif
	}
	cstr2tbstr (t, s);
	return bninchrr ((bstring) this, pos, &t);
}
#endif

int string8::find_first_of(const char * b, int pos) const
{
	assert(b);
	int count = 0;
	int temp = 0;
	int maxcount = drgString::Length(b);
	int retval = npos;
	for( ; count<maxcount; ++count)
	{
		temp = find(b[count], pos);
		if((temp!=STR_ERR) && (temp<retval))
			retval = temp;
	}
	return retval;
}

int string8::find_last_of(const char * b, int pos) const
{
	assert(b);
	int count = 0;
	int temp = 0;
	int maxcount = drgString::Length(b);
	int retval = -1;
	for( ; count<maxcount; ++count)
	{
		temp = find(b[count], pos);
		if((temp!=STR_ERR) && (temp>retval))
			retval = temp;
	}
	if(retval<0)
		retval = npos;
	return retval;
}

int string8::find_first_not_of(const char * b, int pos) const
{
	assert(b);
	int count1 = pos;
	int count2 = 0;
	bool found = false;
	int maxcount = drgString::Length(b);
	for( ; count1<slen; ++count1)
	{
		found = false;
		for(count2=0; count2<maxcount; ++count2)
		{
			if(b[count2] == data[count1])
			{
				found = true;
				break;
			}
		}
		if(found == false)
			return count1;
	}
	return npos;
}

int string8::find_last_not_of(const char * b, int pos) const
{
	assert(b);
	int count1 = slen-1;
	int count2 = 0;
	bool found = false;
	int maxcount = drgString::Length(b);
	for( ; count1>=pos; --count1)
	{
		found = false;
		for(count2=0; count2<maxcount; ++count2)
		{
			if(b[count2] == data[count1])
			{
				found = true;
				break;
			}
		}
		if(found == false)
			return count1;
	}
	return npos;
}

const string8 string8::substr(int pos, int len) const
{
	if (pos < 0)
	{
		len += pos;
		pos = 0;
	}
	if (len > slen - pos)
		len = slen - pos;
	if (len <= 0)
		return string8("");
	return string8((data + pos), len);
}

void string8::alloc(int olen)
{
	int len;
	if (data == NULL || slen < 0 || mlen <= 0 ||  mlen < slen || olen <= 0)
		drgPrintError("string8::alloc");

	if (olen >= mlen)
	{
		unsigned char * x;
		len = snapupsize(olen);
		if (len <= mlen)
			return;

		x = (unsigned char*)stringv_getmem(&len);
		if (slen)
			memcpy((char *) x, (char *) data, (size_t) slen);
		stringv_killmem(data, mlen);

		data = x;
		mlen = len;
		data[slen] = (unsigned char) '\0';
	}
}

void string8::clear()
{
	slen = 0;
	data[0] = '\0';
}


#if 0
void string8::fill(int len, unsigned char cfill)
{
	slen = 0;
	if (STR_ERR == bsetstr (this, len, NULL, cfill))
		drgPrintError("Failure in fill");
}

void string8::setsubstr (int pos, const string8& b, unsigned char cfill)
{
	if (STR_ERR == bsetstr (this, pos, (bstring) &b, cfill))
		drgPrintError("Failure in setsubstr");
}

void string8::setsubstr (int pos, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	if (NULL == s) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("setsubstr NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, s);
	if (STR_ERR == bsetstr (this, pos, &t, cfill))
		drgPrintError("Failure in setsubstr");
}

void string8::insert (int pos, const string8& b, unsigned char cfill)
{
	if (STR_ERR == binsert(this, pos, (bstring) &b, cfill))
		drgPrintError("Failure in insert");
}

void string8::insert(int pos, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	if (NULL == s)
	{
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("insert NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, s);
	if (STR_ERR == binsert (this, pos, &t, cfill))
		drgPrintError("Failure in insert");
}

void string8::insertchrs(int pos, int len, unsigned char cfill)
{
	if (STR_ERR == binsertch (this, pos, len, cfill))
		drgPrintError("Failure in insertchrs");
}

void string8::replace(int pos, int len, const string8& b, unsigned char cfill)
{
	if (STR_ERR == breplace (this, pos, len, (bstring) &b, cfill))
		drgPrintError("Failure in replace");
}

void string8::replace (int pos, int len, const char * s, unsigned char cfill)
{
	struct tagbstring t;
	size_t q;

	if (mlen <= 0) drgPrintError("Write protection error");
	if (NULL == s || (pos|len) < 0) {
		drgPrintError("Failure in replace");
	} else {
		if (pos + len >= slen) {
			cstr2tbstr (t, s);
			if (STR_ERR == bsetstr (this, pos, &t, cfill)) {
				drgPrintError("Failure in replace");
			} else if (pos + t.slen < slen) {
				slen = pos + t.slen;
				data[slen] = '\0';
			}
		} else {

			/* Aliasing case */
			if ((unsigned int) (data - (unsigned char *) s) < (unsigned int) slen) {
				replace (pos, len, string8(s), cfill);
				return;
			}

			if ((q = drgString::Length (s)) > (size_t) len || len < 0) {
				if (slen + q - len >= INT_MAX) drgPrintError("Failure in replace, result too long.");
				alloc ((int) (slen + q - len));
				if (NULL == data) return;
			}
			if ((int) q != len) bstr__memmove (data + pos + q, data + pos + len, slen - (pos + len));
			memcpy(data + pos, s, q);
			slen += ((int) q) - len;
			data[slen] = '\0';
		}
	}
}

void string8::findreplace (const string8& sfind, const string8& repl, int pos) {
	if (STR_ERR == bfindreplace (this, (bstring) &sfind, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string8::findreplace (const string8& sfind, const char * repl, int pos) {
	struct tagbstring t;
	if (NULL == repl) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, repl);
	if (STR_ERR == bfindreplace (this, (bstring) &sfind, (bstring) &t, pos)) {
		drgPrintError("Failure in findreplace");
	}
}

void string8::findreplace (const char * sfind, const string8& repl, int pos) {
	struct tagbstring t;
	if (NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	if (STR_ERR == bfindreplace (this, (bstring) &t, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplace");
	}
}
#endif
void string8::findreplace (const char * sfind, const char * repl, int pos) 
{
	if(sfind == NULL)
		return;
	
	char* found = strstr((char*)data, sfind);
	if(found == NULL)
		return;


}
#if 0
void string8::findreplace (const char * sfind, const char * repl, int pos) 
{
	struct tagbstring t, u;
	if (NULL == repl || NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplace NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	cstr2tbstr (u, repl);
	if (STR_ERR == bfindreplace (this, (bstring) &t, (bstring) &u, pos)) {
		drgPrintError("Failure in findreplace");
	}
}
void string8::findreplacecaseless (const string8& sfind, const string8& repl, int pos) {
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &sfind, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string8::findreplacecaseless (const string8& sfind, const char * repl, int pos) {
	struct tagbstring t;
	if (NULL == repl) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, repl);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &sfind, (bstring) &t, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string8::findreplacecaseless (const char * sfind, const string8& repl, int pos) {
	struct tagbstring t;
	if (NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &t, (bstring) &repl, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}

void string8::findreplacecaseless (const char * sfind, const char * repl, int pos) {
	struct tagbstring t, u;
	if (NULL == repl || NULL == sfind) {
#ifdef BSTRLIB_THROWS_EXCEPTIONS
		drgPrintError("findreplacecaseless NULL.");
#else
		return;
#endif
	}
	cstr2tbstr (t, sfind);
	cstr2tbstr (u, repl);
	if (STR_ERR == bfindreplacecaseless (this, (bstring) &t, (bstring) &u, pos)) {
		drgPrintError("Failure in findreplacecaseless");
	}
}
#endif

void string8::remove(int pos, int len)
{
	if (pos < 0) {
		len += pos;
		pos = 0;
	}
	if (len < 0 || data == NULL || slen < 0 || mlen < slen || mlen <= 0) {
		return;
	}
	if (len > 0 && pos < slen) {
		if (pos + len >= slen) {
			slen = pos;
		}
		else {
			if ((slen - (pos + len)) > 0) {
				memmove((data + pos), (data + pos + len), (slen - (pos + len)));
			}
			slen -= len;
		}
		data[slen] = (unsigned char) '\0';
	}
}

void string8::trunc(int len)
{
	if (len < 0) {
		drgPrintError("Failure in trunc");
	}
	if (len < slen) {
		slen = len;
		data[len] = '\0';
	}
}

void string8::ltrim(const char* trim)
{
	assert(trim);
	if (slen == 0) {
		return;
	}
	unsigned int rem_pos = slen-1;
	unsigned int rem_len = 0;
	while (isoneof(data[rem_pos], trim)) {
		rem_len++;
		rem_pos--;
	}
	if (rem_len > 0) {
		remove(rem_pos+1, rem_len);
	}
}

void string8::rtrim(const char* trim)
{
	assert(trim);
	unsigned int rem_len = 0;
	while (isoneof(data[rem_len], trim)) {
		rem_len++;
	}
	if (rem_len > 0) {
		remove(0, rem_len);
	}
}

void string8::caseupper()
{
	int i, len;
	if(data == NULL || mlen < slen || slen < 0 || mlen <= 0)
		return;
	for(i=0, len = slen; i < len; i++)
		data[i] = (unsigned char)toupper(data[i]);
}

void string8::caselower()
{
	int i, len;
	if(data == NULL || mlen < slen || slen < 0 || mlen <= 0)
		return;
	for(i=0, len = slen; i < len; i++)
		data[i] = (unsigned char)tolower(data[i]);
}

void string8::repeat (int count)
{
	count *= slen;
	if (count == 0)
	{
		trunc(0);
		return;
	}
	if (count < 0 || STR_ERR == pattern (count))
		drgPrintError("Failure in repeat");
}

int string8::get()
{
	if(slen<=0)
		return -1;
	int retval = data[0];
	remove(0, 1);
	return retval;
}

const string8 string8::readline(const char* terminator)
{
	int count;
	int termlen = drgString::Length(terminator);
	string8 retval;
	char getch;
	while(slen > 0)
	{
		getch = get();
		retval += getch;
		for(count=0; count<termlen; ++count)
		{
			if(terminator[count]==getch)
				break;
		}
		if(count<termlen)
			break;
	}
	return retval;
}

int string8::copy(const string8& b)
{
	int i,j;

	if (b.slen < 0 || b.data == NULL)
		return STR_ERR;

	i = b.slen;
	j = snapupsize(i + 1);

	alloc(j);
	mlen = j;
	slen = i;

	if (i)
		memcpy((char *)data, (char *) b.data, i);
	data[slen] = (unsigned char) '\0';
	return STR_OK;
}

int string8::concat(const string8& b)
{
	int len, d;

	if (data == NULL || b.data == NULL)
		return STR_ERR;

	d = slen;
	len = b.slen;
	if ((d | (mlen - d) | len | (d + len)) < 0)
		return STR_ERR;

	if (mlen <= d + len + 1)
	{
		long long pd = b.data - data;
		alloc(d + len + 1);
	}

	if (len > 0)
		memmove(&data[d], &b.data[0], len);
	data[d + len] = (unsigned char) '\0';
	slen = d + len;
	return STR_OK;
}

int string8::catcstr(const char * s)
{
	char * d;
	int i, l;

	if (data == NULL || slen < 0 || mlen < slen	|| mlen <= 0 || s == NULL)
		return STR_ERR;

	l = mlen - slen;
	d = (char *)&data[slen];
	for (i=0; i < l; i++)
	{
		if ((*d++ = *s++) == '\0')
		{
			slen += i;
			return STR_OK;
		}
	}
	slen += i;
	return catblk((const void *) s, (int) drgString::Length (s));
}

int string8::catblk(const void * s, int len)
{
	int nl;
	if (data == NULL || slen < 0 || mlen < slen || mlen <= 0 || s == NULL || len < 0)
		return STR_ERR;

	if (0 > (nl = slen + len))
		return STR_ERR;
	if (mlen <= nl)
		alloc(nl + 1);

	if (len > 0)
		memmove(&data[slen], s, len);
	slen = nl;
	data[nl] = (unsigned char) '\0';
	return STR_OK;
}

int string8::conchar(char c)
{
	int d;
	d = slen;
	if((d | (mlen - d)) < 0)
		return STR_ERR;
	alloc(d + 2);
	data[d] = (unsigned char) c;
	data[d + 1] = (unsigned char) '\0';
	slen++;
	return STR_OK;
}

int string8::iseq(const string8& b) const
{
	if (data == NULL || b.data == NULL || slen < 0 || b.slen < 0)
		return STR_ERR;
	if (slen != b.slen)
		return STR_OK;
	if (data == b.data || slen == 0)
		return 1;
	return !memcmp(data, b.data, slen);
}

int string8::iseqcstr(const char * s) const
{
	int i;
	if(s == NULL || data == NULL || slen < 0)
		return STR_ERR;
	for (i=0; i < slen; i++)
	{
		if (s[i] == '\0' || data[i] != (unsigned char) s[i])
			return STR_OK;
	}
	return s[i] == '\0';
}

int string8::compare(const string8& b) const
{
	int i, v, n;
	if(data == NULL || b.data == NULL || slen < 0 || b.slen < 0)
		return DRG_SHRT_MIN;
	n = slen; if (n > b.slen) n = b.slen;
	if (slen == b.slen && (data == b.data || slen == 0))
		return STR_OK;

	for (i = 0; i < n; i ++)
	{
		v = ((char) data[i]) - ((char) b.data[i]);
		if (v != 0)
			return v;
		if (data[i] == (unsigned char) '\0')
			return STR_OK;
	}

	if (slen > n)
		return 1;
	if (b.slen > n)
		return -1;
	return STR_OK;
}

int string8::pattern(int len)
{
	int i, d;
	d = length();
	if (d <= 0 || len < 0)
		return STR_ERR;
	alloc(len + 1);
	if (len > 0)
	{
		if (d == 1)
			return setstr(len, data[0]);
		for (i = d; i < len; i++)
			data[i] = data[i - d];
	}
	data[len] = (unsigned char) '\0';
	slen = len;
	return STR_OK;
}

int string8::setstr(int pos, unsigned char fill)
{
	int d, newlen;
	if (pos < 0 || slen < 0 || NULL == data ||  mlen < slen || mlen <= 0)
		return STR_ERR;

	d = pos;
	alloc(d + 1);
	newlen = slen;
	if (pos > newlen)
	{
		memset(data + slen, (int) fill, (size_t) (pos - slen));
		newlen = pos;
	}

	if (d > newlen)
		newlen = d;

	slen = newlen;
	data[newlen] = (unsigned char) '\0';

	return STR_OK;
}


//Sets the data in this string to 'b'.
// if 'truncate' is true, then this string will be shortened if the new string is shorter than the original string.
// 'pos' the starting position of this string at which to copy the string 'b'.  
// if 'pos' is past the end of this string, then 'fill' will be coppied into all characters between the end of this string and 'pos'
int string8::setstr(int pos, const string8& b, unsigned char fill, bool truncate)
{
	int d, newlen;
	if (pos < 0 || slen < 0 || NULL == data ||  mlen < slen || mlen <= 0)
		return STR_ERR;
	if (b.slen < 0 || b.data == NULL)
		return STR_ERR;

	d = pos;
	d += b.slen;

	alloc(d + 1);
	newlen = slen;
	if (pos > newlen)
	{
		memset(data + slen, (int) fill, (size_t) (pos - slen));
		newlen = pos;
	}

	if (b.slen > 0)
		memmove((data + pos), b.data, b.slen);

	if (truncate || d > newlen)
		newlen = d;

	slen = newlen;
	data[newlen] = (unsigned char) '\0';

	return STR_OK;
}

int string8::snapupsize(int i)
{
	if (i < 8)
	{
		i = 8;
	}
	else
	{
		unsigned int j;
		j = (unsigned int) i;
		j |= (j >>  1);
		j |= (j >>  2);
		j |= (j >>  4);
		j |= (j >>  8);		/* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xffff)
		j |= (j >> 16);		/* For 32 bit int systems */
#if (UINT_MAX > 0xffffffffUL)
		j |= (j >> 32);		/* For 64 bit int systems */
#endif
#endif
		j++;
		if ((int) j >= i)
			i = (int) j;
	}
	return i;
}


const string8 operator + (const char *a, const string8& b)
{
	return string8(a) + b;
}

const string8 operator + (const unsigned char *a, const string8& b)
{
	return string8((const char *)a) + b;
}

const string8 operator + (char c, const string8& b)
{
	return string8(c) + b;
}

const string8 operator + (unsigned char c, const string8& b)
{
	return string8(c) + b;
}

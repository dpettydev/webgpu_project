#ifndef JSON_HEADER_INCLUDED
#define JSON_HEADER_INCLUDED

#include "system/global.h"


enum JSON_TYPE
{
	JSON_TYPE_FALSE = 0,
	JSON_TYPE_TRUE = 1,
	JSON_TYPE_NULL = 2,
	JSON_TYPE_NUMBER = 3,
	JSON_TYPE_STRING = 4,
	JSON_TYPE_ARRAY = 5,
	JSON_TYPE_OBJECT = 6
};


class ValueJSON
{
public:
	ValueJSON();
	~ValueJSON();

	ValueJSON *next, *prev;	// next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem 
	ValueJSON *child;		// An array or object item will have a child pointer pointing to a chain of the items in the array/object. 

	int m_Type;				// The type of the item, as above. 
	char* m_ValueString;	// The item's string, if type==cJSON_String 
	long long m_ValueInt;	// The item's number, if type==cJSON_Number 
	double m_ValueDouble;	// The item's number, if type==cJSON_Number 
	char* m_String;			// The item's name string, if this item is the child of, or is in the list of subitems of an object. 
	bool m_UseQuotes;

	bool Parse(const char *value, int require_null_terminated);
	bool ParseFile(const char* filename);
	void SaveFile(const char* filename);

	void CleanUp();
	ValueJSON* AddNull(const char* name);
	ValueJSON* AddString(const char* name, const char* value);
	ValueJSON* AddInt(const char* name, long long value);
	ValueJSON* AddDouble(const char* name, double value);
	ValueJSON* AddObject(const char* name);
	ValueJSON* AddArray(const char* name);

	ValueJSON* AddToArray();
	ValueJSON* AddToArrayInt(long long value);
	ValueJSON* AddToArrayString(const char* value);

	ValueJSON* AddItemReferenceToArray(ValueJSON* item);
	ValueJSON* AddItemReferenceToObject(const char* name, ValueJSON* item);
	ValueJSON* AddItemDuplicateToArray(ValueJSON* item, int recurse = 0);
	ValueJSON* AddItemDuplicateToObject(const char* name, ValueJSON* item, int recurse = 0);

	void Duplicate(ValueJSON* item, int recurse = 0);

	int GetCount();
	ValueJSON* GetItem(int item);
	ValueJSON* GetItem(const char *string);

	void DeleteItem(int item);
	void DeleteItem(const char *string);

	char* CreateBody();
	bool CreateBody(char* outbuf, unsigned int* outlen, unsigned int max_len);
	bool CreateFromBody(char* obj_name, const char* inbuf);

	inline const char* GetName()
	{
		return m_String;
	}

	inline const char* GetValString()
	{
		return m_ValueString;
	}

	inline const long long GetValInt()
	{
		if (m_Type == JSON_TYPE_STRING)
			return atoll(m_ValueString);
		return m_ValueInt;
	}

	inline const double GetValDouble()
	{
		return m_ValueDouble;
	}

	inline bool IsNull()
	{
		return (m_Type == JSON_TYPE_NULL);
	}
};

#endif // JSON_HEADER_INCLUDED


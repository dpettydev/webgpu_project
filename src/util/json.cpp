
#include "util/json.h"
#include "system/file.h"
#include "util/math_def.h"

static const char *ep;

#define JSON_IS_REFERENCE 256

ValueJSON *cJSON_Parse(const char *value);
char  *cJSON_Print(ValueJSON *item);
char  *cJSON_PrintUnformatted(ValueJSON *item);
void   cJSON_Delete(ValueJSON *c);

const char *cJSON_GetErrorPtr(void);

ValueJSON *cJSON_CreateNull(void);
ValueJSON *cJSON_CreateTrue(void);
ValueJSON *cJSON_CreateFalse(void);
ValueJSON *cJSON_CreateBool(int b);
ValueJSON *cJSON_CreateNumber(double num);
ValueJSON *cJSON_CreateString(const char *string);
ValueJSON *cJSON_CreateArray(void);
ValueJSON *cJSON_CreateObject(void);

ValueJSON *cJSON_CreateIntArray(const int *numbers, int count);
ValueJSON *cJSON_CreateFloatArray(const float *numbers, int count);
ValueJSON *cJSON_CreateDoubleArray(const double *numbers, int count);
ValueJSON *cJSON_CreateStringArray(const char **strings, int count);

void cJSON_AddItemToArray(ValueJSON *array, ValueJSON *item);
void cJSON_AddItemToObject(ValueJSON *object, const char *string, ValueJSON *item);
void cJSON_AddItemReferenceToArray(ValueJSON *array, ValueJSON *item);
void cJSON_AddItemReferenceToObject(ValueJSON *object, const char *string, ValueJSON *item);

ValueJSON *cJSON_DetachItemFromArray(ValueJSON *array, int which);
void cJSON_DeleteItemFromArray(ValueJSON *array, int which);
ValueJSON *cJSON_DetachItemFromObject(ValueJSON *object, const char *string);
void cJSON_DeleteItemFromObject(ValueJSON *object, const char *string);

void cJSON_ReplaceItemInArray(ValueJSON *array, int which, ValueJSON *newitem);
void cJSON_ReplaceItemInObject(ValueJSON *object, const char *string, ValueJSON *newitem);

ValueJSON *cJSON_Duplicate(ValueJSON *item, int recurse);

ValueJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

void cJSON_Minify(char *json);

#define cJSON_AddNullToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b)	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#define cJSON_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))




const char *cJSON_GetErrorPtr(void)
{
	return ep;
}

static int cJSON_strcasecmp(const char *s1, const char *s2)
{
	if (!s1)
		return (s1 == s2) ? 0 : 1;
	if (!s2)
		return 1;
	for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
		if (*s1 == 0)
			return 0;
	return tolower(*(const unsigned char *)s1) -
		tolower(*(const unsigned char *)s2);
}

static char *cJSON_strdup(const char *str)
{
	size_t len;
	char *copy;

	len = strlen(str) + 1;
	if (!(copy = (char *)drgMemAlloc(len)))
		return 0;
	memcpy(copy, str, len);
	return copy;
}

// Internal constructor.
static ValueJSON *cJSON_New_Item(void)
{
	ValueJSON *node = (ValueJSON *)drgMemAlloc(sizeof(ValueJSON));
	if (node)
		memset(node, 0, sizeof(ValueJSON));
	return node;
}

// Delete a cJSON structure. 
void cJSON_Delete(ValueJSON *c)
{
	ValueJSON *next;
	while (c) 
	{
		next = c->next;
		if (!(c->m_Type & JSON_IS_REFERENCE) && c->child)
			cJSON_Delete(c->child);
		if (!(c->m_Type & JSON_IS_REFERENCE) && c->m_ValueString)
			drgMemFree(c->m_ValueString);
		if (c->m_String)
			drgMemFree(c->m_String);
		drgMemFree(c);
		c = next;
	}
}

// Parse the input text to generate a number, and populate the result into item.
static const char *parse_number(ValueJSON *item, const char *num)
{
	double n = 0, sign = 1, scale = 0;
	int subscale = 0, signsubscale = 1;

	if (*num == '-')
		sign = -1, num++; // Has sign? 
	if (*num == '0')
		num++; // is zero
	if (*num >= '1' && *num <= '9')
		do
	n = (n * 10.0) + (*num++ - '0');
	while (*num >= '0' && *num <= '9'); // Number? 
	if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
		num++;
		do
		n = (n * 10.0) + (*num++ - '0'), scale--;
		while (*num >= '0' && *num <= '9');
	}                               // Fractional part?
	if (*num == 'e' || *num == 'E') // Exponent?
	{
		num++;
		if (*num == '+')
			num++;
		else if (*num == '-')
			signsubscale = -1, num++; // With sign?
		while (*num >= '0' && *num <= '9')
			subscale = (subscale * 10) + (*num++ - '0'); // Number? 
	}

	n = sign * n * pow(10.0, (scale + subscale * signsubscale)); // number = +/- number.fraction * 10^+/- exponent

	item->m_ValueDouble = n;
	item->m_ValueInt = (long long)n;
	item->m_Type = JSON_TYPE_NUMBER;
	return num;
}

// Render the number nicely from the given item into a string. 
static char *print_number(ValueJSON *item)
{
	char *str;
	double d = item->m_ValueDouble;
	if (fabs(((double)item->m_ValueInt) - d) <= DRG_DBL_EPSILON && d <= DRG_MAX_INT &&
		d >= INT_MIN) {
		str = (char *)drgMemAlloc(21); // 2^64+1 can be represented in 21 chars.
		if (str)
			sprintf(str, "%" INT64_FMT "", item->m_ValueInt);
	}
	else {
		str = (char *)drgMemAlloc(64); // This is a nice tradeoff. 
		if (str) {
			if (fabs(floor(d) - d) <= DRG_DBL_EPSILON && fabs(d) < 1.0e60)
				sprintf(str, "%.0f", d);
			else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9)
				sprintf(str, "%e", d);
			else
				sprintf(str, "%f", d);
		}
	}
	return str;
}

static unsigned parse_hex4(const char *str)
{
	unsigned h = 0;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	h = h << 4;
	str++;
	if (*str >= '0' && *str <= '9')
		h += (*str) - '0';
	else if (*str >= 'A' && *str <= 'F')
		h += 10 + (*str) - 'A';
	else if (*str >= 'a' && *str <= 'f')
		h += 10 + (*str) - 'a';
	else
		return 0;
	return h;
}

// Parse the input text into an unescaped cstring, and populate item. 
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(ValueJSON *item, const char *str)
{
	const char *ptr = str + 1;
	char *ptr2;
	char *out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"') {
		ep = str;
		return 0;
	} // not a string! 

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\')
			ptr++; // Skip escaped quotes. 

	out = (char *)drgMemAlloc(
		len + 1); // This is how long we need for the string, roughly.
	if (!out)
		return 0;

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr) {
		if (*ptr != '\\')
			*ptr2++ = *ptr++;
		else {
			ptr++;
			switch (*ptr) {
			case 'b':
				*ptr2++ = '\b';
				break;
			case 'f':
				*ptr2++ = '\f';
				break;
			case 'n':
				*ptr2++ = '\n';
				break;
			case 'r':
				*ptr2++ = '\r';
				break;
			case 't':
				*ptr2++ = '\t';
				break;
			case 'u': /* transcode utf16 to utf8. */
				uc = parse_hex4(ptr + 1);
				ptr += 4; /* get the unicode char. */

				if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
					break; /* check for invalid.	*/

				if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/
				{
					if (ptr[1] != '\\' || ptr[2] != 'u')
						break; /* missing second-half of surrogate.	*/
					uc2 = parse_hex4(ptr + 3);
					ptr += 6;
					if (uc2 < 0xDC00 || uc2 > 0xDFFF)
						break; /* invalid second-half of surrogate.	*/
					uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
				}

				len = 4;
				if (uc < 0x80)
					len = 1;
				else if (uc < 0x800)
					len = 2;
				else if (uc < 0x10000)
					len = 3;
				ptr2 += len;

				switch (len) {
				case 4:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 3:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 2:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 1:
					*--ptr2 = (uc | firstByteMark[len]);
				}
				ptr2 += len;
				break;
			default:
				*ptr2++ = *ptr;
				break;
			}
			ptr++;
		}
	}
	*ptr2 = 0;
	if (*ptr == '\"')
		ptr++;
	item->m_ValueString = out;
	item->m_Type = JSON_TYPE_STRING;
	return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr(const char *str)
{
	const char *ptr;
	char *ptr2, *out;
	int len = 0;
	unsigned char token;

	if (!str)
		return cJSON_strdup("");
	ptr = str;
	while ((token = *ptr) && ++len) {
		if (strchr("\"\\\b\f\n\r\t", token))
			len++;
		else if (token < 32)
			len += 5;
		ptr++;
	}

	out = (char *)drgMemAlloc(len + 3);
	if (!out)
		return 0;

	ptr2 = out;
	ptr = str;
	*ptr2++ = '\"';
	while (*ptr) {
		if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
			*ptr2++ = *ptr++;
		else {
			*ptr2++ = '\\';
			switch (token = *ptr++) {
			case '\\':
				*ptr2++ = '\\';
				break;
			case '\"':
				*ptr2++ = '\"';
				break;
			case '\b':
				*ptr2++ = 'b';
				break;
			case '\f':
				*ptr2++ = 'f';
				break;
			case '\n':
				*ptr2++ = 'n';
				break;
			case '\r':
				*ptr2++ = 'r';
				break;
			case '\t':
				*ptr2++ = 't';
				break;
			default:
				sprintf(ptr2, "u%04x", token);
				ptr2 += 5;
				break; /* escape and print */
			}
		}
	}
	*ptr2++ = '\"';
	*ptr2++ = 0;
	return out;
}
/* Invote print_string_ptr (which is useful) on an item. */
static char *print_string(ValueJSON *item) 
{
	return print_string_ptr(item->m_ValueString);
}

/* Predeclare these prototypes. */
static const char *parse_value(ValueJSON *item, const char *value);
static char *print_value(ValueJSON *item, int depth, int fmt);
static const char *parse_array(ValueJSON *item, const char *value);
static char *print_array(ValueJSON *item, int depth, int fmt);
static const char *parse_object(ValueJSON *item, const char *value);
static char *print_object(ValueJSON *item, int depth, int fmt);

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
	while (in && *in && (unsigned char)*in <= 32)
		in++;
	return in;
}

/* Parse an object - create a new root, and populate. */
ValueJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
	const char *end = 0;
	ValueJSON *c = cJSON_New_Item();
	ep = 0;
	if (!c)
		return 0; /* memory fail */

	end = parse_value(c, skip(value));
	if (!end) {
		cJSON_Delete(c);
		return 0;
	} /* parse failure. ep is set. */

	/* if we require null-terminated JSON without appended garbage, skip and then
	 * check for a null terminator */
	if (require_null_terminated) {
		end = skip(end);
		if (*end) {
			cJSON_Delete(c);
			ep = end;
			return 0;
		}
	}
	if (return_parse_end)
		*return_parse_end = end;
	return c;
}
/* Default options for cJSON_Parse */
ValueJSON *cJSON_Parse(const char *value) 
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

/* Render a cJSON item/entity/structure to text. */
char *cJSON_Print(ValueJSON *item) { return print_value(item, 0, 1); }
char *cJSON_PrintUnformatted(ValueJSON *item)
{
	return print_value(item, 0, 0);
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(ValueJSON *item, const char *value) {
	if (!value)
		return 0; /* Fail on null. */
	if (!strncmp(value, "null", 4)) {
		item->m_Type = JSON_TYPE_NULL;
		return value + 4;
	}
	if (!strncmp(value, "false", 5)) {
		item->m_Type = JSON_TYPE_FALSE;
		return value + 5;
	}
	if (!strncmp(value, "true", 4)) {
		item->m_Type = JSON_TYPE_TRUE;
		item->m_ValueInt = 1;
		return value + 4;
	}
	if (*value == '\"') {
		return parse_string(item, value);
	}
	if (*value == '-' || (*value >= '0' && *value <= '9')) {
		return parse_number(item, value);
	}
	if (*value == '[') {
		return parse_array(item, value);
	}
	if (*value == '{') {
		return parse_object(item, value);
	}

	ep = value;
	return 0; /* failure. */
}

/* Render a value to text. */
static char *print_value(ValueJSON *item, int depth, int fmt)
{
	char *out = 0;
	if (!item)
		return 0;
	switch ((item->m_Type) & 255) {
	case JSON_TYPE_NULL:
		out = cJSON_strdup("null");
		break;
	case JSON_TYPE_FALSE:
		out = cJSON_strdup("false");
		break;
	case JSON_TYPE_TRUE:
		out = cJSON_strdup("true");
		break;
	case JSON_TYPE_NUMBER:
		out = print_number(item);
		break;
	case JSON_TYPE_STRING:
		out = print_string(item);
		break;
	case JSON_TYPE_ARRAY:
		out = print_array(item, depth, fmt);
		break;
	case JSON_TYPE_OBJECT:
		out = print_object(item, depth, fmt);
		break;
	}
	return out;
}

/* Build an array from input text. */
static const char *parse_array(ValueJSON *item, const char *value)
{
	ValueJSON *child;
	if (*value != '[') {
		ep = value;
		return 0;
	} /* not an array! */

	item->m_Type = JSON_TYPE_ARRAY;
	value = skip(value + 1);
	if (*value == ']')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0; /* memory fail */
	value = skip(
		parse_value(child, skip(value))); /* skip any spacing, get the value. */
	if (!value)
		return 0;

	while (*value == ',') {
		ValueJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_value(child, skip(value + 1)));
		if (!value)
			return 0; /* memory fail */
	}

	if (*value == ']')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Render an array to text */
static char *print_array(ValueJSON *item, int depth, int fmt)
{
	char **entries;
	char *out = 0, *ptr, *ret;
	int len = 5;
	ValueJSON *child = item->child;
	int numentries = 0, i = 0, fail = 0;

	/* How many entries in the array? */
	while (child)
		numentries++, child = child->next;
	/* Explicitly handle numentries==0 */
	if (!numentries) {
		out = (char *)drgMemAlloc(3);
		if (out)
			strcpy(out, "[]");
		return out;
	}
	/* Allocate an array to hold the values for each */
	entries = (char **)drgMemAlloc(numentries * sizeof(char *));
	if (!entries)
		return 0;
	memset(entries, 0, numentries * sizeof(char *));
	/* Retrieve all the results: */
	child = item->child;
	while (child && !fail) {
		ret = print_value(child, depth + 1, fmt);
		entries[i++] = ret;
		if (ret)
			len += strlen(ret) + 2 + (fmt ? 1 : 0);
		else
			fail = 1;
		child = child->next;
	}

	if (!fail)
		out = (char *)drgMemAlloc(len);
	if (!out)
		fail = 1;

	if (fail) {
		for (i = 0; i < numentries; i++)
			if (entries[i])
				drgMemFree(entries[i]);
		drgMemFree(entries);
		return 0;
	}

	*out = '[';
	ptr = out + 1;
	*ptr = 0;
	for (i = 0; i < numentries; i++) {
		strcpy(ptr, entries[i]);
		ptr += strlen(entries[i]);
		if (i != numentries - 1) {
			*ptr++ = ',';
			if (fmt)
				*ptr++ = ' ';
			*ptr = 0;
		}
		drgMemFree(entries[i]);
	}
	drgMemFree(entries);
	*ptr++ = ']';
	*ptr++ = 0;
	return out;
}

/* Build an object from the text. */
static const char *parse_object(ValueJSON *item, const char *value)
{
	ValueJSON *child;
	if (*value != '{') {
		ep = value;
		return 0;
	} /* not an object! */

	item->m_Type = JSON_TYPE_OBJECT;
	value = skip(value + 1);
	if (*value == '}')
		return value + 1; /* empty array. */

	item->child = child = cJSON_New_Item();
	if (!item->child)
		return 0;
	value = skip(parse_string(child, skip(value)));
	if (!value)
		return 0;
	child->m_String = child->m_ValueString;
	child->m_ValueString = 0;
	if (*value != ':') {
		ep = value;
		return 0;
	} /* fail! */
	value = skip(parse_value(
		child, skip(value + 1))); /* skip any spacing, get the value. */
	if (!value)
		return 0;

	while (*value == ',') {
		ValueJSON *new_item;
		if (!(new_item = cJSON_New_Item()))
			return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value + 1)));
		if (!value)
			return 0;
		child->m_String = child->m_ValueString;
		child->m_ValueString = 0;
		if (*value != ':') {
			ep = value;
			return 0;
		} /* fail! */
		value = skip(parse_value(
			child, skip(value + 1))); /* skip any spacing, get the value. */
		if (!value)
			return 0;
	}

	if (*value == '}')
		return value + 1; /* end of array */
	ep = value;
	return 0; /* malformed. */
}

/* Render an object to text. */
static char *print_object(ValueJSON *item, int depth, int fmt) 
{
	char **entries = 0, **names = 0;
	char *out = 0, *ptr, *ret, *str;
	int len = 7, i = 0, j;
	ValueJSON *child = item->child;
	int numentries = 0, fail = 0;
	/* Count the number of entries. */
	while (child)
		numentries++, child = child->next;
	/* Explicitly handle empty object case */
	if (!numentries) {
		out = (char *)drgMemAlloc(fmt ? depth + 4 : 3);
		if (!out)
			return 0;
		ptr = out;
		*ptr++ = '{';
		if (fmt) {
			*ptr++ = '\n';
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		}
		*ptr++ = '}';
		*ptr++ = 0;
		return out;
	}
	/* Allocate space for the names and the objects */
	entries = (char **)drgMemAlloc(numentries * sizeof(char *));
	if (!entries)
		return 0;
	names = (char **)drgMemAlloc(numentries * sizeof(char *));
	if (!names) {
		drgMemFree(entries);
		return 0;
	}
	memset(entries, 0, sizeof(char *)* numentries);
	memset(names, 0, sizeof(char *)* numentries);

	/* Collect all the results into our arrays: */
	child = item->child;
	depth++;
	if (fmt)
		len += depth;
	while (child) {
		names[i] = str = print_string_ptr(child->m_String);
		entries[i++] = ret = print_value(child, depth, fmt);
		if (str && ret)
			len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
		else
			fail = 1;
		child = child->next;
	}

	/* Try to allocate the output string */
	if (!fail)
		out = (char *)drgMemAlloc(len);
	if (!out)
		fail = 1;

	/* Handle failure */
	if (fail) {
		for (i = 0; i < numentries; i++) {
			if (names[i])
				drgMemFree(names[i]);
			if (entries[i])
				drgMemFree(entries[i]);
		}
		drgMemFree(names);
		drgMemFree(entries);
		return 0;
	}

	/* Compose the output: */
	*out = '{';
	ptr = out + 1;
	if (fmt)
		*ptr++ = '\n';
	*ptr = 0;
	for (i = 0; i < numentries; i++) {
		if (fmt)
			for (j = 0; j < depth; j++)
				*ptr++ = '\t';
		strcpy(ptr, names[i]);
		ptr += strlen(names[i]);
		*ptr++ = ':';
		if (fmt)
			*ptr++ = '\t';
		strcpy(ptr, entries[i]);
		ptr += strlen(entries[i]);
		if (i != numentries - 1)
			*ptr++ = ',';
		if (fmt)
			*ptr++ = '\n';
		*ptr = 0;
		drgMemFree(names[i]);
		drgMemFree(entries[i]);
	}

	drgMemFree(names);
	drgMemFree(entries);
	if (fmt)
		for (i = 0; i < depth - 1; i++)
			*ptr++ = '\t';
	*ptr++ = '}';
	*ptr++ = 0;
	return out;
}


/* Utility for array list handling. */
static void suffix_object(ValueJSON *prev, ValueJSON *item)
{
	prev->next = item;
	item->prev = prev;
}
/* Utility for handling references. */
static ValueJSON *create_reference(ValueJSON *item)
{
	ValueJSON *ref = cJSON_New_Item();
	if (!ref)
		return 0;
	memcpy(ref, item, sizeof(ValueJSON));
	ref->m_String = 0;
	ref->m_Type |= JSON_IS_REFERENCE;
	ref->next = ref->prev = 0;
	return ref;
}

/* Add item to array/object. */
void cJSON_AddItemToArray(ValueJSON *array, ValueJSON *item)
{
	ValueJSON *c = array->child;
	if (!item)
		return;
	if (!c) {
		array->child = item;
	}
	else {
		while (c && c->next)
			c = c->next;
		suffix_object(c, item);
	}
}

void cJSON_AddItemToObject(ValueJSON *object, const char *string, ValueJSON *item)
{
	if (!item)
		return;
	if (item->m_String)
		drgMemFree(item->m_String);
	item->m_String = cJSON_strdup(string);
	cJSON_AddItemToArray(object, item);
}

void cJSON_AddItemReferenceToArray(ValueJSON *array, ValueJSON *item)
{
	cJSON_AddItemToArray(array, create_reference(item));
}

void cJSON_AddItemReferenceToObject(ValueJSON *object, const char *string,
	ValueJSON *item) {
	cJSON_AddItemToObject(object, string, create_reference(item));
}

ValueJSON *cJSON_DetachItemFromArray(ValueJSON *array, int which) {
	ValueJSON *c = array->child;
	while (c && which > 0)
		c = c->next, which--;
	if (!c)
		return 0;
	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;
	if (c == array->child)
		array->child = c->next;
	c->prev = c->next = 0;
	return c;
}
void cJSON_DeleteItemFromArray(ValueJSON *array, int which)
{
	cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

ValueJSON *cJSON_DetachItemFromObject(ValueJSON *object, const char *string)
{
	int i = 0;
	ValueJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->m_String, string))
		i++, c = c->next;
	if (c)
		return cJSON_DetachItemFromArray(object, i);
	return 0;
}

void cJSON_DeleteItemFromObject(ValueJSON *object, const char *string)
{
	cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

/* Replace array/object items with new ones. */
void cJSON_ReplaceItemInArray(ValueJSON *array, int which, ValueJSON *newitem)
{
	ValueJSON *c = array->child;
	while (c && which > 0)
		c = c->next, which--;
	if (!c)
		return;
	newitem->next = c->next;
	newitem->prev = c->prev;
	if (newitem->next)
		newitem->next->prev = newitem;
	if (c == array->child)
		array->child = newitem;
	else
		newitem->prev->next = newitem;
	c->next = c->prev = 0;
	cJSON_Delete(c);
}

void cJSON_ReplaceItemInObject(ValueJSON *object, const char *string, ValueJSON *newitem)
{
	int i = 0;
	ValueJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->m_String, string))
		i++, c = c->next;
	if (c) {
		newitem->m_String = cJSON_strdup(string);
		cJSON_ReplaceItemInArray(object, i, newitem);
	}
}

/* Create basic types: */
ValueJSON *cJSON_CreateNull(void)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = JSON_TYPE_NULL;
	return item;
}

ValueJSON *cJSON_CreateTrue(void)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = JSON_TYPE_TRUE;
	return item;
}

ValueJSON *cJSON_CreateFalse(void) 
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = JSON_TYPE_FALSE;
	return item;
}

ValueJSON *cJSON_CreateBool(int b) 
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = b ? JSON_TYPE_TRUE : JSON_TYPE_FALSE;
	return item;
}

ValueJSON *cJSON_CreateNumber(double num)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_NUMBER;
		item->m_ValueDouble = num;
		item->m_ValueInt = (int)num;
	}
	return item;
}

ValueJSON *cJSON_CreateString(const char *string)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_STRING;
		item->m_ValueString = cJSON_strdup(string);
	}
	return item;
}

ValueJSON *cJSON_CreateArray(void)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = JSON_TYPE_ARRAY;
	return item;
}

ValueJSON *cJSON_CreateObject(void)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
		item->m_Type = JSON_TYPE_OBJECT;
	return item;
}

/* Create Arrays: */
ValueJSON *cJSON_CreateIntArray(const int *numbers, int count)
{
	int i;
	ValueJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++) {
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}

ValueJSON *cJSON_CreateFloatArray(const float *numbers, int count)
{
	int i;
	ValueJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}

ValueJSON *cJSON_CreateDoubleArray(const double *numbers, int count)
{
	int i;
	ValueJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}

ValueJSON *cJSON_CreateStringArray(const char **strings, int count)
{
	int i;
	ValueJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
	for (i = 0; a && i < count; i++)
	{
		n = cJSON_CreateString(strings[i]);
		if (!i)
			a->child = n;
		else
			suffix_object(p, n);
		p = n;
	}
	return a;
}

/* Duplication */
ValueJSON *cJSON_Duplicate(ValueJSON *item, int recurse) 
{
	ValueJSON *newitem, *cptr, *nptr = 0, *newchild;
	/* Bail on bad ptr */
	if (!item)
		return 0;
	/* Create new item */
	newitem = cJSON_New_Item();
	if (!newitem)
		return 0;
	/* Copy over all vars */
	newitem->m_Type = item->m_Type & (~JSON_IS_REFERENCE),
		newitem->m_ValueInt = item->m_ValueInt, newitem->m_ValueDouble = item->m_ValueDouble;
	if (item->m_ValueString)
	{
		newitem->m_ValueString = cJSON_strdup(item->m_ValueString);
		if (!newitem->m_ValueString)
		{
			cJSON_Delete(newitem);
			return 0;
		}
	}
	if (item->m_String) {
		newitem->m_String = cJSON_strdup(item->m_String);
		if (!newitem->m_String) {
			cJSON_Delete(newitem);
			return 0;
		}
	}
	/* If non-recursive, then we're done! */
	if (!recurse)
		return newitem;
	/* Walk the ->next chain for the child. */
	cptr = item->child;
	while (cptr) {
		newchild = cJSON_Duplicate(cptr, 1); /* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild) {
			cJSON_Delete(newitem);
			return 0;
		}
		if (nptr) {
			nptr->next = newchild, newchild->prev = nptr;
			nptr = newchild;
		} /* If newitem->child already set, then crosswire ->prev and ->next and
			 move on */
		else {
			newitem->child = newchild;
			nptr = newchild;
		} /* Set newitem->child and move to it */
		cptr = cptr->next;
	}
	return newitem;
}

void cJSON_Minify(char *json) 
{
	char *into = json;
	while (*json) {
		if (*json == ' ')
			json++;
		else if (*json == '\t')
			json++; // Whitespace characters.
		else if (*json == '\r')
			json++;
		else if (*json == '\n')
			json++;
		else if (*json == '/' && json[1] == '/')
		while (*json && *json != '\n')
			json++; // double-slash comments, to end of line.
		else if (*json == '/' && json[1] == '*') {
			while (*json && !(*json == '*' && json[1] == '/'))
				json++;
			json += 2;
		} // multiline comments.
		else if (*json == '\"') {
			*into++ = *json++;
			while (*json && *json != '\"') {
				if (*json == '\\')
					*into++ = *json++;
				*into++ = *json++;
			}
			*into++ = *json++;
		} // string literals, which are \" sensitive.
		else
			*into++ = *json++; // All other characters.
	}
	*into = 0; // and null-terminate.
}

ValueJSON::ValueJSON() 
{
	m_Type = JSON_TYPE_OBJECT;
	next = NULL;
	prev = NULL;
	child = NULL;
	m_ValueString = NULL;
	m_ValueInt = 0;
	m_ValueDouble = 0.0;
	m_String = NULL;
}

ValueJSON::~ValueJSON()
{
	CleanUp();
}

void ValueJSON::CleanUp()
{
	if (!(m_Type & JSON_IS_REFERENCE) && child)
		cJSON_Delete(child);
	if (!(m_Type & JSON_IS_REFERENCE) && m_ValueString)
		drgMemFree(m_ValueString);
	if (m_String)
		drgMemFree(m_String);
	cJSON_Delete(next);
	next = NULL;
	prev = NULL;
	child = NULL;
	m_ValueString = NULL;
	m_ValueInt = 0;
	m_ValueDouble = 0.0;
	m_String = NULL;
	m_Type = JSON_TYPE_OBJECT;
}

ValueJSON* ValueJSON::AddNull(const char* name)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
	{
		item->m_Type = JSON_TYPE_NULL;
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddString(const char *name, const char *value)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
	{
		item->m_Type = JSON_TYPE_STRING;
		if (value == NULL)
			item->m_ValueString = cJSON_strdup("");
		else
			item->m_ValueString = cJSON_strdup(value);
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddInt(const char* name, long long value)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_NUMBER;
		item->m_ValueDouble = (double)value;
		item->m_ValueInt = value;
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddDouble(const char* name, double value)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_NUMBER;
		item->m_ValueDouble = value;
		item->m_ValueInt = (long long)value;
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddObject(const char* name)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
	{
		item->m_Type = JSON_TYPE_OBJECT;
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddArray(const char *name)
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
	{
		item->m_Type = JSON_TYPE_ARRAY;
		cJSON_AddItemToObject(this, name, item);
	}
	return item;
}

ValueJSON *ValueJSON::AddToArray()
{
	ValueJSON *item = cJSON_New_Item();
	if (item)
	{
		item->m_Type = JSON_TYPE_OBJECT;
		cJSON_AddItemToArray(this, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddToArrayInt(long long value)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_NUMBER;
		item->m_ValueDouble = (double)value;
		item->m_ValueInt = value;
		cJSON_AddItemToArray(this, item);
	}
	return item;
}

ValueJSON* ValueJSON::AddToArrayString(const char* value)
{
	ValueJSON *item = cJSON_New_Item();
	if (item) {
		item->m_Type = JSON_TYPE_STRING;
		if (value == NULL)
			item->m_ValueString = cJSON_strdup("");
		else
			item->m_ValueString = cJSON_strdup(value);
		cJSON_AddItemToArray(this, item);
	}
	return item;
}

ValueJSON*  ValueJSON::AddItemReferenceToArray(ValueJSON *item)
{
	ValueJSON* ref = create_reference(item);
	cJSON_AddItemToArray(this, ref);
	return ref;
}

ValueJSON*  ValueJSON::AddItemReferenceToObject(const char *name, ValueJSON *item)
{
	ValueJSON* ref = create_reference(item);
	cJSON_AddItemToObject(this, name, ref);
	return ref;
}

ValueJSON*  ValueJSON::AddItemDuplicateToArray(ValueJSON *item, int recurse)
{
	ValueJSON* dup = cJSON_Duplicate(item, recurse);
	cJSON_AddItemToArray(this, dup);
	return dup;
}

ValueJSON*  ValueJSON::AddItemDuplicateToObject(const char *name, ValueJSON *item, int recurse)
{
	ValueJSON* dup = cJSON_Duplicate(item, recurse);
	cJSON_AddItemToObject(this, name, dup);
	return dup;
}

void ValueJSON::Duplicate(ValueJSON* item, int recurse)
{
	CleanUp();
	ValueJSON* dup = cJSON_Duplicate(item, recurse);

	next = dup->next;
	prev = dup->prev;
	child = dup->child;
	m_ValueString = dup->m_ValueString;
	m_ValueInt = dup->m_ValueInt;
	m_ValueDouble = dup->m_ValueDouble;
	m_String = dup->m_String;
	m_Type = dup->m_Type;
	m_UseQuotes = dup->m_UseQuotes;

	dup->next = NULL;
	dup->prev = NULL;
	dup->child = NULL;
	dup->m_ValueString = NULL;
	dup->m_ValueInt = 0;
	dup->m_ValueDouble = 0.0;
	dup->m_String = NULL;
	cJSON_Delete(dup);
}

int ValueJSON::GetCount()
{
	ValueJSON *c = child;
	int i = 0;
	while (c)
	{
		i++;
		c = c->next;
	}
	return i;
}

ValueJSON* ValueJSON::GetItem(int item)
{
	ValueJSON *c = child;
	while (c && item > 0)
	{
		item--;
		c = c->next;
	}
	return c;
}

ValueJSON* ValueJSON::GetItem(const char *string)
{
	ValueJSON *c = child;
	while (c && cJSON_strcasecmp(c->m_String, string))
		c = c->next;
	return c;
}

void ValueJSON::DeleteItem(int item)
{
	cJSON_DeleteItemFromArray(this, item);
}

void ValueJSON::DeleteItem(const char *string)
{
	cJSON_DeleteItemFromObject(this, string);
}

char* ValueJSON::CreateBody()
{
	return cJSON_Print(this);
}

bool ValueJSON::CreateBody(char *outbuf, unsigned int *outlen, unsigned int max_len) 
{
	//char *printed = cJSON_PrintUnformatted(this);
	char* printed = cJSON_Print(this);
	strcpy(outbuf, printed);
	(*outlen) = strlen(outbuf);
	drgMemFree(printed);
	return true;
}

bool ValueJSON::Parse(const char *value, int require_null_terminated)
{
	const char *end = 0;
	ep = 0;

	end = parse_value(this, skip(value));
	if (!end)
	{
		CleanUp();
		return false;
	} // parse failure. ep is set. 

	// if we require null-terminated JSON without appended garbage, skip and then check for a null terminator 
	if (require_null_terminated)
	{
		end = skip(end);
		if (*end)
		{
			ep = end;
			CleanUp();
			return false;
		}
	}
	return true;
}

bool ValueJSON::ParseFile(const char* filename)
{
	unsigned int length = 0;
	char* json_st = (char*)drgFile::MallocFile(filename, &length);
	CleanUp();
	if (json_st == NULL)
		return false;
	return Parse(json_st, 1);
}

void ValueJSON::SaveFile(const char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if (fp == NULL)
		return;
	char body[4096] = "";
	unsigned int len = 0;
	CreateBody(body, &len, 4096);
	fwrite(body, 1, len, fp);
	fclose(fp);
}
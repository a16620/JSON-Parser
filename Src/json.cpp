#include "json.h"
#include <stdexcept>
#include <string.h>
using namespace namespace_json_;
using namespace std;

inline bool CompareFloats(const double& x, const double& y)
{
	return abs(x - y) <= CompareError;
}

JSONArray::JSONArray() : JSONValue(VALUE_TYPE::ARRAY)
{
}

JSONArray::~JSONArray()
{
	for (auto e : elements) {
		delete e;
	}
}

JSONValue* JSONArray::At(const size_t& i)
{
	//Out of range
	return elements.at(i);
}

void JSONArray::Append(JSONValue* item)
{
	elements.push_back(item->Clone());
}

void JSONArray::Remove(JSONValue* item)
{
	for (auto it = elements.begin(); it != elements.end();)
	{
		if ((*it)->Equal(item)) {
			delete* it;
			elements.erase(++it);
		}
		else
			++it;
	}
}

void JSONArray::Clear()
{
	elements.clear();
}

size_t JSONArray::Count() const
{
	return elements.size();
}

bool JSONArray::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;
	auto O = reinterpret_cast<JSONArray*>(o);
	const size_t length = Count();
	if (length != O->Count())
		return false;

	for (size_t i = 0; i < length; ++i) {
		if (!elements[i]->Equal(O->elements[i]))
			return false;
	}
	return true;
}

JSONValue* JSONArray::Clone() const
{
	JSONArray* array = new JSONArray();
	for (auto e : elements) {
		array->Append(e);
	}
	return array;
}

JSONValue::JSONValue(VALUE_TYPE type) noexcept : type(type)
{
}

/*bool namespace_json_::JSONValue::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;
}*/

JSONString::JSONString(const char* str) : JSONValue(VALUE_TYPE::STRING)
{
	if (str == nullptr)
		throw std::runtime_error("문자열이 NULL을 가르킵니다");
	length = strlen(str);
	char* szCopied = new char[length+1];
	if (strcpy_s(szCopied, length+1, str) != 0) {
		delete[] szCopied;
		throw std::runtime_error("문자열 복사 실패");
	}
	szCopied[length] = '\0';
	szStr = szCopied;
}

JSONString::JSONString(const JSONString& o) : JSONValue(VALUE_TYPE::STRING)
{
	length = o.length;
	char* szCopied = new char[length + 1];
	if (strcpy_s(szCopied, length+1, o.szStr) != 0) {
		delete[] szCopied;
		throw std::runtime_error("문자열 복사 실패");
	}
	szCopied[length] = '\0';
	szStr = szCopied;
}

JSONString::~JSONString()
{
	delete[] szStr;
}

void JSONString::Put(const char* str)
{

	if (str == nullptr)
		throw std::runtime_error("문자열이 NULL을 가르킵니다");
	length = strlen(str);
	char* szCopied = new char[length + 1];
	if (strcpy_s(szCopied, length, str) != 0) {
		delete[] szCopied;
		throw std::runtime_error("문자열 복사 실패");
	}
	szCopied[length] = '\0';
	delete[] szStr;
	szStr = szCopied;
}

const char* JSONString::Get() const
{
	return szStr;
}

size_t JSONString::Length() const
{
	return length;
}

bool JSONString::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;

	auto O = reinterpret_cast<JSONString*>(o);
	if (length != O->length)
		return false;
	return strcmp(szStr, O->szStr) == 0;
}

JSONValue* JSONString::Clone() const
{
	return new JSONString(*this);
}

JSONBoolean::JSONBoolean(const bool& v) noexcept : JSONValue(VALUE_TYPE::BOOLEAN)
{
	value = v;
}

JSONBoolean::JSONBoolean(const JSONBoolean& o) noexcept : JSONValue(VALUE_TYPE::BOOLEAN)
{
	value = o.value;
}

bool JSONBoolean::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;

	return value == reinterpret_cast<JSONBoolean*>(o)->value;
}

JSONValue* JSONBoolean::Clone() const
{
	return new JSONBoolean(*this);
}

JSONNull::JSONNull() noexcept : JSONValue(VALUE_TYPE::JNULL)
{
}

bool JSONNull::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type == o->type)
		return true;

	return false;
}

JSONValue* JSONNull::Clone() const
{
	return new JSONNull();
}

JSONNumber::JSONNumber(const double& v) noexcept : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = true;
	fVal = v;
}

JSONNumber::JSONNumber(const int& v) noexcept : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = false;
	iVal = v;
}

JSONNumber::JSONNumber(const JSONNumber& o) noexcept : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = o.isFloating;
	if (isFloating) {
		fVal = o.fVal;
	}
	else {
		iVal = o.iVal;
	}
}

void JSONNumber::Put(const double& v) noexcept
{
	isFloating = true;
	fVal = v;
}

void JSONNumber::Put(const int& v) noexcept
{
	isFloating = false;
	iVal = v;
}

bool JSONNumber::IsFloating() const noexcept
{
	return isFloating;
}

double JSONNumber::GetAsFloat() const
{
	if (isFloating)
		return fVal;
	return static_cast<double>(iVal);
}

int JSONNumber::GetAsInt() const
{
	if (isFloating)
		return static_cast<int>(fVal);
	return iVal;
}

bool JSONNumber::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;

	return CompareFloats(GetAsFloat(), reinterpret_cast<JSONNumber*>(o)->GetAsFloat());
}

JSONValue* JSONNumber::Clone() const
{
	return new JSONNumber(*this);
}

JSONObject::JSONObject() : JSONValue(VALUE_TYPE::OBJECT)
{
}

JSONObject::JSONObject(const JSONObject& o) : JSONValue(VALUE_TYPE::OBJECT)
{
	for (auto it : o.properties) {
		it.second = it.second->Clone();
		properties.insert(it);
	}
}

JSONObject::~JSONObject()
{
	for (auto it : properties) {
		delete it.second;
	}
}

void namespace_json_::JSONObject::Put(const std::string& key, JSONValue* v)
{
	auto it = properties.find(key);
	if (it != properties.end())
	{
		delete it->second;
	}
	properties.insert(make_pair(key, v));
}

bool JSONObject::Has(const std::string& key) const
{
	return properties.find(key) != properties.end();
}

JSONValue* JSONObject::Get(const std::string& key) const
{
	auto it = properties.find(key);
	if (it == properties.end())
		return nullptr;
	return it->second;
}

bool JSONObject::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;
	
	auto O = reinterpret_cast<JSONObject*>(o);
	if (properties.size() != O->properties.size())
		return false;
	//TODO : Compare key, value

	return false;
}

JSONValue* JSONObject::Clone() const
{
	return new JSONObject(*this);
}

string ParseKey(char* buffer, char*& next)
{
	char* end = std::strchr(buffer, '"');
	if (end == NULL) //TODO
		throw runtime_error("Can't find end of string");
	auto ofx = end - buffer;
	next = end + 1;
	return string(buffer, ofx);
}

char* SkipSpaces(char* buffer)
{
	if (buffer == NULL || nullptr)
		return NULL;
	while (*buffer != '\0')
	{
		switch (*buffer)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			buffer++;
			break;
		default:
			return buffer;
		}
	}
	return NULL;
}

JSONObject* namespace_json_::ParseObject(char* buffer, char*& next)
{
	auto const origin = buffer;
	auto obj = new JSONObject();
	bool parseKey = true;
	string key;
	buffer = SkipSpaces(buffer);
	if (*buffer != '{') {
		delete obj;
		throw runtime_error("No Start");
	}
	buffer++;
	while ((*buffer != '}' && *buffer != '\0'))
	{
		if (parseKey)
		{
			if (*buffer == '\"')
			{
				key = ParseKey(buffer + 1, buffer);
				buffer = std::strchr(buffer, ':');
				buffer = SkipSpaces(buffer);
				if (buffer == NULL) {
					delete obj;
					throw runtime_error("Can't find end of token");
				}
				if (*buffer != ':') {
					delete obj;
					throw runtime_error("Can't find end of key");
				}
				buffer = SkipSpaces(++buffer);
				parseKey = false;
			}
			else
			{
				buffer = SkipSpaces(buffer);
			}
		}
		else {
			try {
				if (*buffer == '{') {
					auto v = ParseObject(buffer, buffer);
					obj->Put(key, v);
				}
				else if (*buffer == '"') {
					auto v = ParseString(buffer + 1, buffer);
					obj->Put(key, v);
				}
				else if (isdigit(*buffer)) {
					auto v = ParseNumber(buffer, buffer);
					obj->Put(key, v);
				}
				else if (*buffer == '[') {
					auto v = ParseArray(buffer, buffer);
					obj->Put(key, v);
				}
				else if (isalpha(*buffer))
				{
					auto v = ParseBN(buffer, buffer);
					obj->Put(key, v);
				}
				else
				{
					throw runtime_error("Uncompleted Object");
				}
			}
			catch (exception e)
			{
				delete obj;
				throw e;
			}

			buffer = SkipSpaces(buffer);
			if (*buffer == ',') {
				buffer++;
				parseKey = true;
				buffer = SkipSpaces(buffer);
			}
			else {
				break;
			}
		}
	}
	next = buffer+1;
	return obj;
}

JSONString* namespace_json_::ParseString(char* buffer, char*& next)
{
	//char* end = std::strchr(buffer, '"');
	//if (end == NULL) //TODO
	//	throw runtime_error("Can't find end of string");
	//auto ofx = end - buffer;
	//string str(buffer, ofx);
	//next = end+1;
	auto str = ParseKey(buffer, next);
	return new JSONString(str.c_str());
}

JSONNumber* namespace_json_::ParseNumber(char* buffer, char*& next)
{
	string number;
	while (*buffer != '\0')
	{
		if (!(isdigit(*buffer) || *buffer == '.'))
			break;
		number.push_back(*(buffer++));
	}

	auto fi = number.find_first_of('.');
	
	if (fi != string::npos) {
		if (fi != number.find_last_of('.')) // having two points
			throw runtime_error("Unknown number format");
		try {
			double val = std::stof(number);
			next = buffer;
			return new JSONNumber(val);
		}
		catch (std::exception e) {
			throw runtime_error("Unknown number format");
		}
	}
	else
	{
		try {
			int val = std::stoi(number);
			next = buffer;
			return new JSONNumber(val);
		}
		catch (std::exception e) {
			throw runtime_error("Unknown number format");
		}
	}
}

JSONArray* namespace_json_::ParseArray(char* buffer, char*& next)
{
	auto obj = new JSONArray();
	buffer = SkipSpaces(buffer + 1);
	while (*buffer != ']' && *buffer != '\0')
	{
		try {
			if (*buffer == '{') {
				auto v = ParseObject(buffer, buffer);
				obj->Append(v);
			}
			else if (*buffer == '"') {
				auto v = ParseString(buffer + 1, buffer);
				obj->Append(v);
			}
			else if (isdigit(*buffer)) {
				auto v = ParseNumber(buffer, buffer);
				obj->Append(v);
			}
			else if (*buffer == '[') {
				auto v = ParseArray(buffer, buffer);
				obj->Append(v);
			}
			else if (isalpha(*buffer))
			{
				auto v = ParseBN(buffer, buffer);
				obj->Append(v);
			}
			else
			{
				throw runtime_error("Uncompleted Array");
			}
		}
		catch (exception e)
		{
			delete obj;
			throw e;
		}

		buffer = SkipSpaces(buffer);
		if (*buffer == ',') {
			buffer++;
			buffer = SkipSpaces(buffer);
		}

	}
	
	next = buffer;
	return obj;
}

JSONValue* namespace_json_::ParseBN(char* buffer, char*& next)
{
	JSONValue* v = nullptr;
	if (std::strncmp(buffer, "true", 4) == 0)
	{
		v = new JSONBoolean(true);
		next = buffer + 4;
	}
	else if (std::strncmp(buffer, "false", 5) == 0)
	{
		v = new JSONBoolean(false);
		next = buffer + 5;
	}
	else if (std::strncmp(buffer, "null", 4) == 0)
	{
		v = new JSONNull();
		next = buffer + 4;
	}
	else
	{
		throw runtime_error("Unknown token");
	}
	return v;
}


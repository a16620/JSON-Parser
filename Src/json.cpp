#include "src.h"
#include <stdexcept>
using namespace namespace_json_;

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

JSONValue::JSONValue(VALUE_TYPE type) : type(type)
{
}

/*bool namespace_json_::JSONValue::Equal(JSONValue* o) const
{
	if (this == o)
		return true;
	if (type != o->type)
		return false;
}*/

JSONString::JSONString(const TCHAR* str) : JSONValue(VALUE_TYPE::STRING)
{
	if (str == nullptr)
		throw std::runtime_error("문자열이 NULL을 가르킵니다");
	length = _tcslen(str);
	TCHAR* szCopied = new TCHAR[length+1];
	if (_tcscpy_s(szCopied, length, str) != 0) {
		delete[] szCopied;
		throw std::runtime_error("문자열 복사 실패");
	}
	szCopied[length] = '\0';
	szStr = szCopied;
}

JSONString::JSONString(const JSONString& o) : JSONValue(VALUE_TYPE::STRING)
{
	length = o.length;
	TCHAR* szCopied = new TCHAR[length + 1];
	if (_tcscpy_s(szCopied, length, o.szStr) != 0) {
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

void JSONString::Put(const TCHAR* str)
{

	if (str == nullptr)
		throw std::runtime_error("문자열이 NULL을 가르킵니다");
	length = _tcslen(str);
	TCHAR* szCopied = new TCHAR[length + 1];
	if (_tcscpy_s(szCopied, length, str) != 0) {
		delete[] szCopied;
		throw std::runtime_error("문자열 복사 실패");
	}
	szCopied[length] = '\0';
	delete[] szStr;
	szStr = szCopied;
}

const TCHAR* JSONString::Get() const
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
	return _tcscmp(szStr, O->szStr) == 0;
}

JSONValue* JSONString::Clone() const
{
	return new JSONString(*this);
}

JSONBoolean::JSONBoolean(const bool& v) : JSONValue(VALUE_TYPE::BOOLEAN)
{
	value = v;
}

JSONBoolean::JSONBoolean(const JSONBoolean& o) : JSONValue(VALUE_TYPE::BOOLEAN)
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

JSONNull::JSONNull() : JSONValue(VALUE_TYPE::JNULL)
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

JSONNumber::JSONNumber(const double& v) : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = true;
	fVal = v;
}

JSONNumber::JSONNumber(const int& v) : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = false;
	iVal = v;
}

JSONNumber::JSONNumber(const JSONNumber& o) : JSONValue(VALUE_TYPE::NUMBER)
{
	isFloating = o.isFloating;
	if (isFloating) {
		fVal = o.fVal;
	}
	else {
		iVal = o.iVal;
	}
}

void JSONNumber::Put(const double& v)
{
	isFloating = true;
	fVal = v;
}

void JSONNumber::Put(const int& v)
{
	isFloating = false;
	iVal = v;
}

bool JSONNumber::IsFloating() const
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

bool JSONObject::Has(const std::string& key)
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

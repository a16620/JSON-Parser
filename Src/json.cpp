#include "git_json.h"
#include <stdexcept>
#include <string.h>
#include <sstream>
#include <algorithm>
using namespace namespace_json_;
using namespace std;

inline bool CompareFloats(const double& x, const double& y)
{
	return abs(x - y) <= CompareError;
}

JSONValue::JSONValue(VALUE_TYPE type) noexcept : type(type)
{
}

JSONString::JSONString(const char* str) : JSONValue(VALUE_TYPE::STRING), std::string(str)
{
}

JSONString::JSONString(const JSONString& o) : JSONValue(VALUE_TYPE::STRING), std::string(o)
{
}

namespace_json_::JSONString::~JSONString()
{
}

void namespace_json_::JSONString::Put(const char* str)
{
	*this = str;
}

std::string namespace_json_::JSONString::Repr() const
{
	return std::string();
}

bool namespace_json_::JSONString::Equal(JSONValue* o) const
{
	if (o == this) {
		return true;
	}
	else if (o->type != this->type) {
		return false;
	}

	return *static_cast<JSONString*>(o) == *this; //std::string compare
}

JSONValue* namespace_json_::JSONString::Clone() const
{
	return new JSONString(*this);
}

JSONBoolean::JSONBoolean(const bool& v) noexcept : JSONValue(VALUE_TYPE::BOOLEAN), value{ v }
{
}

JSONBoolean::JSONBoolean(const JSONBoolean& o) noexcept : JSONValue(VALUE_TYPE::BOOLEAN), value{o.value}
{
}

std::string namespace_json_::JSONBoolean::Repr() const
{
	return value ? "true" : "false";
}

bool namespace_json_::JSONBoolean::Equal(JSONValue* o) const
{
	if (o == this) {
		return true;
	}
	else if (o->type != this->type) {
		return false;
	}

	return value == static_cast<JSONBoolean*>(o)->value;
}

JSONValue* namespace_json_::JSONBoolean::Clone() const
{
	return new JSONBoolean(*this);
}

namespace_json_::JSONNull::JSONNull() noexcept : JSONValue(VALUE_TYPE::JNULL)
{
}

std::string namespace_json_::JSONNull::Repr() const
{
	return "null";
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

JSONNumber::JSONNumber(const JSONNumber& o) noexcept : JSONValue(VALUE_TYPE::NUMBER), isFloating{o.isFloating}
{
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

std::string namespace_json_::JSONNumber::Repr() const
{
	return isFloating ? std::to_string(fVal) : std::to_string(iVal);
}

bool namespace_json_::JSONNumber::Equal(JSONValue* o) const
{
	if (o == this) {
		return true;
	}
	else if (o->type != this->type) {
		return false;
	}

	auto n = reinterpret_cast<JSONNumber*>(o);
	if (n->isFloating || isFloating) {
		return CompareFloats(n->GetAsFloat(), GetAsFloat());
	}

	return n->iVal == iVal;
}

JSONValue* namespace_json_::JSONNumber::Clone() const
{
	return new JSONNumber(*this);
}

JSONObject::JSONObject() : JSONValue(VALUE_TYPE::OBJECT)
{
}

JSONObject::JSONObject(const JSONObject& o) : JSONValue(VALUE_TYPE::OBJECT)
{
	for (auto it : o) {
		auto pair = make_pair(it.first, it.second->Clone());
		insert(pair);
	}
}

JSONObject::~JSONObject()
{
	for (auto it : *this) {
		delete it.second;
	}
}

void namespace_json_::JSONObject::Put(const std::string& key, JSONValue* v)
{
	auto it = find(key);
	if (it == end())
	{
		insert(std::make_pair(key, v));
	}
	else {
		delete it->second;
		it->second = v;
	}
}

bool namespace_json_::JSONObject::Has(const std::string& key) const
{
	return count(key);
}

std::string namespace_json_::JSONObject::Repr() const
{
	if (empty()) {
		return "{}";
	}
	const std::string sep = ", ";
	std::ostringstream rep;
	auto it = cbegin(), end = this->cend();
	rep << '{' << (it->first) << ':' << it->second->Repr(); //escape 필요
	for (; it != end; ++it) {
		rep << sep << (it->first) << ':' << it->second->Repr(); //escape 필요
	}
	rep << '}';
	return rep.str();
}

bool namespace_json_::JSONObject::Equal(JSONValue* o) const
{
	if (o == this) {
		return true;
	}
	else if (o->type != this->type) {
		return false;
	}

	auto O = static_cast<JSONObject*>(o);
	if (O->size() != size()) {
		return false;
	}

	const auto compare_pair = [](value_type const& v1, value_type const& v2) {
		return v1.first == v2.first && v1.second->Equal(v2.second);
	};
	return std::equal(begin(), end(), O->begin(), compare_pair);
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
	next = buffer + 1;
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
				obj->push_back(v);
			}
			else if (*buffer == '"') {
				auto v = ParseString(buffer + 1, buffer);
				obj->push_back(v);
			}
			else if (isdigit(*buffer)) {
				auto v = ParseNumber(buffer, buffer);
				obj->push_back(v);
			}
			else if (*buffer == '[') {
				auto v = ParseArray(buffer, buffer);
				obj->push_back(v);
			}
			else if (isalpha(*buffer))
			{
				auto v = ParseBN(buffer, buffer);
				obj->push_back(v);
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

	next = buffer + 1;
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

JSONArray::JSONArray() : JSONValue(VALUE_TYPE::ARRAY)
{
}

namespace_json_::JSONArray::~JSONArray()
{
	for (auto e : *this)
	{
		delete e;
	}
}

void namespace_json_::JSONArray::Remove(JSONValue* item)
{
	auto it = find(begin(), end(), item);
	if (it != end())
	{
		erase(it);
	}
}

void JSONArray::Remove(const JSONValue& value)
{
	auto cmp = [&](JSONValue* item) {
		return value.Equal(item);
	};
	auto it = find_if(begin(), end(), cmp);
	if (it != end())
	{
		erase(it);
	}
}

std::string JSONArray::Repr() const
{
	if (empty()) {
		return "[]";
	}
	const std::string sep = ", ";
	std::ostringstream rep;
	auto it = cbegin(), end = this->cend();
	rep << '[' << (*it++)->Repr();
	for (; it != end; ++it) {
		rep << sep << (*it)->Repr();
	}
	rep << ']';
	return rep.str();
}

bool JSONArray::Equal(JSONValue* o) const
{
	if (o == this) {
		return true;
	}
	else if (o->type != this->type) {
		return false;
	}

	auto O = static_cast<JSONArray*>(o);
	const auto size = this->size();
	if (O->size() != size)
	{
		return false;
	}

	for (size_t i = 0; i < size; i++) {
		if (this->at(i) != O->at(i)) {
			return false;
		}
	}

	return true;
}

JSONValue* JSONArray::Clone() const
{
	auto o = new JSONArray();
	for (auto e : *this) {
		o->push_back(e->Clone());
	}
	return o;
}

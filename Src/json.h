#include <string.h>
#include <stdlib.h>
#include <vector>
#include <tchar.h>
#define _MBCS
#include <string>
#include <map>

constexpr double CompareError = 1e-3;
inline bool CompareFloats(const double& x, const double& y);

namespace namespace_json_ {
	enum class VALUE_TYPE {
		NUMBER,
		STRING,
		BOOLEAN,
		ARRAY,
		JNULL,
		OBJECT,
	};

	class JSONValue {
	public:
		const VALUE_TYPE type;
	protected:
		JSONValue(VALUE_TYPE type) noexcept;
	public:
		virtual JSONValue* Clone() const = 0;
		virtual bool Equal(JSONValue* o) const = 0;
		virtual ~JSONValue() {}
	};

	class JSONNumber : public JSONValue {
		bool isFloating;
		union {
			double fVal;
			int iVal;
		};
	public:
		JSONNumber() = delete;
		JSONNumber(const double& v) noexcept;
		JSONNumber(const int& v) noexcept;
		JSONNumber(const JSONNumber& o) noexcept;
		JSONNumber(JSONNumber&&) = delete;

		void Put(const double& v) noexcept;
		void Put(const int& v) noexcept;
		bool IsFloating() const noexcept;
		double GetAsFloat() const;
		int GetAsInt() const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONString : public JSONValue {
		char* szStr;
		size_t length;
	public:
		JSONString() = delete;
		JSONString(const char* str);
		JSONString(const JSONString& o);
		JSONString(JSONString&&) = delete;
		~JSONString();

		void Put(const char* str);
		const char* Get() const;
		size_t Length() const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONBoolean : public JSONValue {
		bool value;
	public:
		JSONBoolean() = delete;
		JSONBoolean(const bool& v) noexcept;
		JSONBoolean(const JSONBoolean& o) noexcept;
		JSONBoolean(JSONBoolean&&) = delete;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONArray : public JSONValue {
		std::vector<JSONValue*> elements;
	public:
		JSONArray();
		~JSONArray();

		JSONValue* At(const size_t& i);
		void Append(JSONValue* item);
		void Remove(JSONValue* item);
		void Clear();
		size_t Count() const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONNull : public JSONValue {
	public:
		JSONNull() noexcept;
		JSONNull(const JSONNull&) = delete;
		JSONNull(JSONNull&&) = delete;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONObject : public JSONValue {
		std::map<std::string, JSONValue*> properties;
	public:
		JSONObject();
		JSONObject(const JSONObject& o);
		JSONObject(JSONObject&&) = delete;
		~JSONObject();

		void Put(const std::string& key, JSONValue* v);
		bool Has(const std::string& key) const;
		JSONValue* Get(const std::string& key) const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	JSONObject* ParseObject(char* buffer, char*& next);
	JSONString* ParseString(char* buffer, char*& next);
	JSONNumber* ParseNumber(char* buffer, char*& next);
	JSONArray* ParseArray(char* buffer, char*& next);
	JSONValue* ParseBN(char* buffer, char*& next);
}

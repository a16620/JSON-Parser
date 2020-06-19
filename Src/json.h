#include <string.h>
#include <stdlib.h>
#include <vector>
#include <tchar.h>
#include <hash_map>

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
		JSONValue(VALUE_TYPE type);
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
		JSONNumber(const double& v);
		JSONNumber(const int& v);
		JSONNumber(const JSONNumber& o);
		JSONNumber(JSONNumber&&) = delete;

		void Put(const double& v);
		void Put(const int& v);
		bool IsFloating() const;
		double GetAsFloat() const;
		int GetAsInt() const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONString : public JSONValue {
		TCHAR* szStr;
		size_t length;
	public:
		JSONString() = delete;
		JSONString(const TCHAR* str);
		JSONString(const JSONString& o);
		JSONString(JSONString&&) = delete;
		~JSONString();

		void Put(const TCHAR* str);
		const TCHAR* Get() const;
		size_t Length() const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONBoolean : public JSONValue {
		bool value;
	public:
		JSONBoolean() = delete;
		JSONBoolean(const bool& v);
		JSONBoolean(const JSONBoolean& o);
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
		JSONNull();
		JSONNull(const JSONNull&) = delete;
		JSONNull(JSONNull&&) = delete;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};

	class JSONObject : public JSONValue {
		std::hash_map<std::string, JSONValue*> properties;
	public:
		JSONObject();
		JSONObject(const JSONObject& o);
		JSONObject(JSONObject&&) = delete;
		~JSONObject();

		bool Has(const std::string& key);
		JSONValue* Get(const std::string& key) const;

		virtual bool Equal(JSONValue* o) const;
		virtual JSONValue* Clone() const;
	};
}
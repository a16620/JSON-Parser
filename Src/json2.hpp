#include <string.h>
#include <stdexcept>
#include <sstream>
#include <functional>
#include <vector>
#include <string>
#include <map>

namespace namespace_json_2 {
	enum class VALUE_TYPE {
		JLITERAL,
		NUMBER,
		STRING,
		ARRAY,
		OBJECT,
	};

	class JValue {
	public:
		const VALUE_TYPE type;
	protected:
		JValue(VALUE_TYPE type) noexcept : type(type) {}
	public:
		JValue() = delete;
		JValue(const JValue&) = delete;

		std::string to_string() const
		{
			std::ostringstream os;
			Repr(os);
			return os.str();
		}

		virtual std::ostream& Repr(std::ostream& os) const = 0;
		virtual JValue* Clone() const = 0;
		virtual bool Equal(JValue* o) const = 0;
		virtual ~JValue() {}

		static JValue* Parse(std::istream& is);
	};

	std::ostream& operator<<(std::ostream& os, const JValue* v) {
		return v->Repr(os);
	}

	std::ostream& operator<<(std::ostream& os, const JValue& v) {
		return v.Repr(os);
	}

	using JFloat = double;
	static bool CompareFloats(const JFloat& x, const JFloat& y);

	class JNumber : public JValue {
		JFloat fVal;
	public:
		JNumber() noexcept : JValue(VALUE_TYPE::NUMBER), fVal(0) {}
		JNumber(const JFloat& v) noexcept : JValue(VALUE_TYPE::NUMBER), fVal(v) {}
		JNumber(const JNumber& o) noexcept : JValue(VALUE_TYPE::NUMBER), fVal{o.fVal} {}

		void Set(const JFloat& v) noexcept
		{
			fVal = v;
		}
		template<typename number>
		JNumber& operator=(const number& v) noexcept
		{
			fVal = static_cast<JFloat>(v);
		}

		operator JFloat() const noexcept {
			return fVal;
		}
		operator int() const noexcept
		{
			return static_cast<int>(fVal);
		}

		std::ostream& Repr(std::ostream& os) const override
		{
			os << fVal;
			return os;
		}

		bool Equal(JValue* o) const override
		{
			if (o == this) {
				return true;
			}
			else if (o->type != this->type) {
				return false;
			}

			auto n = reinterpret_cast<JNumber*>(o);
			return CompareFloats(fVal, n->fVal);
		}
		JValue* Clone() const override
		{
			return new JNumber(*this);
		}

		static JNumber* Parse(std::istream& is);
	};

	static std::string EscapeString(const std::string& s);
	class JString : public JValue, public std::string {
	public:
		JString() : JValue(VALUE_TYPE::STRING) {}
		JString(const char* str) : JValue(VALUE_TYPE::STRING), std::string(str) {}
		JString(const std::string& str) : JValue(VALUE_TYPE::STRING), std::string(str) {}
		JString(std::string&& rstr) : JValue(VALUE_TYPE::STRING), std::string(rstr) {}
		JString(const JString& o) : JValue(VALUE_TYPE::STRING), std::string(o) {}

		using std::string::operator=;
		using std::string::operator+=;
		using std::string::operator[];
		void Set(const char* str)
		{
			static_cast<std::string*>(this)->operator=(str);
		}

		std::ostream& Repr(std::ostream& os) const override
		{
			return os << EscapeString(*this);
		}
		bool Equal(JValue* o) const override
		{
			if (o == this) {
				return true;
			}
			else if (o->type != this->type) {
				return false;
			}

			return *static_cast<JString*>(o) == *this; //std::string compare
		}

		JValue* Clone() const override
		{
			return new JString(*this);
		}

		static JString* Parse(std::istream& is);
		static std::string ParseString(std::istream& is);
	};

	class JArray : public JValue, public std::vector<JValue*> {
	public:
		JArray() noexcept : JValue(VALUE_TYPE::ARRAY) {}
		~JArray()
		{
			for (const auto& e : *this)
			{
				delete e;
			}
		}

		using std::vector<JValue*>::operator[];
		void Remove(JValue* item)
		{
			auto it = find(begin(), end(), item);
			if (it != end())
			{
				erase(it);
			}
		}

		void Remove(const JValue& value)
		{
			auto cmp = [&](JValue* item) {
				return value.Equal(item);
			};
			auto it = find_if(begin(), end(), cmp);
			if (it != end())
			{
				erase(it);
			}
		}

		std::ostream& Repr(std::ostream& os) const override
		{
			if (empty()) {
				os << "[]";
				return os;
			}
			const std::string sep = ", ";
			auto it = cbegin(), end = this->cend();
			os << '[' << (*it++);
			for (; it != end; ++it) {
				os << sep << (*it);
			}
			os << ']';
			return os;
		}
		bool Equal(JValue* o) const override
		{
			if (o == this) {
				return true;
			}
			else if (o->type != this->type) {
				return false;
			}

			auto O = static_cast<JArray*>(o);
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
		JValue* Clone() const override
		{
			auto o = new JArray();
			for (auto e : *this) {
				o->push_back(e->Clone());
			}

			return o;
		}

		static JArray* Parse(std::istream& is);
	};

	class JObject : public JValue, public std::map<std::string, JValue*> {
	public:
		JObject() noexcept : JValue(VALUE_TYPE::OBJECT) {}
		JObject(const JObject& o) : JValue(VALUE_TYPE::OBJECT) {}
		~JObject()
		{
			for (const auto& kv : *this)
			{
				delete kv.second;
			}
		}

		void Set(const std::string& key, JValue* v)
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
		bool Has(const std::string& key) const
		{
			return count(key);
		}
		using std::map<std::string, JValue*>::operator[];

		std::ostream& Repr(std::ostream& os) const override
		{
			if (empty()) {
				os << "{}";
				return os;
			}
			const std::string sep = ", ";
			auto it = cbegin(), end = this->cend();
			os << '{' << EscapeString(it->first) << ':' << it->second;
			for (; it != end; ++it) {
				os << sep << EscapeString(it->first) << ':' << it->second;
			}
			os << '}';
			return os;
		}
		bool Equal(JValue* o) const override
		{
			if (o == this) {
				return true;
			}
			else if (o->type != this->type) {
				return false;
			}

			auto O = static_cast<JObject*>(o);
			if (O->size() != size()) {
				return false;
			}

			const auto compare_pair = [](value_type const& v1, value_type const& v2) {
				return v1.first == v2.first && v1.second->Equal(v2.second);
			};
			return std::equal(begin(), end(), O->begin(), compare_pair);
		}
		JValue* Clone() const override
		{
			return new JObject(*this);
		}

		static JObject* Parse(std::istream& is);
	};

	class JLiteral : public JValue {
		int flag;
		const int mask_null = 1, mask_bool = 2;
	public:
		JLiteral() : JValue(VALUE_TYPE::JLITERAL), flag{0}
		{
			//memset(&flag, 0, sizeof(int));
			flag |= mask_null;
		}

		JLiteral(const JLiteral& o) : JValue(VALUE_TYPE::JLITERAL), flag{ o.flag } {}

		JLiteral(bool b) : JValue(VALUE_TYPE::JLITERAL), flag{0}
		{
			//memset(&flag, 0, sizeof(int));
			flag |= !!b << 1;
		}

		bool IsNull() const
		{
			return flag & mask_null;
		}

		void SetNull()
		{
			flag |= mask_null;
		}

		void Set(bool b)
		{
			flag = 0;
			flag |= !!b << 1;
		}

		bool Est() const
		{
			return !(flag & mask_null) && (flag & mask_bool);
		}

		operator bool() const
		{
			return Est();
		}

		bool Bool() const
		{
			return flag & mask_bool;
		}

		std::ostream& Repr(std::ostream& os) const override
		{
			if (IsNull())
				os << "null";
			else
				os <<  Bool() ? "true" : "false";
			return os;
		}
		JValue* Clone() const override
		{
			return new JLiteral(*this); 
		}
		bool Equal(JValue* o) const override
		{
			if (o == this) {
				return true;
			}
			else if (o->type == VALUE_TYPE::STRING) {
				auto S = static_cast<JString*>(o);
				return this->to_string() == *static_cast<std::string*>(S);
			}
			else if (o->type != VALUE_TYPE::JLITERAL) {
				return false;
			}

			auto O = static_cast<JLiteral*>(o);
			return (O->IsNull() == IsNull()) && (IsNull() || O->Bool() == Bool());
		}

		static JLiteral* Parse(std::istream& is);
	};

	static void SkipSpaces(std::istream& is)
	{
		std::istream::char_type c;
		while (is >> c) {
			switch (c)
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				break;
			default:
				is.unget();
				return;
			}
		}
	}

	JValue* JValue::Parse(std::istream& is)
	{
		SkipSpaces(is);

		JValue* v = nullptr;
		auto c = is.get();
		is.unget();
		switch (c) {
			//string
		case '"':
		case '\'':
			v = JString::Parse(is);
			break;
			//number with sign 
		case '-':
		case '+':
			v = JNumber::Parse(is);
			break;
			//array
		case '[':
			v = JArray::Parse(is);
			break;
			//object
		case '{':
			v = JObject::Parse(is);
			break;
		default: //else: number, symbolic literal, error
		{
			if (std::isdigit(c)) { //number
				v = JNumber::Parse(is);
			}
			else if (std::isalpha(c)) { //symbolic literal
				v = JLiteral::Parse(is);
			}
			else {
				throw std::runtime_error("인식 불가");
			}
		}
		}

		return v;
	}

	JNumber* JNumber::Parse(std::istream& is)
	{
		double d;
		is >> d;

		if (is.fail())
			throw std::runtime_error("숫자 형식 오류");

		return new JNumber(d);
	}

	JString* JString::Parse(std::istream& is)
	{
		const std::string& str = ParseString(is);
		return new JString(str);
	}

	std::string JString::ParseString(std::istream& is)
	{
		constexpr char escaper = '\\';
		std::istream::char_type c, quot;
		bool escaping = false;
		is.get(quot); //single quot를 지원하기 위함

		std::string str;

		while (is.get(c))
		{
			if (escaping)
			{
				switch (c) {
				case 'b':
					c = '\b';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case 'u':
					str.push_back('\\');
					break;
				}
				str.push_back(c);
				escaping = false;
				continue;
			}
			else if (c == quot)
				return str;
			else if (c == escaper)
				escaping = true;
			else
				str.push_back(c);
		}

		//quot 나오기 전에 스트림 종료->오류
		throw std::out_of_range("비정상적인 종료");
	}

	JArray* JArray::Parse(std::istream& is)
	{
		auto arr = new JArray();
		
		if (is.get() != '[') {
			is.unget();
		}

		SkipSpaces(is);

		std::istream::char_type c;
		while (is.get(c))
		{
			is.unget();
			JValue* v = nullptr;
			try {
				v = JValue::Parse(is);

				arr->push_back(v);
				SkipSpaces(is);
			}
			catch (std::exception e)
			{
				if (v) {
					delete v;
				}
				delete arr;
				throw e;
			}

			SkipSpaces(is);

			if (is.get(c))
			{
				if (c == ',') {
					SkipSpaces(is);
					continue;
				}
				else if (c == ']')
				{
					break;
				}
				else {
					throw std::runtime_error("불완전한 배열");
				}
			}
			else {
				throw std::runtime_error("비정상적인 종료");
			}
		}

		return arr;
	}

	JObject* JObject::Parse(std::istream& is)
	{
		auto obj = new JObject();

		if (is.get() != '{') {
			is.unget();
		}
		SkipSpaces(is);

		std::istream::char_type c;
		while (is.get(c))
		{
			is.unget();
			JValue* v = nullptr;
			try {
				const std::string& key = JString::ParseString(is);

				SkipSpaces(is);
				if (!is.get(c) || c != ':') {
					throw std::runtime_error("':' 없음");
				}
				SkipSpaces(is);

				v = JValue::Parse(is);

				obj->Set(key, v);
			}
			catch (std::runtime_error e)
			{
				if (v) {
					delete v;
				}
				delete obj;
				throw e;
			}

			SkipSpaces(is);
			if (is.get(c))
			{
				if (c == ',') {
					SkipSpaces(is);
					continue;
				}
				else if (c == '}')
				{
					break;
				}
				else {
					throw std::runtime_error("불완전한 배열");
				}
			}
			else {
				throw std::runtime_error("비정상적인 종료");
			}
		}
		return obj;
	}

	JLiteral* JLiteral::Parse(std::istream& is)
	{
		const std::string str_true = "true", str_false = "false", str_null = "null";
		const std::string* cu;
		std::function<JLiteral* (void)> fac;
		size_t idx = 1;
		std::istream::char_type c;
		if (is.get(c))
		{
			switch (c) {
			case 't':
				cu = &str_true;
				fac = []() {
					return new JLiteral(true);
				};
				break;
			case 'f':
				cu = &str_false;
				fac = []() {
					return new JLiteral(false);
				};
				break;
			case 'n':
				cu = &str_null;
				fac = []() {
					return new JLiteral();
				};
				break;
			default:
				throw std::runtime_error("정의되지 않은 토큰");
			}
		}
		else {
			throw std::runtime_error("비정상적 종료 또는 스트림 읽기 실패");
		}
		
		while (is.get(c) && std::isalpha(c)) { //or std::isalnum
			if ((*cu)[idx++] != c) { //symbol이 모두 다르기 때문에
				throw std::runtime_error("매칭되는 리터럴 없음");
			}
		}

		if (idx != cu->length()) {
			throw std::runtime_error("매칭되는 리터럴 없음");
		}

		return fac();
	}

	static std::string EscapeString(const std::string& s)
	{
		std::ostringstream oss;
		oss << "\"";
		for (const auto& c : s) {
			switch (c) {
			case '"':
				oss << "\\\"";
				break;
			case '\\':
				oss << "\\\\";
				break;
			case '/':
				oss << "\\/";
				break;
			case '\b':
				oss << "\\b";
				break;
			case '\f':
				oss << "\\f";
				break;
			case '\n':
				oss << "\\n";
				break;
			case '\r':
				oss << "\\r";
				break;
			case '\t':
				oss << "\\t";
				break;
			default:
				oss << c;
			}
		}
		oss << "\"";
		return oss.str();
	}

	constexpr JFloat CompareError = 1e-3;
	static bool CompareFloats(const JFloat& x, const JFloat& y)
	{
		return std::abs(x - y) < CompareError;
	}
}

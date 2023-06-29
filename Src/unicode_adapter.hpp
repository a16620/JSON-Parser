#include <iostream>
#include <sstream>
#include <memory.h>

using namespace std;
class UnicodeEncoder : public ostream, private streambuf {
	ostream& os;

	static constexpr size_t buffer_size = 10;
	char buffer[buffer_size];

	void resetBuffer()
	{
		memset(buffer, 0, buffer_size);
		setp(buffer, buffer + buffer_size - 2);
	}

public:
	UnicodeEncoder(ostream& os) : os(os), ostream(this)
	{
		resetBuffer();
	}

	~UnicodeEncoder()
	{
		flush();
	}

	virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type c) {
		if (c != std::char_traits<char>::eof()) {
			auto diff = this->pptr() - buffer;
			*this->pptr() = std::char_traits<char>::to_char_type(c);
			this->pbump(1);
		}
		flush();

		return c;
	}

	ostream& flush() {
		char* ptr = buffer;
		while (*ptr != '\0') {
			if (*ptr & 0x80) {
				const int size = mblen(ptr, MB_CUR_MAX);
				char mbBuffer[5] = { 0 };
				strncpy_s(mbBuffer, sizeof(mbBuffer), ptr, size);
				
				const auto copy_len = strlen(mbBuffer);
				if (copy_len == size) {
					wchar_t ucBuffer;
					const auto l = mbtowc(&ucBuffer, mbBuffer, copy_len);
					if (l == -1) {
						string msg("mbtowc can't convert ");
						msg.append(mbBuffer);
						throw std::runtime_error(msg);
					}
					stringstream hex;
					hex << std::hex << ucBuffer;
					os << "\\u" << hex.str();
					ptr += size;
				} else { //버퍼안에 미완성된 멀티바이트
					size_t left = buffer_size - (ptr - buffer)-1;
					memmove(buffer, ptr, left);
					memset(buffer + left, 0, ptr - buffer);
					
					setp(buffer + left, buffer + buffer_size - 2);
					return *this;
				}
			}
			else {
				os.put(*(ptr++));
			}
		}

		resetBuffer();
		return *this;
	}
};

#include "http_request.h"
#include <iostream>
#include <sstream>
#include <mutex>
#pragma comment(lib, "ws2_32.lib")

using namespace http_request;
using namespace std;

template<class T>
const auto ArrayDeleter = [](T* ptr) { delete[] ptr; };

string ReadLine(const char* buffer, size_t& length)
{
	auto end = strstr(buffer, "\r\n");
	if (end == NULL)
		return string(buffer);
	size_t&& len = end - buffer;
	length = len;
	return string(buffer, len);
}

std::future<HTTPRespond> http_request::make_request(const string& url, ThreadPool* pool)
{
	const auto request_func = [](string host, string uri) {
		std::unique_ptr<char, std::function<void(char*)>> result = move(Fetch(host, uri));
		return ParseHTTP(result.get());
	};
	
	string host, uri;
	size_t ofs = url.find("://");
	if (ofs != string::npos)
	{
		auto endHost = url.find_first_of('/', ofs + 3);
		if (endHost == string::npos)
		{
			host = url.substr(ofs + 3);
			uri = '/';
		}
		else
		{
			host = url.substr(ofs + 3, endHost - (ofs + 3));
			uri = url.substr(endHost);
		}
	}
	else
	{
		auto endHost = url.find_first_of('/');
		if (endHost == string::npos)
		{
			host = url;
			uri = '/';
		}
		else
		{
			host = url.substr(0, endHost);
			uri = url.substr(endHost);
		}
	}

	if (pool == nullptr)
	{
		return async(request_func, host, uri);
	}

	return pool->EnqueueTask(request_func, host, uri);
}

std::unique_ptr<char, std::function<void(char*)>> http_request::Fetch(const string& host, const string& uri)
{
	auto s = MakeConnection(host);
	std::unordered_map<std::string, std::string> additionalHeader;
	additionalHeader = {
		{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.106 Safari/537.36"},
		{"Accept", "text/html,application/xhtml+xml,application/xml;\ q=0.9,imgwebp,*/*;q=0.8"},
		{"Host", host}
	};
	SendRequest(s, uri, additionalHeader);

	int rb;
	char temp[512];
	ostringstream oss;
	do {
		rb = recv(s, temp, 512, 0);
		if (rb > 0)
			oss.write(temp, rb);
	} while (rb == 512);

	if (rb == -1) {
		string msg("가져오지 못했습니다 #");
		msg += std::to_string(WSAGetLastError());
		throw runtime_error(msg);
	}
	
	string str = oss.str();
	if (str.empty())
		throw runtime_error("아무것도 받지 못했습니다");

	const auto&& length = str.size();
	std::unique_ptr<char, std::function<void(char*)>> buffer(new char[length+1], ArrayDeleter<char>);
	memcpy(buffer.get(), str.c_str(), length);
	buffer.get()[length] = '\0';

	return buffer;
}

HTTPRespond http_request::ParseHTTP(const char* buffer)
{
	HTTPRespond respond;

	auto endHead = strstr(buffer, "\r\n\r\n");
	if (endHead == NULL)
	{
		respond.headerOnly = true;
		endHead = buffer + strlen(buffer);//헤더만 존재 //throw out_of_range("미완성 헤더입니다");
	}
	size_t n;
	string line = ReadLine(buffer, n);
	buffer += n + 2;
	
	{
		auto idx = line.find_first_of(' '), bg = 0U;
		if (idx == string::npos)
			throw out_of_range("미완성 헤더입니다");
		respond.version = line.substr(0, idx);

		bg = idx+1;
		idx = line.find_first_of(' ', bg);
		if (idx == string::npos)
			throw out_of_range("미완성 헤더입니다");
		auto szCode = line.substr(bg, idx-bg);
		respond.code = std::stoi(szCode);

		bg = idx+1;
		
		respond.respondMessage = line.substr(bg);
	}

	while (buffer <= endHead)
	{
		line = ReadLine(buffer, n);
		buffer += n + 2;
		
		auto idx = line.find_first_of(':');
		if (idx == string::npos)
			break;
		string key = line.substr(0, idx), value = ltrim(line.substr(idx+1));
		respond.header.insert(make_pair(key, value));	
	}
	
	respond.content = string(endHead + 4);

	return respond;
}

string http_request::ltrim(string o)
{
	o.erase(o.begin(), find_if_not(o.begin(), o.end(), ptr_fun<int, int>(std::isspace)));
	return o;
}

SOCKET http_request::MakeConnection(string host)
{
	USHORT port = 80;
	auto pidx = host.find_last_of(':');
	if (pidx != string::npos)
	{
		string szPort = host.substr(pidx+1);
		int tport;
		try {
			tport = std::stoi(szPort);
		}
		catch (exception e)
		{
			throw runtime_error("포트 번호를 변환할 수 없습니다");
		}
		if (tport > USHRT_MAX || tport < 0)
			throw runtime_error("포트 번호가 비정상적입니다");
		port = static_cast<USHORT>(tport);
		host.erase(pidx);
	}

	auto results = DNSLookup(host);
	if (results.empty())
		throw runtime_error("호스트를 찾을 수 없습니다");

	ULONG ipAddress = results[0];
	sockaddr_in address;
	memset(&address, 0, sizeof sockaddr_in);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = ipAddress;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		throw runtime_error("소켓을 생성하지 못했습니다");

	if (connect(s, reinterpret_cast<sockaddr*>(&address), sizeof sockaddr_in) == SOCKET_ERROR)
	{
		string msg = "연결 실패 #";
		msg += std::to_string(WSAGetLastError());
		throw runtime_error(msg);
	}

	return s;
}

vector<ULONG> http_request::DNSLookup(const string& host)
{
	if (host == "localhost")
		return vector<ULONG>({ INADDR_LOOPBACK });

	vector<ULONG> addresses;
	auto result = gethostbyname(host.c_str());
	if (result == NULL)
	{
		auto err = WSAGetLastError();
		if (err == WSATRY_AGAIN)
		{
			do {
				result = gethostbyname(host.c_str());
				if (result != NULL)
					goto ENDERROR;
			} while ((err = WSAGetLastError()) == WSATRY_AGAIN);
		}

		string msg = "주소를 찾을 수 없습니다 #";
		msg += std::to_string(err);
		throw runtime_error(msg);
	}

ENDERROR:
	//TODO result->h_addrtype?
	while (*result->h_addr_list != NULL)
	{
		auto paddr = reinterpret_cast<ULONG*>(*result->h_addr_list);
		addresses.push_back(*paddr);
		result->h_addr_list++;
	}
	return addresses;
}

size_t http_request::SendRequest(SOCKET ss, const std::string& uri, const std::unordered_map<std::string, std::string>& header)
{
	ostringstream context;
	context << "GET " << uri << " HTTP/1.1" << "\r\n";
	for (auto pair : header)
	{
		context << pair.first << ": " << pair.second << "\r\n";
	}
	context << "\r\n";
	string full = context.str();
	auto const raw = full.c_str();
	const size_t len = full.size();
	size_t sentbytes = 0;
	const size_t dataUnit = 512;

	do {
		size_t t = 0;
		if (len - sentbytes > dataUnit)
			t = send(ss, raw + sentbytes, dataUnit, 0);
		else
			t = send(ss, raw + sentbytes, len - sentbytes, 0);

		if (t == -1)
		{
			string msg("전송 실패 #");
			msg += std::to_string(WSAGetLastError());
			throw runtime_error(msg);
		}
		sentbytes += t;
	} while (len > sentbytes);

	return sentbytes;
}
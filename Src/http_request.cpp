#include "http_request.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

using namespace http_request;
using namespace std;

const auto ArrayDeleter = [](char* ptr) { delete[] ptr; };

http_request::ThreadPool::ThreadPool(size_t count) : count(count)
{
	if (count == 0) {
		throw runtime_error("Pool must have more than one thread");
	}

	//for (count)
}

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
	if (pool == nullptr)
	{
		//async()
	}
	return std::future<HTTPRespond>();
}

std::unique_ptr<char, std::function<void(char*)>> http_request::Fetch(const string& host, const string& uri)
{
	auto s = MakeConnection(host);

	

	std::unique_ptr<char, std::function<void(char*)>> buffer(nullptr, ArrayDeleter);
	return buffer;
}

HTTPRespond http_request::ParseHTTP(const char* buffer)
{
	HTTPRespond respond;


	auto endHead = strstr(buffer, "\r\n\r\n");
	if (endHead == NULL)
		throw runtime_error("�̿ϼ� ����Դϴ�");

	size_t n;
	string line = ReadLine(buffer, n);
	buffer += n + 2;
	
	{
		auto idx = line.find_first_of(' '), bg = 0U;
		if (idx == string::npos)
			throw runtime_error("�̿ϼ� ����Դϴ�");
		respond.version = line.substr(0, idx);

		bg = idx+1;
		idx = line.find_first_of(' ', bg);
		if (idx == string::npos)
			throw runtime_error("�̿ϼ� ����Դϴ�");
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
			throw runtime_error("��Ʈ ��ȣ�� ��ȯ�� �� �����ϴ�");
		}
		if (tport > USHRT_MAX || tport < 0)
			throw runtime_error("��Ʈ ��ȣ�� ���������Դϴ�");
		port = static_cast<USHORT>(tport);
		host.erase(pidx);
	}

	auto results = DNSLookup(host);
	if (results.empty())
		throw runtime_error("ȣ��Ʈ�� ã�� �� �����ϴ�");

	ULONG ipAddress = results[0];
	sockaddr_in address;
	memset(&address, 0, sizeof sockaddr_in);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = ipAddress;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		throw runtime_error("������ �������� ���߽��ϴ�");

	if (connect(s, reinterpret_cast<sockaddr*>(&address), sizeof sockaddr_in) == SOCKET_ERROR)
	{
		string msg = "���� ���� #";
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

		string msg = "�ּҸ� ã�� �� �����ϴ� #";
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

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	auto r= DNSLookup("www.google.com");
	for (auto e : r)
	{
		in_addr a;
		a.s_addr = e;
		cout << inet_ntoa(a) << endl;
	}
	WSACleanup();
	return 0;
}
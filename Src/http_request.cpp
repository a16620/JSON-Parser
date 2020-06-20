#include "http_request.h"
#include <iostream>

using namespace http_request;
using namespace std;

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

std::future<HTTPRespond> http_request::make_request(std::string url, ThreadPool* pool)
{
	if (pool == nullptr)
	{
		async()
	}
	return std::future<HTTPRespond>();
}

HTTPRespond http_request::ParseHTTP(const char* buffer)
{
	HTTPRespond respond;


	auto endHead = strstr(buffer, "\r\n\r\n");
	if (endHead == NULL)
		throw runtime_error("미완성 헤더입니다");

	size_t n;
	string line = ReadLine(buffer, n);
	buffer += n + 2;
	//TODO
	{
		auto idx = line.find_first_of(' '), bg = 0U;
		if (idx == string::npos)
			throw runtime_error("미완성 헤더입니다");
		respond.version = line.substr(0, idx);

		bg = idx+1;
		idx = line.find_first_of(' ', bg);
		if (idx == string::npos)
			throw runtime_error("미완성 헤더입니다");
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
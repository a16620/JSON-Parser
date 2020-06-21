#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <string>
#include <unordered_map>
#include <thread>
#include <set>
#include <future>

namespace http_request {
	struct HTTPRespond {
		int code;
		std::string respondMessage, version, content;
		std::unordered_map<std::string, std::string> header;
	};

	class WorkerThread {
		std::thread thread;
		size_t id;
	public:
		WorkerThread();
		
	};

	class ThreadPool {
		const size_t count;
		std::vector<WorkerThread> threads;
		std::set<int> available;
	public:
		ThreadPool() = delete;
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool(size_t count);
	};

	std::future<HTTPRespond> make_request(const std::string& url, ThreadPool* pool=nullptr);
	
	std::unique_ptr<char, std::function<void(char*)>> Fetch(const std::string& host, const std::string& uri);
	HTTPRespond ParseHTTP(const char* buffer);
	std::string ltrim(std::string o);

	SOCKET MakeConnection(std::string host);
	std::vector<ULONG> DNSLookup(const std::string& host);
}
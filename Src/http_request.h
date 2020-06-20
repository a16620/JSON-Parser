#pragma once
#include <WinSock2.h>
#include <string>
#include <unordered_map>
#include <thread>
#include <set>
#include <future>

namespace http_request {
	enum class Method {

	};

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

	std::future<HTTPRespond> make_request(std::string url, ThreadPool* pool=nullptr);
	HTTPRespond ParseHTTP(const char* buffer);
	std::string ltrim(std::string o);
}
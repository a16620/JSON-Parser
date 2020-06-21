#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <thread>
#include <queue>
#include <future>
#include <iostream>

namespace http_request {
	struct HTTPRespond {
		int code=0;
		std::string respondMessage, version, content;
		std::unordered_map<std::string, std::string> header;
		bool headerOnly = false;
	};

	class ThreadPool {
		std::vector<std::thread> threads;
		std::queue<std::function<void()>> tasks;
		std::mutex lock;
		std::condition_variable cv;
		bool stop;

		void Work()
		{
			while (true)
			{
				std::unique_lock<std::mutex> lk(lock);
				cv.wait(lk, [this]() {return this->stop || !this->tasks.empty(); });
				if (stop)
					return;
				std::function<void()> task = move(tasks.front());
				tasks.pop();
				lk.unlock();
				
				task();
			}
		}
	public:
		ThreadPool() = delete;
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool(size_t count) : stop(false)
		{
			if (count == 0)
				throw std::invalid_argument("스레드 수는 0보다 커야합니다");
			for (int i = 0; i < count; i++)
			{
				threads.push_back(std::thread([this]() { this->Work(); }));
			}
		}

		void Stop()
		{
			stop = true;
			cv.notify_all();
		}

		~ThreadPool()
		{
			Stop();

			for (size_t i = 0U; i < threads.size(); i++)
			{
				if (threads[i].joinable())
					threads[i].join();
			}
		}
		
		template <typename Fn, typename... Args>
		auto EnqueueTask(Fn&& fn, Args&&... args)
		{
			using namespace std;
			using rType = typename result_of<Fn(Args...)>::type;
			auto task = make_shared<packaged_task<rType()>>(
				std::bind(forward<Fn>(fn), forward<Args>(args)...)
				);
			auto future = task->get_future();
			{
				lock_guard<mutex> lk(lock);
				tasks.push([task]() { (*task)(); });
			}
			cv.notify_one();
			return future;
		}
	};

	std::future<HTTPRespond> make_request(const std::string& url, ThreadPool* pool=nullptr);
	
	HTTPRespond ParseHTTP(const char* buffer);
	std::string ltrim(std::string o);

	std::unique_ptr<char, std::function<void(char*)>> Fetch(const std::string& host, const std::string& uri);
	SOCKET MakeConnection(std::string host);
	std::vector<ULONG> DNSLookup(const std::string& host);
	size_t SendRequest(SOCKET ss, const std::string& uri, const std::unordered_map<std::string, std::string>& header);
}
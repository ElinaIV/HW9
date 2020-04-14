#pragma once

#include <boost/lockfree/stack.hpp>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <memory>
#include <atomic>
#include <stack>
#include <mutex>
#include <chrono>
#include <random>

using ms = std::chrono::milliseconds;
class Timer {
	using clock_t = std::chrono::steady_clock;
	using timepoint_t = clock_t::time_point;

private:
	timepoint_t m_begin;
	ms m_duration;
	bool m_flag;

public:
	Timer() : m_begin(clock_t::now()), m_duration(0), m_flag(false) {}

	void stop() {
		m_duration += std::chrono::duration_cast<ms> (clock_t::now() - m_begin);
		m_flag = false;
		std::cout << m_duration.count() << " ms" << std::endl;
	}

	void restart() {
		m_begin = clock_t::now();
		m_duration = ms(0);
		m_flag = true;
	}

	~Timer() noexcept {
		if (m_flag) { stop(); }
	}
};

template < typename T >
class threadsafe_stack {
private:
	std::stack<T> data;
	mutable std::mutex mutex;

public:
	threadsafe_stack(){}

	threadsafe_stack(const threadsafe_stack& other) {
		std::lock_guard < std::mutex > lock(other.mutex);
		data = other.data;
	}

	threadsafe_stack& operator=(const threadsafe_stack&) = delete;

	void push(T new_value) {
		std::lock_guard < std::mutex > lock(mutex);
		data.push(new_value);
	}

	std::shared_ptr < T > pop() {
		std::lock_guard < std::mutex > lock(mutex);
		if (data.empty()) { return nullptr; };
		const std::shared_ptr < T > result(std::make_shared < T >(data.top()));
		data.pop();
		return result;
	}

	void pop(T& value) {
		std::lock_guard < std::mutex > lock(mutex);
		if (data.empty()) { return; };
		value = data.top();
		data.pop();
	}
};

template<typename T>
void add_M_elements(threadsafe_stack<T>& stack, size_t M, std::atomic<bool>& flag) {
	while (!flag.load()) {
		std::this_thread::yield();
	}
	
	for (size_t i = 0; i < M; ++i) {
		stack.push(i);
	}
}

template<typename T>
void pop_M_elements(threadsafe_stack<T>& stack, size_t M, std::atomic<bool>& flag) {
	while (!flag.load()) {
		std::this_thread::yield();
	}

	for (size_t i = 0; i < M; ++i) {
		stack.pop();
	}
}

void check() {
	Timer t;
	std::atomic<bool> flag{ false };
	threadsafe_stack<int> stack;
	//boost::lockfree::stack<int> stack;

	const size_t N = 4;
	const size_t M = 1000;
	std::vector<std::thread> producers;
	std::vector<std::thread> consumers;

	for (size_t i = 0; i < N; ++i) {
		producers.push_back(std::thread(add_M_elements<int>, std::ref(stack), M, std::ref(flag)));
		consumers.push_back(std::thread(pop_M_elements<int>, std::ref(stack), M, std::ref(flag)));
	}

	t.restart();
	flag.store(true);
	std::for_each(producers.begin(), producers.end(), std::mem_fn(&std::thread::join));
	std::for_each(consumers.begin(), consumers.end(), std::mem_fn(&std::thread::join));
	t.stop();
}

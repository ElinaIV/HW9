#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename T, typename Container = std::vector<T>, typename Comparator = std::less<T>>
class Threadsafe_PQueue {
private:
	std::priority_queue<T, Container, Comparator> m_pQueue;
	std::condition_variable m_condition_variable;
	mutable std::mutex m_mutex;

public:
	Threadsafe_PQueue() = default;

	Threadsafe_PQueue(const Threadsafe_PQueue& other) {
		std::lock_guard<std::mutex> lock(other.m_mutex);
		m_pQueue = other.m_pQueue;
	}

	Threadsafe_PQueue& operator=(const Threadsafe_PQueue& other) {
		std::lock(m_mutex, other.m_mutex);
		m_pQueue = other.m_pQueue;
		return this;
	}

	void swap(Threadsafe_PQueue& other) {
		std::lock(m_mutex, other.m_mutex);
		m_pQueue.swap(other.m_pQueue);
	}

	void push(T value) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pQueue.push(value);
		m_condition_variable.notify_one();
	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_condition_variable.wait(lock, [this]{return !m_pQueue.empty(); });
		value = m_pQueue.front();
		m_pQueue.pop();
	}

	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condition_variable.wait(lock, [this]{return !m_pQueue.empty(); });
		auto result = std::make_shared<T>(m_pQueue.top());
		m_pQueue.pop();

		return result;
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_pQueue.empty()) {
			return false;
		}
		value = m_pQueue.front();
		m_pQueue.pop();

		return true;
	}

	std::shared_ptr<T> try_pop() {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_pQueue.empty()) {
			return std::shared_ptr<T>();
		}
		auto result = std::make_shared<T>(m_pQueue.top());
		m_pQueue.pop();

		return result;
	}

	T top() {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_pQueue.top();
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_pQueue.empty();
	}
};

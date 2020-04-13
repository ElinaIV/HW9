#pragma once

#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <mutex>
#include <random>
#include <future>

std::mutex pi_mutex;

double calculatePi(long long n) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	size_t inside = 0;

	for (size_t i = 0; i < n; i++) {
		auto randX = dis(gen);
		auto randY = dis(gen);
		if (randX * randX + randY * randY < 1)
			++inside;
	}

	std::lock_guard<std::mutex> guard(pi_mutex);
	return 4.0 * inside / n;
}

double calculatePiParallel(long long n) {
	size_t num_threads = std::thread::hardware_concurrency();
	int block_size = n / num_threads;

	std::vector<std::future<double>> results(num_threads - 1);
	std::vector<std::thread> threads(num_threads - 1);
	for (size_t i = 0; i < num_threads - 1; ++i) {
		std::packaged_task<double(double)> task(calculatePi);
		results[i] = task.get_future();
		threads[i] = std::thread(std::move(task), block_size);
	}
	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

	auto result = calculatePi(n);
	std::for_each(results.begin(), results.end(), [&result](auto& i) { result += i.get(); });

	return result / num_threads;
}
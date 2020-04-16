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
double global = 0;

void calculatePi(long long n) {
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
	global += 4.0 * inside / n;
}

double calculatePiParallel(long long n) {
	size_t num_threads = std::thread::hardware_concurrency();
	int block_size = n / num_threads;

	std::vector<std::thread> threads;
	for (size_t i = 0; i < num_threads; ++i) {
		threads.push_back(std::thread(&calculatePi, n));
	}
	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

	return global / num_threads;
}

#pragma once

#include <boost/lockfree/stack.hpp>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <thread>
#include <array>
#include <iostream>
#include <string>
#include <regex>
#include <memory>
#include <atomic>
#include <stack>
#include <mutex>
#include <random>

std::atomic<bool> flag(false);

//using stack_t = threadsafe_stack < int >;
using stack_t = lock_free_stack<int>;

stack_t s;

std::random_device rd;
std::mt19937 generator(rd());
std::uniform_int_distribution<> uid(0, 100);

struct Data {
	
};

void check() {
	auto produsers = std::thread::hardware_concurrency();
}
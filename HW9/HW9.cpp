#include <iostream>
#include "Monte-Carlo.h"
#include "threadsafe_queue.h"
#include "DNK.h"
#include "Comp_from_Boost.h"

int main() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 100);

	//std::cout << calculatePiParallel(1000000);

	Threadsafe_PQueue<int> queue;
	for (int i = 0; i < 10; ++i) {
		queue.push(dis(gen));
	}

	while (!queue.empty()) {
		int val = queue.top();
		queue.wait_and_pop();
		std::cout << val << " ";
	}
	std::cout << std::endl;
}

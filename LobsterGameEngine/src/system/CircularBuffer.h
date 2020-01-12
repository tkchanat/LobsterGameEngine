#pragma once
#include <memory>
#include <mutex>

template <typename T>
class CircularBuffer {
public:
	//	Set constructor to explicit to prevent accidental creation of CircularBuffer.
	explicit CircularBuffer(size_t size) : buffer(std::unique_ptr<T[]>(new T[size])), max_size(size) {}

	//	Inserts an item to the buffer, move to the front if buffer is full.
	void push(T item) {
		std::lock_guard<std::mutex> lock(mutex);

		//	Only modify tail when it is full, and delete original item.
		if (is_full) {
			tail = (tail + 1) % max_size;
			//	TODO: Item not deleted. Manual memory deallocation needed?
		}

		buffer[head] = item;
		head = (head + 1) % max_size;

		//	Check for fullness after end of insertion.
		is_full = (head == tail);
	}

	//	Return the first item we are pointing to and shift head.
	T pop_front() {
		std::lock_guard<std::mutex> lock(mutex);

		//	Returns an empty object when our buffer is empty.
		if (empty()) return T();

		//	After getting a value, our buffer is definitely not full now.
		//	Shift head backward to point to the start of the buffer again.
		is_full = false;
		head = (head + max_size - 1) % max_size;
		auto val = buffer[head];

		return val;
	}

	//	Return the last item we are pointing to and shift tail.
	T pop_back() {
		std::lock_guard<std::mutex> lock(mutex);

		//	Returns an empty object when our buffer is empty.
		if (empty()) return T();

		//	After getting a value, our buffer is definitely not full now.
		//	Shift tail forward to point to end of the buffer again.
		is_full = false;
		auto val = buffer[tail];
		tail = (tail + 1) % max_size;

		return val;
	}

	//	Set everything back to an empty state.
	void reset() {
		std::lock_guard<std::mutex> lock(mutex);
		is_full = false;
		head = tail;
	}

	//	If it is not full, and position of head = tail, then it is empty.
	inline bool empty() const { return !is_full && head == tail; }

	//	We will use member full to indicate whether our container is now full.
	inline bool full() const { return is_full; }

	//	Capacity is denoted by max_size.
	inline size_t capacity() const { return max_size; }

	//	Returns the size of our buffer right now.
	size_t size() const {
		size_t size = max_size;

		//	If it is full, size = max size.
		if (!is_full) {
			if (head >= tail) {
				size = head - tail;
			}
			else {
				size += head - tail;
			}
		}
		return size;
	}

	//	Convert this circular buffer to a vector, where the most recently added item has a smaller index.
	std::vector<T> to_vector() const {
		if (empty()) return std::vector<T>();

		std::vector<T> result(size());
		size_t tail_ptr = tail;
		for (int i = size() - 1; i >= 0; i--) {
			result[i] = buffer[tail_ptr];
			tail_ptr = (tail_ptr + 1) % max_size;
		}
		return result;
	}

private:
	std::mutex mutex;
	std::unique_ptr<T[]> buffer;
	size_t head = 0;
	size_t tail = 0;
	const size_t max_size;
	bool is_full = false;
};
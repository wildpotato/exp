#include <mutex>

template <class T>
class CircularBuffer {
public:
	explicit CircularBuffer(size_t size) :
		_buf(std::unique_ptr<T[]>(new T[size])),
		_maxSz(size)
	{
		// empty
	}

	void put(T item)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_buf[_head] = item;
		if (_isFull)
			_tail = (_tail + 1) % _maxSz;
		_head = (_head + 1) % _maxSz;
		_isFull = _head == _tail;
	}

	T get()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (empty())
			return T();
		auto val = _buf[_tail];
		_isFull = false;
		_tail = (_tail + 1) % _maxSz;
		return val;
	}

	T peekFirstItem()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (empty())
			return T();
		auto val = _buf[0];
		return val;
	}

	T peekLastItem()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!full())
			return T();
		auto val = _buf[_maxSz-1];
		return val;
	}

	void reset()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_head = _tail;
		_isFull = false;
	}

	bool empty() const { return (!_isFull && _head == _tail); }

	bool full() const { return _isFull; }

	size_t capacity() const { return _maxSz; }

	size_t size() const
	{
		size_t size = _maxSz;
		if (!_isFull)
		{
			if (_head >= _tail)
				size = _head - _tail;
			else
				size = _maxSz + _head - _tail;
		}
		return size;
	}

private:
	std::mutex _mutex;
	std::unique_ptr<T[]> _buf;
	size_t _head = 0;
	size_t _tail = 0;
	const size_t _maxSz;
	bool _isFull = 0;
};

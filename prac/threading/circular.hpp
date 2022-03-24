#include <mutex>

template <class T>
class CircularBuffer {
public:
	explicit CircularBuffer(size_t size) :
		_buf(std::unique_ptr<T[]>(new T[size])),
		_maxSz(size) {}

	/* produces element and put onto the ring pointed to by _head pointer */
	void put(T item)
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		_buf[_head] = item;
		if (_isFull)
			_tail = (_tail + 1) % _maxSz;
		_head = (_head + 1) % _maxSz;
		_isFull = _head == _tail;
		return;
	}

	/* consumes the element pointed to by _tail pointer */
	T get()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		if (empty())
			return T();
		auto val = _buf[_tail];
		_isFull = false;
		_tail = (_tail + 1) % _maxSz;
		return val;
	}

	/* read the first element in the ring, DOES NOT modify the ring */
	T peekFirstItem()
	{
		T ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (empty())
			ret = nodata;
		else
			ret = _buf[_head];
		return ret;
	}

	/* read the last element in the ring, DOES NOT modify the ring */
	T peekLastItem()
	{
		T ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (!full())
			ret = nodata;
		else
			ret = _buf[_tail];
		return ret;
	}

	/* read the first and last elemnt in the ring and returns a pair
	 * DOES NOT modify the ring
	 */
	std::pair<T, T> peekFirstNLast()
	{
		std::pair<T, T> ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (size() == 0) {
			ret = {nodata, nodata};
		} else if (size() == 1) {
			ret.first = nodata;
			ret.second = _buf[_tail];
		} else {
			ret.first = _buf[_head];
			ret.second = _buf[_tail];
		}
		return ret;
	}

	void reset()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		_head = _tail;
		_isFull = false;
	}

	/* returns true if ring is empty, false otherwise */
	bool empty()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		return (!_isFull && _head == _tail);
	}

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
	std::mutex _mtx;
	std::unique_ptr<T[]> _buf;
	size_t _head = 0;
	size_t _tail = 0;
	const size_t _maxSz;
	bool _isFull = 0;
	const T nodata = -1;
};

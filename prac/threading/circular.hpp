#include <mutex>
#include <iostream>

using std::cout;

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

	/* consumes the element pointed to by _tail pointer
	 * THIS FUNCTION STILL NEEDS TO BE TESTED BEFORE USE!
	 */
	T get()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		if (_empty())
			return T();
		auto val = _buf[_tail];
		_isFull = false;
		_tail = (_tail + 1) % _maxSz;
		return val;
	}

	/*
	 * read the first item in the buffer
	 * first item is defined as the newest (most recent) item on the queue
	 * DOES NOT modify the buffer
	 */
	T peekFirstItem()
	{
		T ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (_empty())
			return nodata;
		ret = _buf[_getCurrIdx()];
		return ret;
	}

	/*
	 * read the last element in the buffer
	 * last item is defined as the oldest (least recent) item on the queue
	 * DOES NOT modify the buffer
	 */
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

	/*
	 * read the first and last elemnt in the ring and returns a pair
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
			ret.first = _buf[_getCurrIdx()];
			ret.second = _buf[_tail];
		}
		return ret;
	}

	/*
	 * reset the structure back to default state
	 * by resetting _head, _tail, _isFull
	 * DOE NOT actually modify the underlying container
	 */
	void reset()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		_head = _tail;
		_isFull = false;
	}

	/*
	 * print out all elements in the buffer in reverse chrono order along with
	 * the values of _head and _tail
	 * this function is only useful for debugging purposes
	 */
	void printAll()
	{
		std::pair<T, T> ret = peekFirstNLast();
		std::scoped_lock<std::mutex> lck(_mtx);
		for (int i = 0; i < _maxSz; ++i)
		{
			cout << _buf[i] << " ";
		}
		cout << "first=" << ret.first << ",last=" << ret.second << "(head=" <<
			_head << " tail=" << _tail << ")\n";
	}


	bool full() const { return _isFull; }

	size_t capacity() const { return _maxSz; }

	/*
	 * requires _mtx to be acquired first
	 * reurns actual size of buffer (i.e., number of elements available)
	 * DOES NOT modify the buffer
	 */
	size_t size() const
	{
		size_t sz = _maxSz;
		if (!_isFull)
			sz = _head - _tail;
		return sz;
	}

private:
	/* requires that _buf is non-empty and _mtx has been locked
	 * returns the index to the most-recent item (i.e., one before _head)
	 */
	inline size_t _getCurrIdx()
	{
		if (!_empty() && _head == 0)
			return _maxSz-1;
		return _head-1;
	}

	/* requires _mtx has been locked
	 * returns true if ring is empty, false otherwise */
	bool _empty()
	{
		return (!_isFull && _head == _tail);
	}

	std::mutex _mtx;
	std::unique_ptr<T[]> _buf;
	size_t _head = 0;
	size_t _tail = 0;
	const size_t _maxSz;
	bool _isFull = 0;
	const T nodata = T();
};

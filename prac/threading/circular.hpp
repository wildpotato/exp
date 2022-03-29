#include <mutex>
#include <memory>
#include <iostream>

using std::cout;

template <class T>
class CircularBuffer {
/*
 * Description:
 *		This implementation of CircularBuffer DOES NOT support get() function that
 *		removes an item from the buffer. Instead it supports peeking at the first
 *		and last items currently in the buffer.
 *
 * Public APIs:
 *		CircularBuffer(), put(), peekFirstItem(), peekLastItem(), reset(), full(),
 *		capacity(), size()
 */
public:
	explicit CircularBuffer(size_t size) :
		_buf(std::unique_ptr<T[]>(new T[size])),
		_maxSz(size) {}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: Underlying container
	 * @Effect  : Puts item into the buffer, overwrites oldest item if buffer is full
	 */
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

	// This implementation does not support get() function
	// T get() {}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns the last item in the buffer where last item is defined
	 *			  as the newest (most recent) item put into the buffer. If the buffer
	 *			  is currently empty, returns default type value T(). If only one item
	 *			  is currently available, that item will be returned.
	 */
	T peekLastItem()
	{
		T ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (_empty())
			return nodata;
		ret = _buf[_getCurrIdx()];
		return ret;
	}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns the first item in the buffer where first item is defined
	 *			  as the oldest (least recent) item put into the buffer. If the buffer
	 *			  is currently empty, returns default type value T(). If only one item
	 *			  is currently available, that item will be returned.
	 */
	T peekFirstItem()
	{
		T ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (_empty())
			ret = nodata;
		else
			ret = _buf[_tail];
		return ret;
	}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns a pair of <first, last> elements from the buffer. First and
	 *			  and last item are defined the same way as in peekLastItem() and
	 *			  peekFirstItem()
	 */
	std::pair<T, T> peekFirstNLast()
	{
		std::pair<T, T> ret;
		std::scoped_lock<std::mutex> lck(_mtx);
		if (size() == 0) {
			ret = {nodata, nodata};
		} else if (size() == 1) {
			ret.first = _buf[_tail];
			ret.second = _buf[_tail];
		} else {
			ret.first = _buf[_tail];
			ret.second = _buf[_getCurrIdx()];
		}
		return ret;
	}

	/*
	 * Requires: CircularBuffer instance already created
	 * Modifies: Underlying container
	 * Effect  : Resets the buffer back to when it's first created (i.e. same capacity,
	 *			 zero size, etc)
	 */
	void reset()
	{
		std::scoped_lock<std::mutex> lck(_mtx);
		_head = _tail;
		_isFull = false;
	}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns true if buffer is full (size == capacity), false otherwise
	 */
	bool full() const { return _isFull; }

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns the maximum capacity of buffer
	 */
	size_t capacity() const { return _maxSz; }

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Returns the actual size of buffer (i.e., number of elements currently
	 *			  stored in the buffer)
	 */
	size_t size() const
	{
		size_t sz = _maxSz;
		if (!_isFull)
			sz = _head - _tail;
		return sz;
	}

	/*
	 * @Requires: CircularBuffer instance already created
	 * @Modifies: None
	 * @Effect  : Prints out all elements currently in the buffer in reverse chrono order
	 *			  (least recent on the LHS, most recent on RHS) as well as the values of
	 *			  _head and _tail. This function should only be used for debugging purposes.
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

private:
	/*
	 * Returns the index to the most-recently added item (i.e., one before _head)
	 * requires that _buf is non-empty and _mtx has been locked
	 */
	inline size_t _getCurrIdx()
	{
		if (!_empty() && _head == 0)
			return _maxSz-1;
		return _head-1;
	}

	/*
	 * Returns true if ring is empty, false otherwise
	 * requires _mtx has been locked
	 */
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

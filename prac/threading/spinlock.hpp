#include <atomic>

class Spinlock {
	public:
		void lock()
		{
			while (m_flag.test_and_set(std::memory_order_acquire));
		}

		void unlock()
		{
			m_flag.clear(std::memory_order_release);
		}
		Spinlock& operator=(const Spinlock&) = delete;
		Spinlock(const Spinlock&) = delete;
		Spinlock() = default;

	private:
		std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};

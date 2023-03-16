#ifndef SAFE_NUM_H
#define SAFE_NUM_H

#include <atomic>
#include <concepts>

#include "../typedefs.h"

template<typename T>
concept numeric = std::integral<T> or std::floating_point<T>;

template<typename T>
requires numeric<T>
class SafeNum
{
	static_assert(std::atomic<T>::is_always_lock_free);

public:
	typedef T value_type;

	SafeNum(T val)
	{
		m_val = val;
	}

	// Think std::memory_order_acq_rel in the future
	_INLINE_ T increment()
	{
		return m_val.fetch_add(1, std::memory_order_seq_cst) + 1;
	}

	_INLINE_ T post_increment()
	{
		return m_val.fetch_add(1, std::memory_order_seq_cst);
	}

	_INLINE_ T decrement()
	{
		return m_val.fetch_sub(1, std::memory_order_seq_cst) - 1;
	}

	_INLINE_ T post_decrement()
	{
		return m_val.fetch_sub(1, std::memory_order_seq_cst);
	}

	_INLINE_ T add(T val)
	{
		return m_val.fetch_add(val, std::memory_order_seq_cst) + val;
	}

	_INLINE_ T post_add(T val)
	{
		return m_val.fetch_add(val, std::memory_order_seq_cst);
	}

	_INLINE_ T sub(T val)
	{
		return m_val.fetch_sub(val, std::memory_order_seq_cst) - val;
	}

	_INLINE_ T post_sub(T val)
	{
		return m_val.fetch_sub(val, std::memory_order_seq_cst);
	}

	_INLINE_ T mul(T val)
	{
		T beginVal = m_val.load();
		T res = beginVal * val;
		while (!m_val.compare_exchange_weak(beginVal, res));
		
		return res;
	}

	_INLINE_ T post_mul(T val)
	{
		T beginVal = m_val.load();
		while (!m_val.compare_exchange_weak(beginVal, beginVal * val));

		return beginVal;
	}

	_INLINE_ T div(T val)
	{
		T beginVal = m_val.load();
		T res = beginVal / val;
		while (!m_val.compare_exchange_weak(beginVal, res));

		return res;
	}

	_INLINE_ T post_div(T val)
	{
		T beginVal = m_val.load();
		while (!m_val.compare_exchange_weak(beginVal, beginVal / val));

		return beginVal;
	}

private:
	std::atomic<T> m_val;

};
#endif // SAFE_NUM_H
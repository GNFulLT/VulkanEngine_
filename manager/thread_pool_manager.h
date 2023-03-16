#ifndef THREAD_POOL_MANAGER_H
#define THREAD_POOL_MANAGER_H

#include <taskflow/taskflow.hpp>


#include "../core/typedefs.h"
#include "../core/templates/safe_num.h"


#include "system_manager.h"

class ThreadPoolManager : public SystemManager
{
	OBJECT_DEF(ThreadPoolManager, SystemManager)
	SINGLETON(ThreadPoolManager)
public:
	ThreadPoolManager() = default;
	_INLINE_ size_t get_thread_count()
	{
		return m_execPool.num_workers();
	}
	_INLINE_ tf::Future<void> run_flow(tf::Taskflow& tf)
	{
		return m_execPool.run(tf);
	}

	_INLINE_ tf::Taskflow* get_render_taskflow()
	{
		return &m_renderTask;
	}

private:

private:
	tf::Executor m_execPool;
	tf::Taskflow m_renderTask;
	_INLINE_ static SafeNum<uint32_t> working_thread_count = 0;

};


#endif // THREAD_POOL_MANAGER_H
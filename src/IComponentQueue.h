#pragma once
#include <stdint.h>
#include <string>
#include <time.h>
#include <dpi/bits/complex_ctb_types.h>
#include "boost/thread/locks.hpp"
#include "boost/thread/recursive_mutex.hpp"
/**
 * Interface class for interprocess/interthread communication
 * 		     queues
 *
 */

class IComponentQueue
{
public:
	IComponentQueue();
	IComponentQueue(const IComponentQueue& that);
	virtual ~IComponentQueue();
protected:
    boost::recursive_mutex mMutex;
};

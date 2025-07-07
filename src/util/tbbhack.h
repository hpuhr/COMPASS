//#define X  1
//#pragma push_macro("X")
//#undef X
//#define X -1
//#pragma pop_macro("X")
//int x [X];

#ifdef emit

#pragma push_macro("emit")
#undef emit

#include <tbb/tbb.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

#pragma pop_macro("emit")

#else

#include <tbb/tbb.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

#endif


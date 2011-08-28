//
//  ZWRCompatibility.h
//  Uses zeroing weak references on iOS 5.0+ and OS X 10.7+
//  Falls back to using standard unsafe unretained pointers
//  Supports both ARC and standard/old compile environments
//
//  Created by Ryan Petrich on 11-08-28.
//

#ifndef ZWRCompatibility_h
#define ZWRCompatibility_h

#include <objc/runtime.h>

#if __has_feature(objc_arc)
#define __zwrc_weak __unsafe_unretained
#else
#define __zwrc_weak
#endif

static __attribute__((always_inline)) inline id zwrc_load(__zwrc_weak id *storage)
{
    if (&objc_loadWeak != NULL) {
#if __has_feature(objc_arc)
        return objc_loadWeak((__autoreleasing id *)(void *)storage);
#else
        return objc_loadWeak(storage);
#endif
    }
    return *storage;
}

static __attribute__((always_inline)) inline id zwrc_store(__zwrc_weak id *storage, id value)
{
    if (&objc_storeWeak != NULL) {
#if __has_feature(objc_arc)
        return objc_storeWeak((__autoreleasing id *)(void *)storage, value);
#else
        return objc_storeWeak(storage, value);
#endif
    }
    *storage = value;
    return value;
}

static __attribute__((always_inline)) inline void zwrc_destroy(__zwrc_weak id *storage)
{
    zwrc_store(storage, nil);
}

#define ZWRC_SYNTHESIZE_GETTER(getter, ivar, type) \
- (type)getter \
{ \
    return zwrc_load(&ivar); \
}

#define ZWRC_SYNTHESIZE_SETTER(setter, ivar, type) \
- (void)setter(type)newValue \
{ \
    zwrc_store(&ivar, newValue); \
}

#define ZWRC_SYNTHESIZE(getter, setter, ivar, type) \
    ZWRC_SYNTHESIZE_GETTER(getter, ivar, type) \
    ZWRC_SYNTHESIZE_SETTER(setter, ivar, type)

#endif

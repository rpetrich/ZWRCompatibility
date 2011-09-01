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

// Load+Store+Clear

static __attribute__((always_inline)) __attribute__((warn_unused_result)) inline id _zwrc_load(__zwrc_weak id *storage)
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
#define zwrc_load(storage) ((__attribute__((warn_unused_result)) __typeof__(storage)(*)(__zwrc_weak id *))_zwrc_load)(&storage)

static __attribute__((always_inline)) inline id _zwrc_store(__zwrc_weak id *storage, id value)
{
    if (&objc_storeWeak != NULL) {
#if __has_feature(objc_arc)
        objc_storeWeak((__autoreleasing id *)(void *)storage, value);
#else
        objc_storeWeak(storage, value);
#endif
        return value;
    }
    *storage = value;
    return value;
}
#define zwrc_store(storage, value) ((__typeof__(storage)(*)(__zwrc_weak id *, __typeof__(storage)))_zwrc_store)(&storage, value)
#define zwrc_clear(storage) zwrc_store(storage, nil)

// Synthesize Methods

#define ZWRC_SYNTHESIZE_GETTER(getter, ivar, type) \
- (type)getter \
{ \
    return zwrc_load(ivar); \
}

#define ZWRC_SYNTHESIZE_SETTER(setter, ivar, type) \
- (void)setter(type)newValue \
{ \
    zwrc_store(ivar, newValue); \
}

#define ZWRC_SYNTHESIZE(getter, setter, ivar, type) \
    ZWRC_SYNTHESIZE_GETTER(getter, ivar, type) \
    ZWRC_SYNTHESIZE_SETTER(setter, ivar, type)

// Delegate Macros

#define ZWRC_DELEGATE_IVAR(...) \
    __zwrc_weak id<__VA_ARGS__> _zwrc_delegate;

#define ZWRC_DELEGATE_SYNTHESIZE(...) \
    @synthesize delegate = _zwrc_delegate; \
    ZWRC_SYNTHESIZE(delegate, setDelegate:, _zwrc_delegate, id<__VA_ARGS__>)

#define zwrc_delegate zwrc_load(_zwrc_delegate)
#define zwrc_delegate_clear() zwrc_clear(_zwrc_delegate)

#endif
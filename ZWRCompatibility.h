//
//  ZWRCompatibility.h
//  Uses zeroing weak references on iOS 5.0+ and OS X 10.7+
//  Falls back to using standard unsafe unretained pointers
//  or if selected, an associative reference implementation
//  Supports both ARC and standard/old compile environments
//
//  Created by Ryan Petrich on 11-08-28.
//

#ifndef ZWRCompatibility_h
#define ZWRCompatibility_h

#import <objc/runtime.h>
#import <Availability.h>
#import <TargetConditionals.h>

#ifndef ZWRC_USE_THREAD_UNSAFE_FALLBACK
// Fallback zeroing weak references are very thread-unsafe
// only zero after dealloc has run which is dangerous,
// and aren't supported on the iOS Simulator
// For something more complete, use MAZeroingWeakRef
#define ZWRC_USE_THREAD_UNSAFE_FALLBACK 0
#endif

#if TARGET_OS_IPHONE && defined(__IPHONE_5_0) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_5_0)
	#define ZWRC_SDK_SUPPORTS_WEAK 1
#elif TARGET_OS_MAC && defined(__MAC_10_7) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_7)
	#define ZWRC_SDK_SUPPORTS_WEAK 1
#else
	#define ZWRC_SDK_SUPPORTS_WEAK 0
#endif

#if ZWRC_USE_THREAD_UNSAFE_FALLBACK && !TARGET_IPHONE_SIMULATOR
@interface ZWRCFallbackWeakReference : NSObject {
    void **slotToClear;
}
- (id)initWithSlotToClear:(void **)slot;
@end
#endif

#if __has_feature(objc_arc)
#define __zwrc_weak __unsafe_unretained
#else
#define __zwrc_weak
#endif

// Load+Store+Clear

static __attribute__((always_inline)) __attribute__((warn_unused_result)) inline id _zwrc_load(__zwrc_weak id *storage)
{
#if ZWRC_SDK_SUPPORTS_WEAK
    if (&objc_loadWeak != NULL) {
#if __has_feature(objc_arc)
        return objc_loadWeak((__autoreleasing id *)(void *)storage);
#else
        return objc_loadWeak(storage);
#endif
    }
#endif
    return *storage;
}
#define zwrc_load(storage) ((__attribute__((warn_unused_result)) __typeof__(storage)(*)(__zwrc_weak id *))_zwrc_load)(&storage)

static __attribute__((always_inline)) inline id _zwrc_store(__zwrc_weak id *storage, id value)
{
#if ZWRC_SDK_SUPPORTS_WEAK
    if (&objc_storeWeak != NULL) {
#if __has_feature(objc_arc)
        objc_storeWeak((__autoreleasing id *)(void *)storage, value);
#else
        objc_storeWeak(storage, value);
#endif
        return value;
    }
#endif
#if ZWRC_USE_THREAD_UNSAFE_FALLBACK && !TARGET_IPHONE_SIMULATOR
    if (&objc_setAssociatedObject != NULL) {
        if (*storage)
            objc_setAssociatedObject(*storage, storage, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        if (value) {
            ZWRCFallbackWeakReference *wr = [[ZWRCFallbackWeakReference alloc] initWithSlotToClear:(void *)storage];
            objc_setAssociatedObject(value, storage, wr, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            [wr release];
        }
    }
#endif
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

#if !ZWRC_SDK_SUPPORTS_WEAK
#define _zwrc_delegate _zwrn_delegate
#endif

#define ZWRC_DELEGATE_IVAR(...) \
    __zwrc_weak id<__VA_ARGS__> _zwrc_delegate;

#define ZWRC_DELEGATE_SYNTHESIZE(...) \
    @synthesize delegate = _zwrc_delegate; \
    ZWRC_SYNTHESIZE(delegate, setDelegate:, _zwrc_delegate, id<__VA_ARGS__>)

#define zwrc_delegate zwrc_load(_zwrc_delegate)
#define zwrc_delegate_clear() zwrc_clear(_zwrc_delegate)

#endif

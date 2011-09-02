ZWRCompatibility
================
Support Objective-C zeroing weak references with unsafe fallback on non-OS X 10.7/iOS 5.0

**Warning**: References are not zeroing on older OS versions and should be used to provide additional safety only. If you need true zeroing references, use `MAZeroingWeakRef` instead.

Example
-------
```objc
#import "ZWRCompatibility.h"
@protocol FooDelegate <NSObject>
@end

@interface Foo : NSObject {
    __zwr_weak id<FooDelegate> delegate;
}

@property (nonatomic, assign) id<FooDelegate> delegate;

@end

@implementation Foo

- (id<FooDelegate>)delegate
{
    return zwrc_load(delegate);
}

- (void)setDelegate:(id<FooDelegate>)newDelegate
{
    zwrc_store(delegate, newDelegate);
}

- (void)dealloc
{
    zwrc_clear(delegate);
    [super dealloc];
}

@end
```

Simplified Delegate Example
---------------------------
```objc
#import "ZWRCompatibility.h"
@protocol BarDelegate <NSObject>
@end

@interface Bar : NSObject
@property (nonatomic, assign) id<BarDelegate> delegate;
@end

@implementation Bar

ZWRC_DELEGATE_SYNTHESIZE(BarDelegate);

- (void)dealloc
{
    zwrc_delegate_clear();
    [super dealloc];
}

@end
```

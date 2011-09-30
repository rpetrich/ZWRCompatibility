//
//  ZWRCompatibility.m
//  MMClients
//
//  Created by Ryan Petrich on 11-09-06.
//  Copyright 2011 Medialets. All rights reserved.
//

#import "ZWRCompatibility.h"

#if ZWRC_USE_THREAD_UNSAFE_FALLBACK && !TARGET_IPHONE_SIMULATOR

@implementation ZWRCFallbackWeakReference

- (id)initWithSlotToClear:(void **)slot
{
    if ((self = [super init])) {
        slotToClear = slot;
    }
    return self;
}

- (void)dealloc
{
    if (slotToClear)
        *slotToClear = NULL;
    [super dealloc];
}

@end

#endif

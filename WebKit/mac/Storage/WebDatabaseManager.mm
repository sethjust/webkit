/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "WebDatabaseManagerPrivate.h"
#import "WebDatabaseManagerInternal.h"

#import "WebDatabaseTrackerClient.h"
#import "WebSecurityOriginPrivate.h"
#import "WebSecurityOriginInternal.h"

#import <WebCore/DatabaseTracker.h>

using namespace WebCore;

const NSString *WebDatabaseDirectoryDefaultsKey = @"WebDatabaseDirectory";

const NSString *WebDatabaseDisplayNameKey = @"WebDatabaseDisplayNameKey";
const NSString *WebDatabaseExpectedSizeKey = @"WebDatabaseExpectedSizeKey";
const NSString *WebDatabaseUsageKey = @"WebDatabaseUsageKey";

const NSString *WebDatabaseDidModifyOriginNotification = @"WebDatabaseDidModifyOriginNotification";
const NSString *WebDatabaseDidModifyDatabaseNotification = @"WebDatabaseDidModifyDatabaseNotification";

@implementation WebDatabaseManager

+ (WebDatabaseManager *) sharedWebDatabaseManager
{
    static WebDatabaseManager *sharedManager = [[WebDatabaseManager alloc] init];
    return sharedManager;
}

- (NSArray *)origins
{
    Vector<WebCoreSecurityOriginData> coreOrigins;
    DatabaseTracker::tracker().origins(coreOrigins);
    NSMutableArray *webOrigins = [[NSMutableArray alloc] initWithCapacity:coreOrigins.size()];

    for (unsigned i = 0; i < coreOrigins.size(); ++i) {
        WebSecurityOrigin *webOrigin = [[WebSecurityOrigin alloc] _initWithWebCoreSecurityOriginData:&coreOrigins[i]];
        [webOrigins addObject:webOrigin];
        [webOrigin release];
    }

    return [webOrigins autorelease];
}

- (NSArray *)databasesWithOrigin:(WebSecurityOrigin *)origin
{
    Vector<String> nameVector;
    if (!DatabaseTracker::tracker().databaseNamesForOrigin([origin _core], nameVector))
        return nil;
    
    NSMutableArray *names = [[NSMutableArray alloc] initWithCapacity:nameVector.size()];

    for (unsigned i = 0; i < nameVector.size(); ++i)
        [names addObject:(NSString *)nameVector[i]];

    return [names autorelease];
}

- (NSDictionary *)detailsForDatabase:(NSString *)databaseName withOrigin:(WebSecurityOrigin *)origin
{
    static id keys[3] = {WebDatabaseDisplayNameKey, WebDatabaseExpectedSizeKey, WebDatabaseUsageKey};
    
    DatabaseDetails details = DatabaseTracker::tracker().detailsForNameAndOrigin(databaseName, [origin _core]);
    if (!details.isValid())
        return nil;
        
    id objects[3];
    objects[0] = details.displayName().isEmpty() ? databaseName : (NSString *)details.displayName();
    objects[1] = [NSNumber numberWithUnsignedLongLong:details.expectedUsage()];
    objects[2] = [NSNumber numberWithUnsignedLongLong:details.currentUsage()];
    
    return [[[NSDictionary alloc] initWithObjects:objects forKeys:keys count:3] autorelease];
}

- (void)deleteAllDatabases
{
    DatabaseTracker::tracker().deleteAllDatabases();
}

- (void)deleteDatabasesWithOrigin:(WebSecurityOrigin *)origin
{
    SecurityOriginData coreOrigin([origin protocol], [origin domain], [origin port]);

    DatabaseTracker::tracker().deleteDatabasesWithOrigin(coreOrigin);
}

- (void)deleteDatabase:(NSString *)databaseName withOrigin:(WebSecurityOrigin *)origin
{
    SecurityOriginData coreOrigin([origin protocol], [origin domain], [origin port]);
    
    DatabaseTracker::tracker().deleteDatabase(coreOrigin, databaseName);
}
@end

void WebKitInitializeDatabasesIfNecessary()
{
    static BOOL initialized = NO;
    if (initialized)
        return;

    // Set the database root path in WebCore
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    NSString *databasesDirectory = [defaults objectForKey:WebDatabaseDirectoryDefaultsKey];
    if (!databasesDirectory || ![databasesDirectory isKindOfClass:[NSString class]])
        databasesDirectory = @"~/Library/WebKit/Databases";

    DatabaseTracker::tracker().setDatabasePath([databasesDirectory stringByStandardizingPath]);

    // Set the DatabaseTrackerClient
    DatabaseTracker::tracker().setClient(WebDatabaseTrackerClient::sharedWebDatabaseTrackerClient());
    
    initialized = YES;
}

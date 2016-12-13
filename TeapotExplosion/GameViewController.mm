//
//  GameViewController.m
//  TeapotExplosion
//
//  Created by James Folk on 12/6/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#import "GameViewController.h"
#import <OpenGLES/ES2/glext.h>

#include "World.hpp"

#include <pthread.h>

struct thread_data{
    float timeSinceLastUpdate;
    float aspect;
};

@interface GameViewController ()
{
    struct thread_data threadData;
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
- (uint64_t)freeDiskspace;

@end

@implementation GameViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    jamesfolk::World::createInstance();
    
    NSMutableString *path = [[NSMutableString alloc] initWithString:[[NSBundle mainBundle] resourcePath]];
    [path appendString: [[NSString alloc] initWithCString:"/assets/"
                                                 encoding:NSASCIIStringEncoding]];
    jamesfolk::World::setBundlePath([path UTF8String]);
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)dealloc
{    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    jamesfolk::World::destroyInstance();
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    jamesfolk::World::getInstance()->create();
}

- (void)tearDownGL
{
    jamesfolk::World::getInstance()->destroy();
    
    [EAGLContext setCurrentContext:self.context];
}

- (uint64_t)freeDiskspace
{
    uint64_t totalSpace = 0;
    uint64_t totalFreeSpace = 0;
    
    __autoreleasing NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSDictionary *dictionary = [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error: &error];
    
    if (dictionary) {
        NSNumber *fileSystemSizeInBytes = [dictionary objectForKey: NSFileSystemSize];
        NSNumber *freeFileSystemSizeInBytes = [dictionary objectForKey:NSFileSystemFreeSize];
        totalSpace = [fileSystemSizeInBytes unsignedLongLongValue];
        totalFreeSpace = [freeFileSystemSizeInBytes unsignedLongLongValue];
        NSLog(@"Memory Capacity of %llu MiB with %llu MiB Free memory available.", ((totalSpace/1024ll)/1024ll), ((totalFreeSpace/1024ll)/1024ll));
    } else {
        NSLog(@"Error Obtaining System Memory Info: Domain = %@, Code = %ld", [error domain], (long)[error code]);
    }
    
    return totalFreeSpace;
}

-(void)viewDidLayoutSubviews
{
    GLKView *view = (GLKView *)self.view;
    
    jamesfolk::World::getInstance()->resize(0, 0,
                                            view.frame.size.width * view.contentScaleFactor,
                                            view.frame.size.height * view.contentScaleFactor);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->addTouch(jamesfolk::World::TouchState_Down,
                                                  btVector2(point.x * touch.view.contentScaleFactor,
                                                            point.y * touch.view.contentScaleFactor));
    }
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->addTouch(jamesfolk::World::TouchState_Move,
                                                  btVector2(point.x * touch.view.contentScaleFactor,
                                                            point.y * touch.view.contentScaleFactor));
    }
}


- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->addTouch(jamesfolk::World::TouchState_Up,
                                                  btVector2(point.x * touch.view.contentScaleFactor,
                                                            point.y * touch.view.contentScaleFactor));
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->addTouch(jamesfolk::World::TouchState_Cancelled,
                                                  btVector2(point.x * touch.view.contentScaleFactor,
                                                            point.y * touch.view.contentScaleFactor));
    }
}

#pragma mark - GLKView and GLKViewController delegate methods

void *_update(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    
    jamesfolk::World::getInstance()->update(my_data->timeSinceLastUpdate);
    
    pthread_exit(NULL);
}

- (void)update
{
//    jamesfolk::World::getInstance()->update(self.timeSinceLastUpdate);
    
    threadData.timeSinceLastUpdate = self.timeSinceLastUpdate;
    
    pthread_t threads;
    int rc;
    rc = pthread_create(&threads, NULL, _update, (void *)&threadData);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    if(pthread_join(threads, NULL))
    {
        fprintf(stderr, "Error joining thread\n");
        exit(-1);
    }

}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    jamesfolk::World::getInstance()->render();
}

@end

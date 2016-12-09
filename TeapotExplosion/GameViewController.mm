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

@interface GameViewController ()

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

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
        jamesfolk::World::getInstance()->touch(jamesfolk::World::TouchState_Down,
                                               point.x * touch.view.contentScaleFactor,
                                               point.y * touch.view.contentScaleFactor);
    }
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->touch(jamesfolk::World::TouchState_Move,
                                               point.x * touch.view.contentScaleFactor,
                                               point.y * touch.view.contentScaleFactor);
    }
}


- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->touch(jamesfolk::World::TouchState_Up,
                                               point.x * touch.view.contentScaleFactor,
                                               point.y * touch.view.contentScaleFactor);
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    for(UITouch *touch in touches)
    {
        CGPoint point = [touch locationInView:touch.view];
        jamesfolk::World::getInstance()->touch(jamesfolk::World::TouchState_Cancelled,
                                               point.x * touch.view.contentScaleFactor,
                                               point.y * touch.view.contentScaleFactor);
    }
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    jamesfolk::World::getInstance()->update(self.timeSinceLastUpdate);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    jamesfolk::World::getInstance()->render();
}

@end

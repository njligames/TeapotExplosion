//
//  SettingsViewController.m
//  TeapotExplosion
//
//  Created by James Folk on 12/13/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#import "SettingsViewController.h"

#include "World.hpp"
#include "Geometry.hpp"

#import "FCColorPickerViewController.h"

@interface SettingsViewController () <FCColorPickerViewControllerDelegate>
@property (weak, nonatomic) IBOutlet UIButton *explodeResetTeapotButton;
@property (weak, nonatomic) IBOutlet UIButton *tesselateTeapotButton;
@property (weak, nonatomic) IBOutlet UIStepper *teapotCountStepper;
@property (weak, nonatomic) IBOutlet UILabel *teapotCountLabel;


@property (nonatomic, copy) UIColor *color;

@end

@implementation SettingsViewController

- (IBAction)explodeResetTeapot:(id)sender
{
    if(jamesfolk::World::getInstance()->isExploding())
    {
        jamesfolk::World::getInstance()->resetTeapots();
        [[self explodeResetTeapotButton] setTitle:@"Explode Teapot" forState:UIControlStateNormal];
    }
    else
    {
        jamesfolk::World::getInstance()->explodeTeapots();
        [[self explodeResetTeapotButton] setTitle:@"Reset Explosion" forState:UIControlStateNormal];
    }
}

- (IBAction)TesselateTeapot:(id)sender
{
    if(jamesfolk::World::getInstance()->isMaxTesselations())
    {
        
    }
    else
    {
        jamesfolk::World::getInstance()->subdivideTeapots();
        if(jamesfolk::World::getInstance()->isMaxTesselations())
        {
            [[self tesselateTeapotButton] setTitle:@"Max Tesselation" forState:UIControlStateNormal];
        }
    }
    [[self explodeResetTeapotButton] setTitle:@"Explode Teapot" forState:UIControlStateNormal];
    
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"Teapot Settings";
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

//- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
//#warning Incomplete implementation, return the number of sections
//    return 0;
//}
//
//- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
//#warning Incomplete implementation, return the number of rows
//    return 0;
//}

/*
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:<#@"reuseIdentifier"#> forIndexPath:indexPath];
    
    // Configure the cell...
    
    return cell;
}
*/

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

#pragma mark - FCColorPickerViewControllerDelegate Methods

-(void)colorPickerViewController:(FCColorPickerViewController *)colorPicker didSelectColor:(UIColor *)color {
    self.color = color;
    
    CGFloat red, green, blue, alpha;
    
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    btVector3 c(red, green, blue);
    
    switch(colorSetting)
    {
        case ColorSetting_RimColor:
            jamesfolk::World::getInstance()->getGeometry()->setRimLightColor(c);
            break;
        case ColorSetting_LightSourceAmbientColor:
            jamesfolk::World::getInstance()->getGeometry()->setLightSourceAmbientColor(c);
            break;
        case ColorSetting_LightSourceDiffuseColor:
            jamesfolk::World::getInstance()->getGeometry()->setLightSourceDiffuseColor(c);
            break;
        case ColorSetting_LightSourceSpecularColor:
            jamesfolk::World::getInstance()->getGeometry()->setLightSourceSpecularColor(c);
            break;
        case ColorSetting_LightAmbientColor:
            jamesfolk::World::getInstance()->getGeometry()->setLightAmbientColor(c);
            break;
        case ColorSetting_FogColor:
            jamesfolk::World::getInstance()->getGeometry()->setFogColor(c);
            break;
        default:
            break;
    }
    [self dismissViewControllerAnimated:YES completion:nil];
}

-(void)colorPickerViewControllerDidCancel:(FCColorPickerViewController *)colorPicker {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)handleRimSettings:(NSInteger)row
{
    switch (row)
    {
        case 0:
        {
            colorSetting = ColorSetting_RimColor;
            
            //color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getRimLightColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
        case 1:
        {
            //start
            float v = jamesfolk::World::getInstance()->getGeometry()->getRimLightStart();
        }
            break;
        case 2:
        {
            //end
            float v = jamesfolk::World::getInstance()->getGeometry()->getRimLightEnd();
        }
            break;
        case 3:
        {
            //coefficient
            float v = jamesfolk::World::getInstance()->getGeometry()->getRimLightCoefficient();
        }
            break;
            
        default:
            break;
    }
}

- (void)handleLightSourceSettings:(NSInteger)row
{
    
    switch (row)
    {
        case 0:
        {
            colorSetting = ColorSetting_LightAmbientColor;
            
            //ambient color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getLightSourceAmbientColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
        case 1:
        {
            colorSetting = ColorSetting_LightSourceDiffuseColor;
            
            //Diffuse color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getLightSourceDiffuseColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
        case 2:
        {
            colorSetting = ColorSetting_LightSourceSpecularColor;
            
            //Specular Color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getLightSourceSpecularColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
        case 3:
        {
            //Position
            btVector4 position(jamesfolk::World::getInstance()->getGeometry()->getLightSourcePosition());
        }
            break;
        case 4:
        {
            //Spot Direction
            btVector3 dir(jamesfolk::World::getInstance()->getGeometry()->getLightSourceSpotDirection());
        }
            break;
        case 5:
        {
            //Spot Exponent
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceSpotExponent();
        }
            break;
        case 6:
        {
            //Spot Cutoff
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceSpotCutoff();
        }
            break;
        case 7:
        {
            //SpotCosCutoff
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceSpotCosCutoff();
        }
            break;
        case 8:
        {
            //Constant Attenuation
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceConstantAttenuation();
        }
            break;
        case 9:
        {
            //Linear Attenuation
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceLinearAttenuation();
        }
            break;
        case 10:
        {
            //Qudradic Attenuation
            float v = jamesfolk::World::getInstance()->getGeometry()->getLightSourceQuadraticAttenuation();
        }
            break;
            
        default:
            break;
    }
}

- (void)handleLightSettings:(NSInteger)row
{
    switch (row)
    {
        case 0:
        {
            colorSetting = ColorSetting_LightAmbientColor;
            
            //ambient color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getLightAmbientColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
            
        default:
            break;
    }
}

- (void)handleMaterialSettings:(NSInteger)row
{
    switch (row)
    {
        case 0:
        {
            //shininess
            float v = jamesfolk::World::getInstance()->getGeometry()->getMaterialShininess();
        }
            break;
            
        default:
            break;
    }
}

- (void)handleFogSettings:(NSInteger)row
{
    switch (row)
    {
        case 0:
        {
            //max distance
            float v = jamesfolk::World::getInstance()->getGeometry()->getFogMaxDistance();
        }
            break;
        case 1:
        {
            //min distance
            float v = jamesfolk::World::getInstance()->getGeometry()->getFogMinDistance();
        }
            break;
        case 2:
        {
            colorSetting = ColorSetting_FogColor;
            
            //color
            btVector3 c(jamesfolk::World::getInstance()->getGeometry()->getFogColor());
            
            UIColor *color = [[UIColor alloc] initWithRed:c.x() green:c.y() blue:c.z() alpha:1.0f];
            
            FCColorPickerViewController *colorPicker = [FCColorPickerViewController colorPickerWithColor:self.color
                                                                                                delegate:self];
            colorPicker.tintColor = [UIColor whiteColor];
            colorPicker.color = color;
            
            [colorPicker setModalPresentationStyle:UIModalPresentationFormSheet];
            [self presentViewController:colorPicker
                               animated:YES
                             completion:nil];
        }
            break;
        case 3:
        {
            //density
            float v = jamesfolk::World::getInstance()->getGeometry()->getFogDensity();
        }
            break;
            
        default:
            break;
    }
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSInteger row = [indexPath row];
    NSInteger section = [indexPath section];
    NSLog(@"%ld %ld", (long)row, (long)section);
    
    switch (section)
    {
        case 1:
        {
            //rim
            [self handleRimSettings:row];
        }
            break;
        case 2:
        {
            //light source
            [self handleLightSourceSettings:row];
        }
            break;
        case 3:
        {
            //light
            [self handleLightSettings:row];
        }
            break;
        case 4:
        {
            //material
            [self handleMaterialSettings:row];
        }
            break;
        case 5:
        {
            //fog
            [self handleFogSettings:row];
        }
            break;
            
        default:
            break;
    }
}
@end

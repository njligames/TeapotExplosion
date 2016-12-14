//
//  SettingsViewController.h
//  TeapotExplosion
//
//  Created by James Folk on 12/13/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

enum ColorSetting
{
    ColorSetting_RimColor,
    ColorSetting_LightSourceAmbientColor,
    ColorSetting_LightSourceDiffuseColor,
    ColorSetting_LightSourceSpecularColor,
    ColorSetting_LightAmbientColor,
    ColorSetting_FogColor
};

#import <UIKit/UIKit.h>
@interface SettingsViewController : UITableViewController
{
    ColorSetting colorSetting;
    
    
}

@end

//
//  Shader.fsh
//  TeapotExplosion
//
//  Created by James Folk on 12/6/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}

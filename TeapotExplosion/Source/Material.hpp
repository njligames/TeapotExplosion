//
//  Material.hpp
//  TeapotExplosion
//
//  Created by James Folk on 12/8/16.
//  Copyright © 2016 NJLIGames Ltd. All rights reserved.
//

#ifndef Material_hpp
#define Material_hpp

namespace jamesfolk
{
    class Material
    {
    public:
        Material();
        virtual ~Material();
    protected:
        
    private:
        Material(const Material &copy);
        const Material &operator=(const Material &rhs);
    };
}

#endif /* Material_hpp */

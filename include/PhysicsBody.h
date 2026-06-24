#pragma once
#include "Entity.h"

class PhysicsBody : public Entity {             //"physicsbody is an entity, inheriting everything public from it" 
                                                //physicsbody now automatically has position, the virtual destructor, and the obligation to define update/draw
    public:
        Vector3 velocity{0.0f,0.0f,0.0f};      //the velocity vector
        //Now we introduce wind, which accerlates the ball mid-flight one way or another
        Vector3 windAcceleration{0.0f, 0.0f, 0.0f};
        float mass {1.0f};                     // mass in kg, default 1kg


        void Update(float dt) override;       //declared here, defined in PhysicsBody.cpp //dt is unit time, at 60FPS dt becomes 0.016 seconds, thats how often the frame updates.
                                                //because its a generic physics body, i do not declare or define draw, which stays = 0 as per the entity definition (unimplemented) We will draw the projectile after this.
        void GenerateWind(); //because wind will be randomly generated each shot, we need to define this behavior here.
        float getSpeed() const; //magnitude of the velocity vector, in m/s
        
        
}; //after class definitions you need ;, this was a foolish source of constant errors.


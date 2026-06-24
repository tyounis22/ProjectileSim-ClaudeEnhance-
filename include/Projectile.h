#pragma once 
#include "PhysicsBody.h" //inherits from physics body which inherits from entity.

class Projectile : public PhysicsBody { //says projectile is a class that inherits everything public from PhysicsBody
                                        //projectile gets position (from entity), velocity + mass + update method from physicsBody
                                        //why do this? So that when we code projectile we only have to write ball-specific things.
public:
    float radius{0.3f};    //sphere radius in meters, can change
    Color color{DARKGRAY}; //color of the projectile, can change
    bool active{false}; //is the ball currently flying? makes sure it starts dormant. The logic for this lives in main
    void Draw() override; //implements the last pure virtual, now class is concrete. 
                          //Think of virtual from entity.h as deferring the definition 
                          //of certain functions to when a subclass does so, allowing 
                          //different uses depending on the object. 
};




#pragma once 
#include "PhysicsBody.h"

class Debris : public PhysicsBody {
public:
    float radius{0.5f}; //cube length in meters, change based on visuals
    Color color{255,20,147,255}; //hot pink debris, change based on visuals

    void Draw() override; //notice I didn't add update, because this is handled in physicsbody! nifty reason why we used inheritance.
};

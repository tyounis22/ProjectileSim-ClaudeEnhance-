#include "PhysicsBody.h"
#include "Constants.h"
#include <cmath>

using namespace Constants;

void PhysicsBody::Update(float dt) {

    velocity.y += GRAVITY * dt;     //gravity accelerates velocity downwards, changing it every dt
    //Wind effect on velocity in x and z directions
    velocity.x += windAcceleration.x * dt; //wind pushing sideways (x)
    velocity.z += windAcceleration.z * dt; //wind pushing sideways (z)



    position.y += velocity.y * dt;  //velocity moves the position, changing it every dt
    position.x += velocity.x * dt; // both x and z drift at constant speed determined at launch, since no acceleration force is on x or z, only on y.
                                       // this will change if we decide to add drag later, and will depend on the radius of the ball.
    position.z += velocity.z * dt;

    //the loop below is what prevents it from going through the floor, so it bounces
    if (position.y < 1.0f) {
        position.y = 1.0f;
        velocity.y = -velocity.y * 0.7f; //reverse vertical velocity, keep 70%, lose 30% to the bounce
    }

}

float PhysicsBody::getSpeed() const {
    return sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z); //pythagorean theorem in 3D, gives speed regardless of direction
}

void PhysicsBody::GenerateWind() {
    do {
        float angle = GetRandomValue(0,359) * DEG2RAD; //random compass direction, GetRandomValue is raylib built in random number generator, cosf, sinf from cmath only work in radians so we confvert.
        float strength = GetRandomValue(0,100) / 10.0f;
        windAcceleration.x = cosf(angle) * strength;          // direction -> X component
        windAcceleration.z = sinf(angle) * strength;          // direction -> Z component
        windAcceleration.y = 0.0f;   
    } while (windAcceleration.x == 0.0f && windAcceleration.z == 0.0f); //keep generating until we get a non-zero wind vector
}



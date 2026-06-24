#pragma once 
#include "raylib.h" //need this here because we need vector3 class which lives in raylib

class Entity {
    public: 
        Vector3 position{0.0f,0.0f,0.0f}; //every entity has a location in our world

        virtual ~Entity() = default;      //virtual destructor, super important, without it, only entity will be deleted, not everything built on it. 

        virtual void Update(float dt) = 0; //virtual means will be defined in subclasses
        virtual void Draw() = 0;           //same as above. Eric when you create a target class, which inherits from entity, you will define draw specifically for targets. Setting it to 0 here is just kicking the can to the subclasses. My draw in projectile will be different than yours in target.
                                           //virtual defers the decision of what draw or update is acting upon until runtime, ensuring when we do Entity::Update it updates entity, and Projectile::Update it updates projectile. Without virtual, cpp decides which to do at compile time using only the declared type, which is Entity, so it will ALWAYS do Entity::update, even when we want Projectile::update
                                           
                                           // note: no function bodies in entity itself, so we dont need an entity.cpp entity is purely virtual.


};
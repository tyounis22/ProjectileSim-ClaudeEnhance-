#include "Projectile.h"

#include "Projectile.h"

void Projectile::Draw() {
    DrawSphere(position,radius,color); //draws sphere at current position, all variables previously defined.
                                       //position updates based on the integration stuff going on in physicsbody
                                       //ready for a test fire 
}
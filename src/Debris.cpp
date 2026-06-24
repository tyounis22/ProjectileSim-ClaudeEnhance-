#include "Debris.h"

void Debris::Draw() {
    // low rings/slices (matches the wireframe) instead of DrawSphere's default 16x16 mesh,
    // which raylib regenerates from scratch every frame for every piece
    DrawSphereEx(position, radius, 6, 6, color);
    DrawSphereWires(position, radius, 6, 6, BLACK);   // low rings/slices = visible facets

}
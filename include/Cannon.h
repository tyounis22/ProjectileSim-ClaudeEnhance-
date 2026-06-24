#pragma once
#include "raylib.h"
#include "Projectile.h"
#include "Constants.h"
#include "Target.h"

using namespace Constants;

// Hey Sid, read this first, this is where a lot of your work will be, connecting the cannon to key instructions
// Right now it just loads the cannon pointing straight (azimuth 0) and at a 30 degree angle, power is 20 m/s
// You will code stuff that writes to azimuth / elevation / power from the users input.
// The barrel and the launch both read these — nothing else to wire up.
// To fire: call cannon.Fire(ball) on spacebar release.
// you'll have to do the key input stuff in the while loop in main, so it updates the position of the cannon every dt based on keyboard inputs

class Cannon {
public:
    Vector3 AimDirection() const;       // (azimuth, elevation) -> unit direction vector
    Vector3 getPivot() const;
    float getLaunchSpeed() const;
    void incrAzimuth(float frameTime);
    void decrAzimuth(float frameTime);
    void incrElevation(float frameTime);
    void decrElevation(float frameTime);
    void incrLaunchSpeed(float frameTime);
    void Draw() const;                  // draws base + barrel along current aim
    void Fire(Projectile& ball);        // launches ball along aim at power
    void Update(float dt);              // recoil spring-damper + flash countdown
    Vector3 GetMuzzlePos() const;       // world position of barrel tip
    void Reset();                       // restore launch speed, clear recoil

private:
    static constexpr float ROTATION_RATE = 30.0f;   // degrees/sec for azimuth + elevation incr/decr
    Vector3 pivot   = {0.0f, 1.0f, 0.0f};   // barrel hinge + launch point
    float _azimuth      = 0.0f;     // left/right, degrees, range -90 to 90
    float _elevation    = 25.0f;    // up/down, degrees, range 0 to 85
    float _launchSpeed  = PMIN;     // launch speed m/s, range 0 to 100 (spacebar hold)
    float _recoilOffset   {0.0f};   // barrel pushed back along aim direction (negative)
    float _recoilVelocity {0.0f};   // spring-damper velocity for recoil
    int   _flashFrames    {0};      // frames remaining for muzzle flash
    float _wheelAngle     {0.0f};   // accumulated wheel rotation in degrees (spins with azimuth)
};
#include "Cannon.h"
#include "raylib.h"
#include "rlgl.h"
#include <cmath>   // cosf, sinf, sqrtf, fabsf
#include "Constants.h"

using namespace Constants;

Vector3 Cannon::getPivot() const { return pivot; }
float   Cannon::getLaunchSpeed() const { return _launchSpeed; }

void Cannon::incrAzimuth(float frameTime) {
    float temp = _azimuth + ROTATION_RATE * frameTime;
    if (temp >= AMIN && temp <= AMAX) {
        _azimuth     = temp;
        _wheelAngle += ROTATION_RATE * frameTime * 3.0f;
    }
}

void Cannon::decrAzimuth(float frameTime) {
    float temp = _azimuth - ROTATION_RATE * frameTime;
    if (temp >= AMIN && temp <= AMAX) {
        _azimuth     = temp;
        _wheelAngle -= ROTATION_RATE * frameTime * 3.0f;
    }
}

void Cannon::incrElevation(float frameTime) {
    float temp = _elevation + ROTATION_RATE * frameTime;
    if (temp >= EMIN && temp <= EMAX) _elevation = temp;
}

void Cannon::decrElevation(float frameTime) {
    float temp = _elevation - ROTATION_RATE * frameTime;
    if (temp >= EMIN && temp <= EMAX) _elevation = temp;
}

void Cannon::incrLaunchSpeed(float frameTime) {
    float temp = _launchSpeed + 60.0f * frameTime;
    if (temp >= PMIN && temp <= PMAX) _launchSpeed = temp;
}

Vector3 Cannon::AimDirection() const {
    float az = _azimuth   * DEG2RAD;
    float el = _elevation * DEG2RAD;
    return {
        cosf(el) * cosf(az),
        sinf(el),
        cosf(el) * sinf(az)
    };
}

void Cannon::Draw() const {
    const Color woodColor   = {139,  90,  43, 255};
    const Color darkWood    = {110,  70,  30, 255};
    const Color ironColor   = { 55,  55,  58, 255};
    const Color darkIron    = { 35,  35,  38, 255};
    const Color bronzeColor = {180, 120,  40, 255};

    // ── Carriage + Wheels (rotate with azimuth only) ──────────────────────
    rlPushMatrix();
    rlRotatef(-_azimuth, 0.0f, 1.0f, 0.0f);

    // Main carriage bed (horizontal plank floor)
    DrawCube({0.40f, 0.88f, 0.0f},  2.2f, 0.26f, 0.70f, woodColor);
    // Front limber brace (vertical cross-piece at the muzzle end)
    DrawCube({1.30f, 0.98f, 0.0f},  0.40f, 0.44f, 1.18f, darkWood);
    // Cheek beams — tall side planks that cradle the barrel
    DrawCube({0.20f, 1.18f,  0.40f}, 1.80f, 0.58f, 0.12f, darkWood);
    DrawCube({0.20f, 1.18f, -0.40f}, 1.80f, 0.58f, 0.12f, darkWood);
    // Iron bolts through the cheeks (small cubes for detail)
    DrawCube({-0.4f, 1.18f,  0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    DrawCube({ 0.0f, 1.18f,  0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    DrawCube({ 0.6f, 1.18f,  0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    DrawCube({-0.4f, 1.18f, -0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    DrawCube({ 0.0f, 1.18f, -0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    DrawCube({ 0.6f, 1.18f, -0.47f}, 0.08f, 0.08f, 0.05f, ironColor);
    // Trunnion brackets (U-clamps that hold the barrel at the pivot)
    DrawCube({0.05f, 1.48f,  0.40f}, 0.38f, 0.15f, 0.08f, ironColor);
    DrawCube({0.05f, 1.48f, -0.40f}, 0.38f, 0.15f, 0.08f, ironColor);
    // Axle connecting both wheels
    DrawCylinderEx({0.0f, 0.90f, -1.35f}, {0.0f, 0.90f, 1.35f}, 0.09f, 0.09f, 10, darkIron);

    // ── Wheels ────────────────────────────────────────────────────────────
    const float wR = 0.90f;
    for (int side = -1; side <= 1; side += 2) {
        float wz = side * 1.28f;

        rlPushMatrix();
        rlTranslatef(0.0f, wR, wz);                         // wheel center
        rlRotatef(_wheelAngle, 0.0f, 0.0f, 1.0f);           // spin around axle (local Z)

        // 8 wooden spokes radiating in the local XY plane
        for (int s = 0; s < 8; s++) {
            float ang = s * (3.14159265f / 4.0f);
            float xa = sinf(ang) * wR * 0.88f;
            float ya = cosf(ang) * wR * 0.88f;
            DrawCylinderEx({0.0f, 0.0f, 0.0f}, {xa, ya, 0.0f}, 0.045f, 0.030f, 6, darkWood);
        }
        // Hub cap
        DrawCylinderEx({0.0f, 0.0f, -0.13f}, {0.0f, 0.0f, 0.13f}, 0.13f, 0.13f, 10, darkIron);

        // Outer iron rim (polygon ring, 18 segments)
        const int rimN = 18;
        for (int s = 0; s < rimN; s++) {
            float a1 = s       * (6.28318530f / rimN);
            float a2 = (s + 1) * (6.28318530f / rimN);
            Vector3 p1 = {sinf(a1) * wR, cosf(a1) * wR, 0.0f};
            Vector3 p2 = {sinf(a2) * wR, cosf(a2) * wR, 0.0f};
            DrawCylinderEx(p1, p2, 0.055f, 0.055f, 5, darkIron);
        }

        rlPopMatrix();
    }

    rlPopMatrix(); // end azimuth rotation

    // ── Multi-section barrel (positioned geometrically for azimuth+elevation) ──
    Vector3 dir  = AimDirection();
    float   rOff = _recoilOffset;

    // Helper: world position at fraction t along barrel axis (with recoil)
    auto along = [&](float t) -> Vector3 {
        return {
            pivot.x + dir.x * (rOff + t),
            pivot.y + dir.y * (rOff + t),
            pivot.z + dir.z * (rOff + t)
        };
    };

    // Cascabel button at the breech end
    DrawSphere(along(-0.68f), 0.20f, ironColor);
    // Neck between cascabel and breech body
    DrawCylinderEx(along(-0.55f), along(-0.68f), 0.19f, 0.15f, 12, bronzeColor);
    // Breech section (thick body)
    DrawCylinderEx(along(-0.55f), along( 0.72f), 0.36f, 0.30f, 16, bronzeColor);
    // Reinforcing band #1 — breech/chase junction
    DrawCylinderEx(along( 0.65f), along( 0.82f), 0.38f, 0.38f, 16, darkIron);
    // Chase section (gradual taper)
    DrawCylinderEx(along( 0.82f), along( 2.00f), 0.29f, 0.23f, 16, bronzeColor);
    // Reinforcing band #2 — mid-chase
    DrawCylinderEx(along( 1.30f), along( 1.44f), 0.30f, 0.30f, 16, darkIron);
    // Muzzle section
    DrawCylinderEx(along( 2.00f), along( 2.70f), 0.23f, 0.21f, 16, bronzeColor);
    // Reinforcing band #3 — just behind muzzle bell
    DrawCylinderEx(along( 2.50f), along( 2.62f), 0.27f, 0.27f, 16, darkIron);
    // Muzzle bell (flared)
    DrawCylinderEx(along( 2.70f), along( 3.00f), 0.21f, 0.38f, 16, bronzeColor);
    // Dark bore
    DrawSphere(along(3.02f), 0.12f, {12, 12, 12, 255});

    // Muzzle flash on fire
    if (_flashFrames > 0) {
        DrawSphere(along(3.0f), 0.62f, {255, 200,  50, 220});
        DrawSphere(along(3.0f), 0.33f, {255, 255, 180, 255});
    }

    // ── Fuse ──────────────────────────────────────────────────────────────
    // Barrel-perpendicular direction pointing skyward (Gram-Schmidt vs world up)
    float    proj   = dir.y;  // dot(dir, {0,1,0})
    Vector3  perpUp = { -dir.x * proj,  1.0f - proj * proj,  -dir.z * proj };
    float    pLen   = sqrtf(perpUp.x*perpUp.x + perpUp.y*perpUp.y + perpUp.z*perpUp.z);
    if (pLen > 0.001f) { perpUp.x /= pLen; perpUp.y /= pLen; perpUp.z /= pLen; }

    // Touchhole nub at ~15% of barrel length, on the topside of the breech
    Vector3 base = along(0.12f);
    Vector3 th   = { base.x + perpUp.x * 0.38f,
                     base.y + perpUp.y * 0.38f,
                     base.z + perpUp.z * 0.38f };
    DrawSphere(th, 0.075f, darkIron);

    // Fuse rope: full-length when uncharged, shrinks as power builds
    float fuseLen = 0.55f * (1.0f - _launchSpeed / PMAX);
    if (fuseLen > 0.02f) {
        Vector3 fuseEnd = {th.x, th.y - fuseLen, th.z};
        DrawCylinderEx(th, fuseEnd, 0.025f, 0.022f, 5, {160, 130, 80, 255});

        // Ember at the tip: always a dim glow; intensifies + flickers while charging
        float flicker = 0.0f;
        if (_launchSpeed > PMIN + 1.0f) {
            flicker = ((sinf((float)GetTime() * 22.0f) + 1.0f) * 0.5f);
        }
        Color emberOut = {
            255,
            (unsigned char)(55 + (int)(flicker * 105)),
            8,
            (unsigned char)(185 + (int)(flicker * 70))
        };
        Color emberIn = {
            255,
            255,
            (unsigned char)(70 + (int)(flicker * 185)),
            255
        };
        DrawSphere(fuseEnd, 0.065f + flicker * 0.035f, emberOut);
        DrawSphere(fuseEnd, 0.038f, emberIn);
    }
}

void Cannon::Fire(Projectile& ball) {
    Vector3 dir  = AimDirection();
    ball.position = pivot;
    ball.velocity = { dir.x * _launchSpeed,
                      dir.y * _launchSpeed,
                      dir.z * _launchSpeed };
    _launchSpeed    = 0.0f;
    _recoilOffset   = -0.5f;
    _recoilVelocity = -3.0f;
    _flashFrames    = 3;
}

void Cannon::Update(float dt) {
    if (_flashFrames > 0) _flashFrames--;

    // Spring-damper recoil back to rest
    const float springK = 50.0f;
    const float dampC   = 12.0f;
    float acc = -springK * _recoilOffset - dampC * _recoilVelocity;
    _recoilVelocity += acc * dt;
    _recoilOffset   += _recoilVelocity * dt;

    if (fabsf(_recoilOffset) < 0.001f && fabsf(_recoilVelocity) < 0.001f) {
        _recoilOffset   = 0.0f;
        _recoilVelocity = 0.0f;
    }
}

Vector3 Cannon::GetMuzzlePos() const {
    Vector3 dir = AimDirection();
    return {
        pivot.x + dir.x * (_recoilOffset + 3.0f),
        pivot.y + dir.y * (_recoilOffset + 3.0f),
        pivot.z + dir.z * (_recoilOffset + 3.0f)
    };
}

void Cannon::Reset() {
    _launchSpeed    = PMIN;
    _recoilOffset   = 0.0f;
    _recoilVelocity = 0.0f;
    _flashFrames    = 0;
    _wheelAngle     = 0.0f;
}

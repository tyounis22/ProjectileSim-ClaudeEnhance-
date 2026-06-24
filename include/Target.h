#pragma once

#include "Entity.h"

class Target : public Entity {
    public:
        void Draw() override;
        void Update(float dt) override;
        // checked only on the single frame the ball crosses the disk's face,
        // so a ball passing through can't register as many hits in a row.
        // returns 0 for no hit, 1 for the outer ring, ... up to 5 for a dead-center bullseye.
        int CheckHit(Vector3 prevBallPos, Vector3 ballPos, float ballRadius) const;
        // true once the ball can no longer score on this target: either it flew past the
        // target's downrange plane this frame, or it fell short (behind the target and no
        // longer moving toward it, which with constant wind means it can never reach it).
        // the caller should only treat this as a miss when CheckHit didn't register a hit.
        bool Missed(Vector3 prevBallPos, Vector3 ballPos, float ballVelocityX) const;
        void ChangeColor();
        Color GetColor() const { return color; }
        void Shrink();
        void Reset();
        void ApplyDifficulty(int tier);  // set patrol speed + arc mode from difficulty tier

        float radius{10.0f};

        // patrol / arc state (public so main can call ApplyDifficulty after randomizing position)
        float patrolSpeed{0.0f};   // m/s (patrol) or rad/s (arc)
        float patrolDir{1.0f};     // +1 or -1 for patrol direction
        float patrolBound{25.0f};  // |Z| limit for patrol bounce
        bool  arcMode{false};      // true → circular Y-Z arc instead of flat patrol
        float arcPhase{0.0f};      // current arc angle, radians
        float baseY{15.0f};        // arc centre Y
        float baseZ{0.0f};         // arc centre Z

    private:
        static constexpr Color kPalette[] = {
            RED, ORANGE, PINK, GOLD,MAGENTA, PURPLE, VIOLET, MAROON, BROWN, BEIGE, DARKBLUE, 
        };
        int ColorIndex{0};   // index into the palette for the current color
        Color color{kPalette[ColorIndex]};     //red targets
};


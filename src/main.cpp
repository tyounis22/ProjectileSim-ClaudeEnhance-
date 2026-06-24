#include "raylib.h"
#include "Projectile.h"
#include <cmath>
#include <algorithm>
#include <deque>
#include "Cannon.h"
#include "Target.h"
#include "Constants.h"
#include "Debris.h"
#include <vector>

using Constants::DEBRIS_COLOR_VARIATION;

enum class GameState  { MENU, PLAYING, GAME_OVER };
enum class TimeOfDay  { DAY, SUNSET, NIGHT };

struct SmokeParticle { Vector3 pos, vel; float radius, alpha, life, maxLife; };
struct StarData       { Vector3 pos; float size; Color color; };

constexpr int TRAIL_MAX = 12;

// ── SOUND SYNTHESIS ──────────────────────────────────────────────────────────
// Hash-based pseudo-noise: deterministic, no rand() needed
static float hashNoise(int i, float seed) {
    float n = sinf((float)i * seed) * 43758.5453f;
    return (n - floorf(n)) * 2.0f - 1.0f;
}

Sound GenBoom() {
    const int sampleRate = 44100;
    const int n          = (int)(sampleRate * 0.35f);
    short* d = new short[n];
    for (int i = 0; i < n; i++) {
        float t     = (float)i / sampleRate;
        float decay = expf(-t * 18.0f);
        float noise = hashNoise(i, 127.1f) * 0.5f + hashNoise(i, 311.7f) * 0.3f;
        float sub   = sinf(2.0f * PI * 55.0f * t) * 0.2f;
        d[i] = (short)((noise + sub) * decay * 28000.0f);
    }
    Wave w; w.frameCount = n; w.sampleRate = sampleRate; w.sampleSize = 16; w.channels = 1; w.data = d;
    Sound s = LoadSoundFromWave(w);
    delete[] d;
    return s;
}

Sound GenHit() {
    const int sampleRate = 44100;
    const int n          = (int)(sampleRate * 0.5f);
    short* d = new short[n];
    for (int i = 0; i < n; i++) {
        float t      = (float)i / sampleRate;
        float decay  = expf(-t * 7.0f);
        float freq   = 380.0f * expf(-t * 3.5f) + 130.0f;
        float tone   = sinf(2.0f * PI * freq * t);
        float h2     = 0.3f * sinf(4.0f * PI * freq * t);
        float attack = hashNoise(i, 173.1f) * expf(-t * 80.0f) * 0.3f;
        d[i] = (short)((tone + h2 + attack) * decay * 26000.0f);
    }
    Wave w; w.frameCount = n; w.sampleRate = sampleRate; w.sampleSize = 16; w.channels = 1; w.data = d;
    Sound s = LoadSoundFromWave(w);
    delete[] d;
    return s;
}

// ── HELPERS ───────────────────────────────────────────────────────────────────
int ClampColorChannel(int v) { return v < 0 ? 0 : v > 255 ? 255 : v; }

int getDifficultyTier(int hitCount) {
    if (hitCount < 6)  return 0;
    if (hitCount < 12) return 1;
    if (hitCount < 18) return 2;
    return 3;
}

void DrawWindHUD(Vector3 wind, int screenWidth, bool warning) {
    Vector2 center = { (float)screenWidth - 80.0f, 90.0f };
    float radius = 38.0f;
    float windSpeed = sqrtf(wind.x * wind.x + wind.z * wind.z);
    float dirX = wind.z, dirY = -wind.x;

    bool flashOn    = warning && ((int)(GetTime() * 5) % 2 == 0);
    Color ringColor = flashOn ? RED : GRAY;
    Color lblColor  = flashOn ? RED : BLACK;

    DrawCircleLines((int)center.x, (int)center.y, radius + 10, ringColor);
    DrawText(flashOn ? "WIND!" : "WIND",
             (int)center.x - (flashOn ? 25 : 20),
             (int)center.y - (int)radius - 34, 16, lblColor);

    if (windSpeed > 0.01f) {
        float len = sqrtf(dirX * dirX + dirY * dirY);
        dirX /= len; dirY /= len;
        float arrowLen = windSpeed < 2.0f ? 14.0f : windSpeed < 5.0f ? 22.0f : windSpeed < 8.0f ? 30.0f : radius;
        Vector2 tip  = { center.x + dirX * arrowLen, center.y + dirY * arrowLen };
        Vector2 tail = { center.x - dirX * arrowLen, center.y - dirY * arrowLen };
        DrawLineEx(tail, tip, 4.0f, RED);
        float px = -dirY, py = dirX;
        Vector2 base  = { center.x + dirX * fmaxf(arrowLen - 12, 0.0f), center.y + dirY * fmaxf(arrowLen - 12, 0.0f) };
        DrawTriangle(tip, {base.x - px*9, base.y - py*9}, {base.x + px*9, base.y + py*9}, RED);
    }

    int fs = 18, whole = (int)windSpeed;
    int tenths = (int)roundf((windSpeed - whole) * 10.0f);
    if (tenths >= 10) { tenths = 0; whole++; }
    int tx = (int)center.x - 30, ty = (int)center.y + (int)radius + 14;
    DrawText(TextFormat("%d", whole), tx, ty, fs, BLACK);
    int ww = MeasureText(TextFormat("%d", whole), fs);
    float dx = tx + ww + 4.5f, dy = ty + fs - 4.5f;
    DrawCircle((int)dx, (int)dy, 2.5f, BLACK);
    DrawText(TextFormat("%d m/s", tenths), (int)(dx + 5), ty, fs, BLACK);
}

// ── WORLD DRAWING ─────────────────────────────────────────────────────────────
void DrawWorld(TimeOfDay tod, const std::vector<StarData>& stars) {
    float t = (float)GetTime();

    // ground color varies by time of day
    Color groundColor = (tod == TimeOfDay::NIGHT)  ? (Color){20,  50,  20,  255}
                      : (tod == TimeOfDay::SUNSET) ? (Color){80,  108, 45,  255}
                                                   : (Color){76,  124, 60,  255};
    DrawPlane({0.0f, 0.0f, 0.0f}, {1000.0f, 1000.0f}, groundColor);
    DrawGrid(200, 5.0f);

    // range markers
    for (int dist = 25; dist <= 200; dist += 25) {
        float x = (float)dist;
        DrawCube({x, 1.0f,  12.0f}, 0.4f, 2.0f, 0.4f, {200,200,200,255});
        DrawCube({x, 1.0f, -12.0f}, 0.4f, 2.0f, 0.4f, {200,200,200,255});
    }

    // trees with rustling leaves
    Color trunk  = {101, 67, 33, 255};
    Color leaves = (tod == TimeOfDay::NIGHT) ? (Color){18, 70, 18, 255} : (Color){34, 139, 34, 255};

    // each tree gets a distinct phase offset (multiples of 1.7 rad)
    DrawCylinder({30.0f,  0.0f,  25.0f}, 0.5f, 0.3f,  8.0f, 8, trunk);
    DrawSphere(  {30.0f,  9.8f,  25.0f}, 3.0f * (1.0f + 0.045f*sinf(t*1.8f+0.0f)), leaves);

    DrawCylinder({70.0f,  0.0f, -30.0f}, 0.5f, 0.3f, 10.0f, 8, trunk);
    DrawSphere(  {70.0f, 12.4f, -30.0f}, 4.0f * (1.0f + 0.045f*sinf(t*1.8f+1.7f)), leaves);

    DrawCylinder({120.0f, 0.0f,  28.0f}, 0.5f, 0.3f,  6.0f, 8, trunk);
    DrawSphere(  {120.0f, 7.5f,  28.0f}, 2.5f * (1.0f + 0.045f*sinf(t*1.8f+3.4f)), leaves);

    DrawCylinder({45.0f,  0.0f, -35.0f}, 0.5f, 0.3f,  9.0f, 8, trunk);
    DrawSphere(  {45.0f, 11.1f, -35.0f}, 3.5f * (1.0f + 0.045f*sinf(t*1.8f+5.1f)), leaves);

    DrawCylinder({90.0f,  0.0f,  32.0f}, 0.6f, 0.3f, 12.0f, 8, trunk);
    DrawSphere(  {90.0f, 14.7f,  32.0f}, 4.5f * (1.0f + 0.045f*sinf(t*1.8f+6.8f)), leaves);

    DrawCylinder({160.0f, 0.0f, -25.0f}, 0.5f, 0.3f,  7.0f, 8, trunk);
    DrawSphere(  {160.0f, 8.8f, -25.0f}, 3.0f * (1.0f + 0.045f*sinf(t*1.8f+8.5f)), leaves);

    // boulders
    Color stone = {140,140,140,255};
    DrawSphere({ 25.0f, 0.9f,  18.0f}, 1.5f, stone);
    DrawSphere({ 80.0f, 1.2f, -20.0f}, 2.0f, stone);
    DrawSphere({ 55.0f, 0.6f,  22.0f}, 1.0f, stone);
    DrawSphere({140.0f, 1.0f, -18.0f}, 1.8f, stone);

    // mountains
    Color mountain = {100,100,120,255};
    DrawCylinder({300.0f, 0.0f,-190.0f}, 0.0f, 60.0f, 80.0f, 6, mountain);
    DrawCylinder({350.0f, 0.0f,-160.0f}, 0.0f, 50.0f, 65.0f, 6, mountain);
    DrawCylinder({260.0f, 0.0f,-210.0f}, 0.0f, 45.0f, 55.0f, 6, mountain);
    DrawCylinder({320.0f, 0.0f, -90.0f}, 0.0f, 55.0f, 70.0f, 6, mountain);

    // sun / moon / stars
    if (tod == TimeOfDay::DAY) {
        DrawSphere({200.0f, 110.0f, -110.0f}, 20.0f, YELLOW);
    } else if (tod == TimeOfDay::SUNSET) {
        DrawSphere({360.0f, 16.0f, -230.0f}, 28.0f, {255, 70, 10, 255});
    } else {
        // moon
        DrawSphere({-120.0f, 160.0f, -80.0f}, 18.0f, {240, 240, 210, 255});
        // stars
        for (const auto& s : stars) DrawSphere(s.pos, s.size, s.color);
    }

    // clouds with gentle sinusoidal drift in Z
    float dz = sinf(t * 0.08f) * 20.0f;
    Color cloud = (tod == TimeOfDay::SUNSET) ? (Color){255,155,75,200}
               : (tod == TimeOfDay::NIGHT)   ? (Color){28, 28, 55,160}
                                             : (Color){255,255,255,220};

    DrawSphere({ 60.0f,  80.0f, -48.0f+dz},  8.0f, cloud);
    DrawSphere({ 65.0f,  80.0f, -43.0f+dz},  7.0f, cloud);
    DrawSphere({ 55.0f,  80.0f, -45.0f+dz},  6.0f, cloud);

    DrawSphere({120.0f,  70.0f,  45.0f+dz}, 10.0f, cloud);
    DrawSphere({125.0f,  70.0f,  28.0f+dz},  8.0f, cloud);
    DrawSphere({120.0f,  70.0f,  34.0f+dz},  7.0f, cloud);

    DrawSphere({175.0f,  85.0f, -20.0f+dz},  9.0f, cloud);
    DrawSphere({185.0f,  80.0f, -16.0f+dz},  7.0f, cloud);
}

void DrawPowerBar(float power, int screenWidth, int screenHeight) {
    int bw = 300, bh = 24;
    int x  = screenWidth / 2 - bw / 2, y = screenHeight - 60;
    DrawRectangle(x, y, bw, bh, {40,40,40,200});
    DrawRectangle(x, y, (int)(bw * power / 100.0f), bh, RED);
    DrawRectangleLines(x, y, bw, bh, BLACK);
    DrawText("POWER", x, y - 22, 18, BLACK);
}

void spawnDebris(std::vector<Debris>& debris, Vector3 origin, Color tColor) {
    const float debrisSpeed = 3.5f;
    for (int i = 0; i < 100; i++) {
        Debris piece;
        piece.position = origin;
        piece.radius   = GetRandomValue(10, 30) / 100.0f;
        piece.color    = { (unsigned char)GetRandomValue(ClampColorChannel(tColor.r - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.r + DEBRIS_COLOR_VARIATION)),
                           (unsigned char)GetRandomValue(ClampColorChannel(tColor.g - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.g + DEBRIS_COLOR_VARIATION)),
                           (unsigned char)GetRandomValue(ClampColorChannel(tColor.b - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.b + DEBRIS_COLOR_VARIATION)),
                           255 };
        piece.velocity = { debrisSpeed * GetRandomValue(-90,-30)/10.0f, GetRandomValue(20,120)/10.0f, GetRandomValue(-50,50)/10.0f };
        debris.push_back(piece);
    }
    if ((int)debris.size() > Constants::MAX_DEBRIS)
        debris.erase(debris.begin(), debris.begin() + (debris.size() - Constants::MAX_DEBRIS));
}

void randomizeTarget(Target& target) {
    target.position = { 50.0f, (float)GetRandomValue((int)target.radius, 22), (float)GetRandomValue(-22, 22) };
}

void centerTargetPosition(Target& target) { target.position = {50.0f, 15.0f, 0.0f}; }

// ── MAIN ──────────────────────────────────────────────────────────────────────
int main() {
    const int screen_width  = 1280;
    const int screen_height = 720;
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(screen_width, screen_height, "Projectile Simulator");
    SetTargetFPS(60);

    InitAudioDevice();
    Music music    = LoadMusicStream("music/8bit_music.mp3");
    music.looping  = true;
    PlayMusicStream(music);
    Sound sndFire  = GenBoom();
    Sound sndHit   = GenHit();

    Texture2D menuTex = LoadTexture("images/start_screen.png");

    // pick time-of-day once per session
    TimeOfDay tod = (TimeOfDay)GetRandomValue(0, 2);

    // star field for night sky (golden-angle distribution on upper hemisphere)
    std::vector<StarData> stars;
    for (int i = 0; i < 90; i++) {
        float az = (float)i * 2.3998f;
        float el = (5.0f + 80.0f * (float)(i*i) / (90.0f*90.0f)) * DEG2RAD;
        float r  = 250.0f;
        StarData s;
        s.pos   = { r*cosf(el)*cosf(az), r*sinf(el), r*cosf(el)*sinf(az) };
        s.size  = 0.4f + fmodf((float)i * 0.7f, 1.3f);
        int ci  = i % 7;
        s.color = (ci < 4) ? (Color){220,220,255,255}
                : (ci < 5) ? (Color){255,230,140,255}
                : (ci < 6) ? (Color){255,140,140,255}
                :             (Color){180,180,255,255};
        stars.push_back(s);
    }

    GameState gameState = GameState::MENU;

    int   turnCount  = 0, hitCount = 0, missesLeft = Constants::MISSES_ALLOWED;
    int   score = 0, comboCount = 0;
    float comboDisplayTimer = 0.0f;
    bool  windChangeWarning = false;

    Camera3D camera = {};
    camera.position = {-10.0f, 2.5f, 0.0f};
    camera.target   = { 0.0f,  3.0f, 0.0f};
    camera.up       = { 0.0f,  1.0f, 0.0f};
    camera.fovy     = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Cannon    cannon;
    Projectile ball;
    ball.position = cannon.getPivot();
    ball.active   = false;

    Target target;
    centerTargetPosition(target);
    ball.GenerateWind();

    std::vector<Debris>        debris;
    std::vector<SmokeParticle> smokeParticles;
    std::deque<Vector3>        ballTrail;

    int  hitValue  = 0;
    bool collision = false;
    bool targetVisible      = true;
    float targetRespawnTimer = 0.0f;

    Vector3 hitPopupPos   = {};
    int     hitPopupValue = 0;

    bool missCounted = false;
    bool muted       = false;
    Rectangle muteButton = { (float)screen_width - 100.0f, (float)screen_height - 50.0f, 90.0f, 35.0f };

    float shakeTimer = 0.0f, shakeMagnitude = 0.0f, hitFlashTimer = 0.0f;

    // sky colours by time-of-day
    auto skyColor = [](TimeOfDay t) -> Color {
        if (t == TimeOfDay::SUNSET) return {220, 110, 55, 255};
        if (t == TimeOfDay::NIGHT)  return {  5,   5, 30, 255};
        return {135, 206, 235, 255};
    };

    while (!WindowShouldClose()) {
        float fTime = GetFrameTime();
        UpdateMusicStream(music);

        // ── MENU ────────────────────────────────────────────────────────────
        if (gameState == GameState::MENU) {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                gameState = GameState::PLAYING;

            BeginDrawing();
                ClearBackground(BLACK);
                if (menuTex.width > 0)
                    DrawTexturePro(menuTex,
                        {0,0,(float)menuTex.width,(float)menuTex.height},
                        {0,0,(float)screen_width, (float)screen_height},
                        {0,0}, 0.0f, WHITE);
                else
                    DrawText("PROJECTILE SIM", screen_width/2-200, screen_height/2-60, 60, WHITE);
                const char* msg = "Press SPACE or ENTER to Play";
                int mw = MeasureText(msg, 28);
                DrawRectangle(screen_width/2-mw/2-12, screen_height-88, mw+24, 44, {0,0,0,180});
                DrawText(msg, screen_width/2-mw/2, screen_height-78, 28, WHITE);
            EndDrawing();
            continue;
        }

        // ── MUTE ────────────────────────────────────────────────────────────
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(GetMousePosition(), muteButton)) {
            muted = !muted;
            SetMusicVolume(music, muted ? 0.0f : 1.0f);
        }

        bool gameOver = missesLeft <= 0;
        cannon.Update(fTime);

        // ── GAMEPLAY ─────────────────────────────────────────────────────────
        if (!gameOver) {
            if (IsKeyDown(KEY_LEFT))  cannon.decrAzimuth(fTime);
            if (IsKeyDown(KEY_RIGHT)) cannon.incrAzimuth(fTime);
            if (IsKeyDown(KEY_UP))    cannon.incrElevation(fTime);
            if (IsKeyDown(KEY_DOWN))  cannon.decrElevation(fTime);
            if (IsKeyDown(KEY_SPACE)) cannon.incrLaunchSpeed(fTime);

            if (IsKeyReleased(KEY_SPACE)) {
                cannon.Fire(ball);
                ball.active = true;
                missCounted = false;
                turnCount++;
                ballTrail.clear();
                shakeTimer = 0.12f; shakeMagnitude = 0.06f;
                if (!muted) PlaySound(sndFire);

                Vector3 muzzle = cannon.GetMuzzlePos();
                Vector3 aimDir = cannon.AimDirection();
                for (int i = 0; i < 10; i++) {
                    SmokeParticle p;
                    p.pos    = muzzle;
                    p.vel    = { aimDir.x*0.4f + GetRandomValue(-20,20)/100.0f,
                                 0.4f          + GetRandomValue(  0,30)/100.0f,
                                 aimDir.z*0.4f + GetRandomValue(-20,20)/100.0f };
                    p.radius  = GetRandomValue(8,20)/100.0f;
                    p.alpha   = 170.0f;
                    p.maxLife = GetRandomValue(20,55)/100.0f;
                    p.life    = p.maxLife;
                    smokeParticles.push_back(p);
                }
            }

            Vector3 prevBallPos = ball.position;
            if (ball.active)   ball.Update(fTime);
            if (targetVisible) target.Update(fTime);

            hitValue  = (targetVisible && ball.active) ? target.CheckHit(prevBallPos, ball.position, ball.radius) : 0;
            collision = hitValue > 0;
            for (Debris& piece : debris) piece.Update(fTime);

            if (collision) {
                hitCount++;
                comboCount++;
                int pts       = hitValue * comboCount;
                score        += pts;
                hitPopupValue = pts;
                comboDisplayTimer = 2.0f;
                hitFlashTimer = 0.10f;
                shakeTimer    = 0.25f; shakeMagnitude = 0.12f;
                if (!muted) PlaySound(sndHit);

                spawnDebris(debris, ball.position, target.GetColor());
                collision = false; ball.active = false; ballTrail.clear();
                targetVisible      = false;
                targetRespawnTimer = Constants::TARGET_RESPAWN_DELAY;
                hitPopupPos        = target.position;
                randomizeTarget(target);
                if (hitCount % 3 == 0) { target.ChangeColor(); target.Shrink(); ball.GenerateWind(); }
                target.ApplyDifficulty(getDifficultyTier(hitCount));
                windChangeWarning = (hitCount % 3 == 2);

            } else if (ball.active && targetVisible && !missCounted &&
                       target.Missed(prevBallPos, ball.position, ball.velocity.x)) {
                missCounted = true;
                missesLeft--;
                comboCount = 0;
            }

            if (!targetVisible) {
                targetRespawnTimer -= fTime;
                if (targetRespawnTimer <= 0.0f) targetVisible = true;
            }
        }

        // ── RESTART ──────────────────────────────────────────────────────────
        if (gameOver && IsKeyPressed(KEY_R)) {
            turnCount = hitCount = 0; missesLeft = Constants::MISSES_ALLOWED;
            score = comboCount = 0; comboDisplayTimer = 0.0f;
            windChangeWarning = false;
            ball.active = false; ball.position = cannon.getPivot();
            debris.clear(); smokeParticles.clear(); ballTrail.clear();
            target.Reset(); centerTargetPosition(target);
            ball.GenerateWind();
            targetVisible = true; targetRespawnTimer = 0.0f; missCounted = false;
            hitFlashTimer = shakeTimer = 0.0f;
            cannon.Reset();
            tod = (TimeOfDay)GetRandomValue(0, 2);  // new time-of-day on restart
        }

        // ── TRAIL + SMOKE UPDATE ──────────────────────────────────────────────
        if (ball.active) {
            ballTrail.push_front(ball.position);
            if ((int)ballTrail.size() > TRAIL_MAX) ballTrail.pop_back();
        } else if (!ballTrail.empty()) {
            ballTrail.clear();
        }

        for (auto& p : smokeParticles) {
            p.pos.x += p.vel.x * fTime; p.pos.y += p.vel.y * fTime; p.pos.z += p.vel.z * fTime;
            p.life -= fTime; p.radius += fTime * 0.25f;
            p.alpha = (p.life / p.maxLife) * 170.0f;
        }
        smokeParticles.erase(std::remove_if(smokeParticles.begin(), smokeParticles.end(),
            [](const SmokeParticle& p){ return p.life <= 0.0f; }), smokeParticles.end());

        if (hitFlashTimer    > 0.0f) hitFlashTimer    -= fTime;
        if (shakeTimer       > 0.0f) { shakeTimer     -= fTime; if (shakeTimer < 0.0f) shakeTimer = 0.0f; }
        if (comboDisplayTimer > 0.0f) comboDisplayTimer -= fTime;

        // ── DRAW ─────────────────────────────────────────────────────────────
        Camera3D renderCam = camera;
        if (shakeTimer > 0.0f) {
            float s = shakeMagnitude;
            renderCam.position.x += GetRandomValue(-50,50)/1000.0f * s * 10.0f;
            renderCam.position.y += GetRandomValue(-50,50)/1000.0f * s * 10.0f;
            renderCam.position.z += GetRandomValue(-50,50)/1000.0f * s * 10.0f;
        }

        BeginDrawing();
            ClearBackground(skyColor(tod));

            BeginMode3D(renderCam);
                DrawWorld(tod, stars);
                cannon.Draw();

                // ball trail
                BeginBlendMode(BLEND_ALPHA);
                for (int i = 0; i < (int)ballTrail.size(); i++) {
                    float frac = 1.0f - (float)(i+1) / (float)(TRAIL_MAX+1);
                    unsigned char a = (unsigned char)(200.0f * frac);
                    DrawSphere(ballTrail[i], ball.radius * 0.85f * frac, {80,80,80,a});
                }
                EndBlendMode();

                if (ball.active) ball.Draw();

                if (targetVisible) {
                    target.Draw();
                    if (hitFlashTimer > 0.0f) {
                        Vector3 ff = {target.position.x-0.06f, target.position.y, target.position.z};
                        Vector3 fb = {target.position.x+0.06f, target.position.y, target.position.z};
                        DrawCylinderEx(ff, fb, target.radius, target.radius, 24, {255,255,255,200});
                    }
                }

                for (Debris& piece : debris) piece.Draw();

                // smoke
                BeginBlendMode(BLEND_ALPHA);
                for (const auto& p : smokeParticles) {
                    unsigned char a = (unsigned char)fmaxf(0.0f, fminf(255.0f, p.alpha));
                    DrawSphere(p.pos, p.radius, {210,210,210,a});
                }
                EndBlendMode();

                //DrawSphere({0,0,0}, 0.3f, RED);
            EndMode3D();

            // ── HUD ──────────────────────────────────────────────────────────

            // Score box
            DrawRectangleRounded({5,5,170,72}, 0.25f, 8, {0,0,0,160});
            DrawText("SCORE", 16, 13, 16, {180,180,180,255});
            DrawText(TextFormat("%d", score), 16, 33, 38, RAYWHITE);

            // Combo text
            if (comboCount >= 2 && comboDisplayTimer > 0.0f) {
                const char* cs = TextFormat("x%d COMBO!", comboCount);
                int cw = MeasureText(cs, 38);
                unsigned char ca = (unsigned char)(255.0f * fminf(1.0f, comboDisplayTimer / 0.5f));
                DrawText(cs, screen_width/2 - cw/2, 10, 38, {255,215,0,ca});
            }

            // Wind HUD
            DrawWindHUD(ball.windAcceleration, screen_width, windChangeWarning);

            // Difficulty tier badge (below wind HUD)
            {
                int tier = getDifficultyTier(hitCount);
                static const Color tc[] = {{120,120,120,255},{60,200,60,255},{240,210,0,255},{255,100,0,255}};
                static const char* tl[] = {"TIER 0","TIER 1","TIER 2","TIER 3"};
                DrawRectangleRounded({(float)screen_width-115, 175.0f, 105, 30}, 0.3f, 6, {0,0,0,140});
                DrawText(tl[tier], screen_width-100, 183, 18, tc[tier]);
            }

            // Time-of-day badge (below tier badge)
            {
                static const char* todLabel[] = {"DAY","SUNSET","NIGHT"};
                static const Color todColor[] = {{255,230,50,255},{255,130,30,255},{100,120,255,255}};
                int ti = (int)tod;
                DrawRectangleRounded({(float)screen_width-115, 210.0f, 105, 26}, 0.3f, 6, {0,0,0,140});
                DrawText(todLabel[ti], screen_width-100, 216, 16, todColor[ti]);
            }

            // Power bar
            DrawPowerBar(cannon.getLaunchSpeed(), screen_width, screen_height);

            // Instruction text on first shot
            if (turnCount == 0)
                DrawText("Arrow keys to aim  |  Hold SPACE to charge  |  Release to fire",
                         10, 80, 17, DARKGRAY);

            // Miss icons
            {
                int used = Constants::MISSES_ALLOWED - missesLeft;
                DrawRectangleRounded({5,(float)screen_height-58,230,48}, 0.3f, 6, {0,0,0,140});
                DrawText("MISSES", 14, screen_height-50, 16, {180,180,180,255});
                for (int i = 0; i < Constants::MISSES_ALLOWED; i++) {
                    Color ic = (i < used) ? (Color){200,30,30,255} : (Color){30,160,30,255};
                    DrawRectangle(85+i*20, screen_height-49, 14, 16, ic);
                    DrawRectangleLines(85+i*20, screen_height-49, 14, 16, BLACK);
                }
            }

            // "+N" hit popup
            if (!targetVisible) {
                Vector3 wp  = {hitPopupPos.x, hitPopupPos.y + target.radius/3, hitPopupPos.z};
                Vector2 sp  = GetWorldToScreen(wp, renderCam);
                const char* pt = TextFormat("+%d", hitPopupValue);
                int pw = MeasureText(pt, 30);
                DrawText(pt, (int)sp.x - pw/2, (int)sp.y, 30, GOLD);
            }

            // Game over overlay
            if (gameOver) {
                DrawRectangle(0, 0, screen_width, screen_height, {0,0,0,160});
                const char* ot = "GAME OVER";
                int ow = MeasureText(ot, 80);
                DrawText(ot, screen_width/2-ow/2, screen_height/2-90, 80, RED);
                const char* ft = TextFormat("Final Score: %d", score);
                int fw = MeasureText(ft, 40);
                DrawText(ft, screen_width/2-fw/2, screen_height/2+10, 40, RAYWHITE);
                const char* rt = "Press R to Play Again";
                int rw = MeasureText(rt, 24);
                DrawText(rt, screen_width/2-rw/2, screen_height/2+68, 24, LIGHTGRAY);
            }

            // Mute button
            bool mh = CheckCollisionPointRec(GetMousePosition(), muteButton);
            DrawRectangleRec(muteButton, mh ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(muteButton, 2.0f, BLACK);
            const char* mt = muted ? "Unmute" : "Mute";
            int mts = 18, mtw = MeasureText(mt, mts);
            DrawText(mt, (int)(muteButton.x+muteButton.width/2-mtw/2),
                        (int)(muteButton.y+muteButton.height/2-mts/2), mts, BLACK);

        EndDrawing();
    }

    UnloadSound(sndFire);
    UnloadSound(sndHit);
    UnloadTexture(menuTex);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

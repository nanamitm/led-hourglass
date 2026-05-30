#pragma once
// Adafruit PixelDust を Qt6 向けに移植 (動的粒子数対応版)
#include <cstdint>
#include <cmath>
#include <QRandomGenerator>

class PixelDust {
public:
    // maxGrains: 最大収容粒子数。実際の粒子数は addGrain/removeGrainAt で動的に変化
    PixelDust(int w, int h, int maxGrains,
              uint8_t scale = 128, uint8_t elasticity = 64);
    ~PixelDust();

    bool begin();
    void resetAll();  // bitmap と粒子を全クリア (begin() 後に再利用)

    // 障害物ピクセル (壁)
    void setPixel(int x, int y);
    void clearPixel(int x, int y);
    bool getPixel(int x, int y) const;

    // 粒子管理 (動的)
    bool addGrain(int x, int y);       // 追加。満杯・占有済みなら false
    bool removeGrainAt(int x, int y);  // (x,y) にある粒子を削除。なければ false
    int  activeCount() const { return m_nActive; }
    int  maxCount()    const { return m_maxGrains; }

    // 粒子座標取得 (i: 0..activeCount-1)
    int getX(int i) const;
    int getY(int i) const;

    // 1フレーム物理ステップ
    void iterate(int16_t ax, int16_t ay, int16_t az = 0);

private:
    struct Grain {
        int32_t x, y;
        int16_t vx, vy;
    };

    int      m_w, m_h, m_maxGrains, m_nActive;
    int      m_w8;
    int32_t  m_xMax, m_yMax;
    uint8_t  m_scale, m_elasticity;
    uint8_t *m_bitmap = nullptr;
    Grain   *m_grains = nullptr;

    static inline int rnd(int n) {
        return QRandomGenerator::global()->bounded(n);
    }
    static inline int16_t bounce(int16_t v, uint8_t e) {
        return int16_t(-v * e / 256);
    }
};

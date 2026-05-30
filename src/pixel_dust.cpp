#include "pixel_dust.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

PixelDust::PixelDust(int w, int h, int maxGrains, uint8_t scale, uint8_t elasticity)
    : m_w(w), m_h(h), m_maxGrains(maxGrains), m_nActive(0)
    , m_w8((w + 7) / 8)
    , m_xMax(int32_t(w) * 256 - 1), m_yMax(int32_t(h) * 256 - 1)
    , m_scale(scale), m_elasticity(elasticity)
{}

PixelDust::~PixelDust() { delete[] m_bitmap; delete[] m_grains; }

bool PixelDust::begin() {
    m_bitmap = new uint8_t[m_h * m_w8]();
    m_grains = new Grain[m_maxGrains]();
    return m_bitmap && m_grains;
}

void PixelDust::resetAll() {
    memset(m_bitmap, 0, m_h * m_w8);
    m_nActive = 0;
}

void PixelDust::setPixel(int x, int y) {
    if (x < 0 || x >= m_w || y < 0 || y >= m_h) return;
    m_bitmap[y * m_w8 + x / 8] |= (0x80 >> (x & 7));
}
void PixelDust::clearPixel(int x, int y) {
    if (x < 0 || x >= m_w || y < 0 || y >= m_h) return;
    m_bitmap[y * m_w8 + x / 8] &= ~(0x80 >> (x & 7));
}
bool PixelDust::getPixel(int x, int y) const {
    if (x < 0 || x >= m_w || y < 0 || y >= m_h) return true;
    return m_bitmap[y * m_w8 + x / 8] & (0x80 >> (x & 7));
}

int PixelDust::getX(int i) const { return m_grains[i].x / 256; }
int PixelDust::getY(int i) const { return m_grains[i].y / 256; }

bool PixelDust::addGrain(int x, int y) {
    if (m_nActive >= m_maxGrains) return false;
    if (getPixel(x, y)) return false;  // 占有済み
    int i = m_nActive++;
    m_grains[i].x  = x * 256 + 128;
    m_grains[i].y  = y * 256 + 128;
    m_grains[i].vx = 0;
    m_grains[i].vy = 0;
    setPixel(x, y);
    return true;
}

bool PixelDust::removeGrainAt(int x, int y) {
    for (int i = 0; i < m_nActive; ++i) {
        if (m_grains[i].x / 256 == x && m_grains[i].y / 256 == y) {
            clearPixel(x, y);
            // 末尾の粒子と入れ替えて詰める
            if (i < m_nActive - 1) m_grains[i] = m_grains[m_nActive - 1];
            --m_nActive;
            return true;
        }
    }
    return false;
}

void PixelDust::iterate(int16_t ax, int16_t ay, int16_t az) {
    ax = int16_t(int32_t(ax) * m_scale / 256);
    ay = int16_t(int32_t(ay) * m_scale / 256);
    az = int16_t(std::abs(int32_t(az) * m_scale / 2048));
    int az2 = az * 2 + 1;

    for (int i = 0; i < m_nActive; i++) {
        m_grains[i].vx += ax + rnd(az2) - az;
        m_grains[i].vy += ay + rnd(az2) - az;

        int32_t v2 = int32_t(m_grains[i].vx) * m_grains[i].vx
                   + int32_t(m_grains[i].vy) * m_grains[i].vy;
        if (v2 > 65536) {
            float sc = 256.0f / std::sqrt(float(v2));
            m_grains[i].vx = int16_t(m_grains[i].vx * sc);
            m_grains[i].vy = int16_t(m_grains[i].vy * sc);
        }

        int32_t nx = std::clamp(m_grains[i].x + m_grains[i].vx, int32_t(0), m_xMax);
        int32_t ny = std::clamp(m_grains[i].y + m_grains[i].vy, int32_t(0), m_yMax);

        if (nx <= 0 || nx >= m_xMax) { m_grains[i].vx = bounce(m_grains[i].vx, m_elasticity); nx = std::clamp(nx, int32_t(0), m_xMax); }
        if (ny <= 0 || ny >= m_yMax) { m_grains[i].vy = bounce(m_grains[i].vy, m_elasticity); ny = std::clamp(ny, int32_t(0), m_yMax); }

        int oldX = m_grains[i].x / 256, oldY = m_grains[i].y / 256;
        int newX = nx / 256,            newY = ny / 256;

        if (oldX == newX && oldY == newY) {
            m_grains[i].x = nx; m_grains[i].y = ny;
            continue;
        }

        if (!getPixel(newX, newY)) {
            clearPixel(oldX, oldY);
            m_grains[i].x = nx; m_grains[i].y = ny;
            setPixel(newX, newY);
        } else {
            bool movedX = (newX != oldX), movedY = (newY != oldY);
            if (movedX && movedY) {
                if (!getPixel(newX, oldY)) {
                    clearPixel(oldX, oldY);
                    m_grains[i].x = int32_t(newX) * 256 + 128;
                    setPixel(newX, oldY);
                    m_grains[i].vy = bounce(m_grains[i].vy, m_elasticity);
                } else if (!getPixel(oldX, newY)) {
                    clearPixel(oldX, oldY);
                    m_grains[i].y = int32_t(newY) * 256 + 128;
                    setPixel(oldX, newY);
                    m_grains[i].vx = bounce(m_grains[i].vx, m_elasticity);
                } else {
                    m_grains[i].vx = bounce(m_grains[i].vx, m_elasticity);
                    m_grains[i].vy = bounce(m_grains[i].vy, m_elasticity);
                }
            } else if (movedX) {
                m_grains[i].vx = bounce(m_grains[i].vx, m_elasticity);
            } else {
                m_grains[i].vy = bounce(m_grains[i].vy, m_elasticity);
            }
        }
    }
}

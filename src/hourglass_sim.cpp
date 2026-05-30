#include "hourglass_sim.h"
#include <QDateTime>
#include <QSettings>
#include <QTimer>
#include <cmath>
#include <algorithm>
#include <cstring>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

static constexpr int G = HourglassSim::GRID - 1;

HourglassSim::HourglassSim(QObject *parent)
    : QObject(parent)
{
    m_top.begin();
    m_bottom.begin();

    QSettings s("LedHourglass", "LedHourglass");
    m_durationSec = s.value("durationSec", 180).toInt();
    m_colorHue     = s.value("colorHue",     30).toInt();
    m_autoLoop     = s.value("autoLoop",     false).toBool();
    m_sensitivity  = qBound(0.5f, s.value("sensitivity", 1.0f).toFloat(), 2.0f);
    m_grainDensity = s.value("grainDensity", 1).toInt();
    m_nGrains      = (m_grainDensity == 1) ? 120 : (m_grainDensity == 2) ? 200 : 350;
    updateTransferRate();

    buildFunnels();
    placeGrains();
    m_flipTime = QDateTime::currentMSecsSinceEpoch();
}

void HourglassSim::buildFunnels() {}

void HourglassSim::placeGrains() {
    int placed = 0;
    for (int y = G; y >= 0 && placed < m_nGrains; --y)
        for (int x = G; x >= 0 && placed < m_nGrains; --x)
            if (m_top.addGrain(x, y)) ++placed;
}

void HourglassSim::placeGrainsBottom() {
    int placed = 0;
    for (int y = G; y >= 0 && placed < m_nGrains; --y)
        for (int x = G; x >= 0 && placed < m_nGrains; --x)
            if (m_bottom.addGrain(x, y)) ++placed;
}

void HourglassSim::updateTransferRate() {
    m_transferInterval = std::max(1, m_durationSec * 60 / m_nGrains);
}

void HourglassSim::setGrainDensity(int d) {
    d = qBound(1, d, 3);
    if (d == m_grainDensity) return;
    m_grainDensity = d;
    m_nGrains = (d == 1) ? 120 : (d == 2) ? 200 : 350;
    updateTransferRate();
    QSettings("LedHourglass", "LedHourglass").setValue("grainDensity", d);
    emit grainDensityChanged();
    reset();  // 粒子数変更は即リセット
}

void HourglassSim::setSensitivity(float s) {
    s = qBound(0.5f, s, 2.0f);
    if (qFuzzyCompare(s, m_sensitivity)) return;
    m_sensitivity = s;
    QSettings("LedHourglass", "LedHourglass").setValue("sensitivity", s);
    emit sensitivityChanged();
}

void HourglassSim::setAutoLoop(bool on) {
    if (on == m_autoLoop) return;
    m_autoLoop = on;
    QSettings("LedHourglass", "LedHourglass").setValue("autoLoop", on);
    emit autoLoopChanged();
}

void HourglassSim::setColorHue(int h) {
    h = qBound(0, h, 359);
    if (h == m_colorHue) return;
    m_colorHue = h;
    QSettings("LedHourglass", "LedHourglass").setValue("colorHue", h);
    emit colorHueChanged();
}

void HourglassSim::setDurationSec(int d) {
    if (d == m_durationSec) return;
    m_durationSec = d;
    updateTransferRate();
    QSettings("LedHourglass", "LedHourglass").setValue("durationSec", d);
    emit durationChanged();
}

int HourglassSim::remainingSec() const {
    if (complete()) return 0;
    int grainsLeft = (m_gravity.y() >= 0)
                     ? m_top.activeCount()
                     : m_bottom.activeCount();
    return std::max(0, int(std::ceil(grainsLeft * float(m_transferInterval) / 60.0f)));
}

void HourglassSim::resetLapCount() {
    m_lapCount = 0;
    emit stateChanged();
}

void HourglassSim::reset() {
    m_top.resetAll();
    m_bottom.resetAll();
    buildFunnels();
    placeGrains();
    m_flipTime    = QDateTime::currentMSecsSinceEpoch();
    m_wasComplete = false;
    m_lapCount    = 0;
    emit stateChanged();
}

void HourglassSim::setGravity(QVector2D g) {
    float prevY = m_gravity.y();
    m_gravity = g;
    if (prevY * g.y() < -20.0f) {
        m_flipTime    = QDateTime::currentMSecsSinceEpoch();
        m_wasComplete = false;
    }
}

void HourglassSim::vibrateDevice() {
#ifdef Q_OS_ANDROID
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!activity.isValid()) return;
    QJniObject vibrator = activity.callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        QJniObject::fromString("vibrator").object<jstring>());
    if (vibrator.isValid())
        vibrator.callMethod<void>("vibrate", "(J)V", jlong(800));
#endif
}

bool HourglassSim::transferGrain(PixelDust &src, int srcCx, int srcCy,
                                  PixelDust &dst, int dstCx, int dstCy) {
    const float maxDist2 = float(GRID / 2) * float(GRID / 2);
    int   bestIdx = -1;
    float bestD2  = maxDist2;
    for (int i = 0; i < src.activeCount(); ++i) {
        float dx = src.getX(i) - srcCx;
        float dy = src.getY(i) - srcCy;
        float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) { bestD2 = d2; bestIdx = i; }
    }
    if (bestIdx < 0) return false;

    int px = src.getX(bestIdx), py = src.getY(bestIdx);
    static const int OFFSETS[][2] = {
        {0,0},{1,0},{0,1},{1,1},{-1,0},{0,-1},{-1,1},{1,-1},{-1,-1},
        {2,0},{0,2},{2,1},{1,2},{2,2},{-1,2},{2,-1},{3,0},{0,3}
    };
    for (auto &off : OFFSETS) {
        int tx = dstCx + off[0], ty = dstCy + off[1];
        if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) continue;
        if (!dst.getPixel(tx, ty)) {
            src.removeGrainAt(px, py);
            dst.addGrain(tx, ty);
            return true;
        }
    }
    return false;
}

void HourglassSim::tick() {
    static const float S = 1.0f / float(M_SQRT2);
    float gx_d = m_gravity.x(), gy_d = m_gravity.y();
    int16_t ax = int16_t(( gx_d + gy_d) * S * 40.0f * m_sensitivity);
    int16_t ay = int16_t((-gx_d + gy_d) * S * 40.0f * m_sensitivity);

    m_top.iterate(ax, ay);
    m_bottom.iterate(ax, ay);

    // 転送しきい値: 感度が高いほど小さな傾きで転送発動
    const float threshold = 1.0f / m_sensitivity;
    if (--m_transferCooldown <= 0) {
        bool transferred = false;
        if (gy_d > threshold)
            transferred = transferGrain(m_top, G, G, m_bottom, 0, 0);
        else if (gy_d < -threshold)
            transferred = transferGrain(m_bottom, 0, 0, m_top, G, G);
        m_transferCooldown = transferred ? m_transferInterval : 1;
    }

    bool nowComplete = complete();
    if (!m_wasComplete && nowComplete) {
        m_wasComplete = true;
        m_lapCount++;
        emit completionTriggered();
        vibrateDevice();

        if (m_autoLoop) {
            // 2秒後に自動リスタート: 重力と反対側のチャンバーを満たす
            QTimer::singleShot(2000, this, [this]() {
                if (!m_autoLoop) return;
                m_top.resetAll();
                m_bottom.resetAll();
                if (m_gravity.y() >= 0)
                    placeGrains();        // 重力が下 → 上チャンバーに配置
                else
                    placeGrainsBottom();  // 重力が上 → 下チャンバーに配置
                m_flipTime    = QDateTime::currentMSecsSinceEpoch();
                m_wasComplete = false;
                emit stateChanged();
            });
        }
    }

    // 粒子ビットマップ更新
    memset(m_topBitmap, 0, sizeof(m_topBitmap));
    for (int i = 0; i < m_top.activeCount(); ++i)
        m_topBitmap[m_top.getY(i)][m_top.getX(i)] = true;
    memset(m_botBitmap, 0, sizeof(m_botBitmap));
    for (int i = 0; i < m_bottom.activeCount(); ++i)
        m_botBitmap[m_bottom.getY(i)][m_bottom.getX(i)] = true;

    ++m_frameCount;
    if (m_frameCount % 6 == 0) emit stateChanged();
}

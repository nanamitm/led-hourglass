#pragma once
#include <QObject>
#include <QVector2D>
#include "pixel_dust.h"

class HourglassSim : public QObject {
    Q_OBJECT
    Q_PROPERTY(int   totalGrains  READ totalGrains   NOTIFY grainDensityChanged)
    Q_PROPERTY(int   grainDensity READ grainDensity  WRITE setGrainDensity NOTIFY grainDensityChanged)
    Q_PROPERTY(int   bottomGrains READ bottomGrains  NOTIFY stateChanged)
    Q_PROPERTY(bool  complete     READ complete      NOTIFY stateChanged)
    Q_PROPERTY(bool  running      READ running       NOTIFY stateChanged)
    Q_PROPERTY(int   durationSec  READ durationSec   WRITE setDurationSec  NOTIFY durationChanged)
    Q_PROPERTY(int   colorHue     READ colorHue      WRITE setColorHue     NOTIFY colorHueChanged)
    Q_PROPERTY(bool  autoLoop     READ autoLoop      WRITE setAutoLoop     NOTIFY autoLoopChanged)
    Q_PROPERTY(float sensitivity  READ sensitivity   WRITE setSensitivity  NOTIFY sensitivityChanged)
    Q_PROPERTY(int   lapCount     READ lapCount      NOTIFY stateChanged)
    Q_PROPERTY(int   remainingSec READ remainingSec  NOTIFY stateChanged)

public:
    static constexpr int GRID       = 20;
    static constexpr int MAX_GRAINS = 350;  // PixelDust 容量

    explicit HourglassSim(QObject *parent = nullptr);

    int  totalGrains()  const { return m_nGrains; }
    int  grainDensity() const { return m_grainDensity; }
    int  bottomGrains() const { return m_bottom.activeCount(); }
    bool complete()     const {
        return m_gravity.y() >= 0
               ? m_top.activeCount() == 0
               : m_bottom.activeCount() == 0;
    }
    bool running()      const { int b = bottomGrains(); return b > 0 && b < m_nGrains; }
    int  durationSec()  const { return m_durationSec; }
    int  colorHue()     const { return m_colorHue; }
    bool  autoLoop()    const { return m_autoLoop; }
    float sensitivity() const { return m_sensitivity; }
    int  lapCount()     const { return m_lapCount; }
    int  remainingSec() const;

    bool isGrainTop(int x, int y)    const { return m_topBitmap[y][x]; }
    bool isGrainBottom(int x, int y) const { return m_botBitmap[y][x]; }

    Q_INVOKABLE void reset();
    Q_INVOKABLE void resetLapCount();

public slots:
    void tick();
    void setGravity(QVector2D g);
    void setDurationSec(int d);
    void setColorHue(int h);
    void setAutoLoop(bool on);
    void setSensitivity(float s);
    void setGrainDensity(int d);

signals:
    void stateChanged();
    void durationChanged();
    void colorHueChanged();
    void autoLoopChanged();
    void sensitivityChanged();
    void grainDensityChanged();
    void completionTriggered();

private:
    void buildFunnels();
    void placeGrains();
    void placeGrainsBottom();
    void updateTransferRate();
    void vibrateDevice();
    bool transferGrain(PixelDust &src, int srcCx, int srcCy,
                       PixelDust &dst, int dstCx, int dstCy);

    PixelDust m_top   {GRID, GRID, MAX_GRAINS, 96, 64};
    PixelDust m_bottom{GRID, GRID, MAX_GRAINS, 96, 64};

    QVector2D m_gravity;
    qint64    m_flipTime         = 0;
    int       m_frameCount       = 0;
    int       m_transferCooldown = 0;
    int       m_transferInterval = 135;
    bool      m_wasComplete      = false;

    bool      m_topBitmap[GRID][GRID] = {};
    bool      m_botBitmap[GRID][GRID] = {};

    int       m_durationSec = 180;
    int       m_colorHue    = 30;
    bool      m_autoLoop    = false;
    int       m_lapCount    = 0;
    float     m_sensitivity  = 1.0f;
    int       m_grainDensity = 1;
    int       m_nGrains      = 120;
};

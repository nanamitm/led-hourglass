#pragma once
#include <QQuickItem>
#include "hourglass_sim.h"

class HourglassItem : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(HourglassSim* sim READ sim WRITE setSim NOTIFY simChanged)

public:
    explicit HourglassItem(QQuickItem *parent = nullptr);

    HourglassSim *sim() const { return m_sim; }
    void setSim(HourglassSim *s);

signals:
    void simChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

private:
    HourglassSim *m_sim = nullptr;
};

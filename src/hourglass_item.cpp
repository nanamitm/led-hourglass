#include "hourglass_item.h"
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>

HourglassItem::HourglassItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void HourglassItem::setSim(HourglassSim *s) {
    if (m_sim == s) return;
    if (m_sim) disconnect(m_sim, &HourglassSim::stateChanged, this, nullptr);
    m_sim = s;
    if (m_sim) connect(m_sim, &HourglassSim::stateChanged, this, &QQuickItem::update);
    emit simChanged();
    update();
}

static void writeQuad(QSGGeometry::ColoredPoint2D *v,
                      float x0, float y0, float x1, float y1,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    v[0].set(x0,y0,r,g,b,a); v[1].set(x1,y0,r,g,b,a); v[2].set(x0,y1,r,g,b,a);
    v[3].set(x1,y0,r,g,b,a); v[4].set(x1,y1,r,g,b,a); v[5].set(x0,y1,r,g,b,a);
}

static void updateChamber(QSGGeometry::ColoredPoint2D *verts,
                          HourglassSim *sim, bool isTop,
                          float cx, float cy, float cell)
{
    const int N = HourglassSim::GRID;
    const float led = cell * 0.85f;

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            int idx = (y * N + x) * 6;
            float lx = (x - (N-1)*0.5f) * cell;
            float ly = (y - (N-1)*0.5f) * cell;
            float rx = lx * float(M_SQRT1_2) - ly * float(M_SQRT1_2);
            float ry = lx * float(M_SQRT1_2) + ly * float(M_SQRT1_2);
            float sx = cx + rx, sy = cy + ry;
            float x0 = sx - led*0.5f, y0 = sy - led*0.5f;
            float x1 = sx + led*0.5f, y1 = sy + led*0.5f;

            bool grain = isTop ? sim->isGrainTop(x, y) : sim->isGrainBottom(x, y);
            if (grain) {
                // colorHue (0-359) → RGB
                float h6 = sim->colorHue() / 60.0f;
                float c = 1.0f, xv = c * (1.0f - std::abs(std::fmod(h6,2.0f)-1.0f));
                float r1=0,g1=0,b1=0;
                if      (h6<1){r1=c;g1=xv;}  else if (h6<2){r1=xv;g1=c;}
                else if (h6<3){g1=c;b1=xv;}  else if (h6<4){g1=xv;b1=c;}
                else if (h6<5){r1=xv;b1=c;}  else           {r1=c;b1=xv;}
                writeQuad(verts + idx, x0, y0, x1, y1,
                          uint8_t(r1*255), uint8_t(g1*255), uint8_t(b1*255), 255);
            } else {
                writeQuad(verts + idx, x0, y0, x1, y1, 25, 25, 30, 180);
            }
        }
    }
}

QSGNode *HourglassItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_sim) { delete oldNode; return nullptr; }

    const int N = HourglassSim::GRID;
    const int totalVerts = N * N * 6 * 2;

    QSGGeometryNode *node = nullptr;
    if (oldNode) {
        node = static_cast<QSGGeometryNode *>(oldNode);
    } else {
        node = new QSGGeometryNode;
        auto *mat = new QSGVertexColorMaterial;
        mat->setFlag(QSGMaterial::Blending);
        node->setMaterial(mat);
        node->setFlag(QSGNode::OwnsMaterial);
        auto *geo = new QSGGeometry(
            QSGGeometry::defaultAttributes_ColoredPoint2D(), totalVerts);
        geo->setDrawingMode(QSGGeometry::DrawTriangles);
        node->setGeometry(geo);
        node->setFlag(QSGNode::OwnsGeometry);
    }

    auto *verts = static_cast<QSGGeometry::ColoredPoint2D *>(
        node->geometry()->vertexData());

    const float W = float(width()), H = float(height());
    const float slotH = H / 2.0f;
    const float maxDiag = std::min(W * 0.95f, slotH * 0.95f);
    const float cell    = maxDiag / (N * float(M_SQRT2));

    updateChamber(verts,            m_sim, true,  W/2.0f, slotH/2.0f,          cell);
    updateChamber(verts + N*N*6,    m_sim, false, W/2.0f, slotH + slotH/2.0f,  cell);

    node->geometry()->markVertexDataDirty();
    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}

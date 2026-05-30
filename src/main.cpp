#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "hourglass_sim.h"
#include "gravity_source.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QTimer>
#endif

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    auto *sim = new HourglassSim(&app);

#ifdef Q_OS_ANDROID
    auto *gravity = new RealGravitySource(&app);
#else
    auto *gravity = new MouseGravitySource(&app);
#endif
    gravity->start();

    QObject::connect(gravity, &GravitySource::gravityChanged,
                     sim,     &HourglassSim::setGravity);

    QTimer timer;
    timer.setInterval(16);  // 60 FPS
    QObject::connect(&timer, &QTimer::timeout, sim, &HourglassSim::tick);
    timer.start();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("HourglassSim", sim);

    using namespace Qt::StringLiterals;
    const QUrl url(u"qrc:/LedHourglass/qml/Main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);

#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
#endif

#ifdef Q_OS_ANDROID
    // Android: スリープ抑止 (FLAG_KEEP_SCREEN_ON) をウィンドウ生成後に設定
    QTimer::singleShot(500, []() {
        QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
        if (!activity.isValid()) return;
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        if (window.isValid())
            window.callMethod<void>("addFlags", "(I)V", jint(0x00000080));
    });
#endif

    return app.exec();
}

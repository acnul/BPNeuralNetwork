#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "mitenetworkmodel.h"
#include "mnistmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 设置应用图标
    app.setWindowIcon(QIcon(":/logo.png"));

    // 注册QML类型
    qmlRegisterType<MiteNetworkModel>("MiteClassification", 1, 0, "MiteNetworkModel");
    qmlRegisterType<MnistModel>("HandwritingRecognition", 1, 0, "MnistModel");

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}

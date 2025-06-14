#ifndef MNISTMODEL_H
#define MNISTMODEL_H

#include <QObject>
#include <QImage>
#include <QVariant>
#include <QVariantList>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <memory>
#include "mnist_classifier.h"

class MnistModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isModelLoaded READ isModelLoaded NOTIFY modelStatusChanged)
    Q_PROPERTY(QString modelStatus READ modelStatus NOTIFY modelStatusChanged)
    Q_PROPERTY(int predictedDigit READ predictedDigit NOTIFY predictionChanged)
    Q_PROPERTY(double confidence READ confidence NOTIFY predictionChanged)
    Q_PROPERTY(QVariantList probabilities READ probabilities NOTIFY predictionChanged)
    Q_PROPERTY(QString processedImageUrl READ processedImageUrl NOTIFY imageProcessed)

public:
    explicit MnistModel(QObject *parent = nullptr);

    // Property getters
    bool isModelLoaded() const { return m_isModelLoaded; }
    QString modelStatus() const { return m_modelStatus; }
    int predictedDigit() const { return m_predictedDigit; }
    double confidence() const { return m_confidence; }
    QVariantList probabilities() const { return m_probabilities; }
    QString processedImageUrl() const { return m_processedImageUrl; }

public slots:
    void loadModel(const QString& modelPath);
    void processCanvasImage(const QVariant& imageVariant);
    void clearPrediction();

signals:
    void modelStatusChanged();
    void predictionChanged();
    void imageProcessed();

private:
    // 原有的预处理方法
    QImage preprocessImage(const QImage& originalImage);

    // 新的高级预处理方法（基于Python实现）
    QImage preprocessImageAdvanced(const QImage& originalImage);

    // 辅助方法
    QRect findContentBoundingBox(const QImage& image);
    QImage applyGaussianBlur(const QImage& image, double sigma);

    std::vector<double> imageToVector(const QImage& image);
    void saveProcessedImage(const QImage& image);

    std::unique_ptr<MNISTClassifier> m_classifier;
    bool m_isModelLoaded;
    QString m_modelStatus;
    int m_predictedDigit;
    double m_confidence;
    QVariantList m_probabilities;
    QString m_processedImageUrl;

    static int s_imageCounter;
};

#endif // MNISTMODEL_H

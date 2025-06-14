#ifndef MITENETWORKMODEL_H
#define MITENETWORKMODEL_H

#include <QObject>
#include <QVariantList>
#include <QPointF>
#include <QTimer>
#include <QThread>
#include "bpnn.h"

Q_DECLARE_METATYPE(QPointF)

class MiteNetworkModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList inputValues READ inputValues NOTIFY valuesChanged)
    Q_PROPERTY(QVariantList hiddenValues READ hiddenValues NOTIFY valuesChanged)
    Q_PROPERTY(QVariantList outputValues READ outputValues NOTIFY valuesChanged)
    Q_PROPERTY(QVariantList weightsIH READ weightsIH NOTIFY weightsChanged)
    Q_PROPERTY(QVariantList weightsHO READ weightsHO NOTIFY weightsChanged)
    Q_PROPERTY(QVariantList trainingDataA READ getTrainingDataA NOTIFY trainingDataChanged)
    Q_PROPERTY(QVariantList trainingDataB READ getTrainingDataB NOTIFY trainingDataChanged)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(bool isTraining READ isTraining NOTIFY trainingStateChanged)
    Q_PROPERTY(QString trainingStatus READ trainingStatus NOTIFY trainingStateChanged)

public:
    explicit MiteNetworkModel(QObject *parent = nullptr);

    QVariantList inputValues() const { return m_inputValues; }
    QVariantList hiddenValues() const { return m_hiddenValues; }
    QVariantList outputValues() const { return m_outputValues; }
    QVariantList weightsIH() const { return m_weightsIH; }
    QVariantList weightsHO() const { return m_weightsHO; }
    bool isInitialized() const { return m_isInitialized; }
    bool isTraining() const { return m_isTraining; }
    QString trainingStatus() const { return m_trainingStatus; }

    // 训练数据
    Q_INVOKABLE QVariantList getTrainingDataA() const { return m_trainingDataA; }
    Q_INVOKABLE QVariantList getTrainingDataB() const { return m_trainingDataB; }

public slots:
    void initializeAndTrain();  // 合并的初始化和训练方法
    void predict(double x, double y);
    void addTrainingSample(double x, double y, bool isClassA);
    void backpropagateSample(double x, double y, bool isClassA, double learningRate);
    QVariantList getDecisionBoundary(double minX, double maxX, double minY, double maxY, int resolution);

signals:
    void valuesChanged();
    void weightsChanged();
    void initializedChanged();
    void trainingDataChanged();
    void trainingStateChanged();
    void nodeActivated(int layer, int nodeIndex);
    void connectionActivated(int fromLayer, int fromIndex, int toLayer, int toIndex);
    void nodeError(int layer, int nodeIndex);
    void connectionError(int fromLayer, int fromIndex, int toLayer, int toIndex);
    void predictionComplete(QString result, double confidence);
    void trainingComplete();

private slots:
    void onTrainingFinished();

private:
    void initializeTrainingData();
    void initializeNetwork();
    void trainNetworkAsync();
    void updateNetworkVisualization();
    void animateForwardPass(const std::vector<double>& input);
    void animateBackwardPass(const std::vector<double>& target);
    double calculateTrueConfidence(double rawOutput, bool predictedClass);  // 新增：计算真正的置信度

    std::unique_ptr<NeuralNetwork> m_network;
    QVariantList m_inputValues;
    QVariantList m_hiddenValues;
    QVariantList m_outputValues;
    QVariantList m_weightsIH;
    QVariantList m_weightsHO;
    QVariantList m_trainingDataA;
    QVariantList m_trainingDataB;
    bool m_isInitialized;
    bool m_isTraining;
    QString m_trainingStatus;

    QTimer* m_animationTimer;
    QTimer* m_trainingTimer;
    int m_animationStep;
    int m_currentEpoch;
    int m_totalEpochs;
    std::vector<std::vector<double>> m_trainingInputs;
    std::vector<std::vector<double>> m_trainingTargets;
};

#endif // MITENETWORKMODEL_H

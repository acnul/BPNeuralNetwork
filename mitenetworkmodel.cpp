#include "mitenetworkmodel.h"
#include <QDebug>
#include <QtMath>
#include <random>

MiteNetworkModel::MiteNetworkModel(QObject *parent)
    : QObject(parent)
    , m_network(nullptr)
    , m_isInitialized(false)
    , m_isTraining(false)
    , m_trainingStatus("未开始")
    , m_animationTimer(new QTimer(this))
    , m_trainingTimer(new QTimer(this))
    , m_animationStep(0)
    , m_currentEpoch(0)
    , m_totalEpochs(6000)
{
    qRegisterMetaType<QPointF>();

    // 立即初始化训练数据
    initializeTrainingData();

    m_animationTimer->setSingleShot(true);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        m_animationStep++;
    });

    // 设置训练定时器
    m_trainingTimer->setSingleShot(false);
    m_trainingTimer->setInterval(1); // 每1ms处理一个epoch
    connect(m_trainingTimer, &QTimer::timeout, this, &MiteNetworkModel::trainNetworkAsync);
}

void MiteNetworkModel::initializeTrainingData()
{
    // 清空并重新初始化A类数据
    m_trainingDataA.clear();
    QList<QPointF> dataA = {
        {1.24, 1.27}, {1.36, 1.74}, {1.38, 1.64}, {1.38, 1.82}, {1.38, 1.90},
        {1.40, 1.70}, {1.48, 1.82}, {1.54, 1.82}, {1.56, 2.08}
    };
    for (const auto& point : dataA) {
        m_trainingDataA.append(QVariant::fromValue(point));
    }

    // 清空并重新初始化B类数据
    m_trainingDataB.clear();
    QList<QPointF> dataB = {
        {1.14, 1.82}, {1.18, 1.96}, {1.20, 1.86},
        {1.26, 2.00}, {1.28, 2.00}, {1.30, 1.96}
    };
    for (const auto& point : dataB) {
        m_trainingDataB.append(QVariant::fromValue(point));
    }

    qDebug() << "Training data initialized - A count:" << m_trainingDataA.size() << "B count:" << m_trainingDataB.size();

    // 发送信号通知数据已更新
    emit trainingDataChanged();
}

void MiteNetworkModel::initializeNetwork()
{
    m_network = std::make_unique<NeuralNetwork>(0.01);

    // 构建网络：2输入 -> 3隐藏 -> 1输出
    m_network->addLayer(3, ActivationType::SIGMOID); // 隐藏层
    m_network->addLayer(1, ActivationType::SIGMOID); // 输出层

    m_isInitialized = true;
    emit initializedChanged();

    updateNetworkVisualization();
}

void MiteNetworkModel::initializeAndTrain()
{
    if (m_isTraining) return;

    // 设置训练状态
    m_isTraining = true;
    m_trainingStatus = "初始化中...";
    emit trainingStateChanged();

    // 初始化网络
    initializeNetwork();

    // 准备训练数据
    m_trainingInputs.clear();
    m_trainingTargets.clear();

    // A类数据 (目标输出: 0)
    for (const auto& variant : m_trainingDataA) {
        QPointF point = variant.value<QPointF>();
        m_trainingInputs.push_back({point.x(), point.y()});
        m_trainingTargets.push_back({0.0});
    }

    // B类数据 (目标输出: 1)
    for (const auto& variant : m_trainingDataB) {
        QPointF point = variant.value<QPointF>();
        m_trainingInputs.push_back({point.x(), point.y()});
        m_trainingTargets.push_back({1.0});
    }

    qDebug() << "Starting training with" << m_trainingInputs.size() << "samples for" << m_totalEpochs << "epochs";

    // 重置训练参数
    m_currentEpoch = 0;
    m_trainingStatus = "训练中 " + QString::number(m_currentEpoch) + "/" + QString::number(m_totalEpochs);
    emit trainingStateChanged();

    // 开始异步训练
    m_trainingTimer->start();
}

void MiteNetworkModel::trainNetworkAsync()
{
    if (!m_network || m_currentEpoch >= m_totalEpochs) {
        onTrainingFinished();
        return;
    }

    // 训练一个batch
    double loss = m_network->trainBatch(m_trainingInputs, m_trainingTargets);
    m_currentEpoch++;

    // 每50个epoch更新状态 (更频繁的更新)
    if (m_currentEpoch % 50 == 0) {
        m_trainingStatus = "训练中 " + QString::number(m_currentEpoch) + "/" + QString::number(m_totalEpochs);
        emit trainingStateChanged();
    }

    // 每200个epoch输出日志
    if (m_currentEpoch % 200 == 0) {
        qDebug() << "Epoch" << m_currentEpoch << "Loss:" << loss;
    }

    // 检查是否完成
    if (m_currentEpoch >= m_totalEpochs) {
        onTrainingFinished();
    }
}

void MiteNetworkModel::onTrainingFinished()
{
    m_trainingTimer->stop();
    m_isTraining = false;
    m_trainingStatus = "训练完毕";
    emit trainingStateChanged();

    qDebug() << "Training completed after" << m_currentEpoch << "epochs";

    updateNetworkVisualization();
    emit trainingComplete();

    // 2秒后重置状态
    QTimer::singleShot(2000, this, [this]() {
        m_trainingStatus = "已完成";
        emit trainingStateChanged();
    });
}

// 计算真正的置信度
double MiteNetworkModel::calculateTrueConfidence(double rawOutput, bool predictedClass)
{
    // 方法1: 基于距离决策边界的远近计算置信度
    // 决策边界是0.5，距离边界越远，置信度越高
    double distanceFromBoundary = qAbs(rawOutput - 0.5);
    double maxDistance = 0.5; // 最大可能距离
    double confidence = distanceFromBoundary / maxDistance;

    // 确保置信度在合理范围内 (50% - 100%)
    confidence = 0.5 + (confidence * 0.5);

    return qMin(qMax(confidence, 0.5), 1.0);
}

void MiteNetworkModel::predict(double x, double y)
{
    if (!m_network || m_isTraining) return;

    std::vector<double> input = {x, y};

    // 设置输入值用于可视化
    m_inputValues.clear();
    m_inputValues.append(x);
    m_inputValues.append(y);

    // 执行前向传播获取实际的隐藏层和输出层数值
    auto hiddenOutput = m_network->getHiddenLayerOutput(input);
    auto finalOutput = m_network->predict(input);

    // 更新隐藏层数值
    m_hiddenValues.clear();
    for (double val : hiddenOutput) {
        m_hiddenValues.append(val);
    }

    // 更新输出层数值
    m_outputValues.clear();
    m_outputValues.append(finalOutput[0]);

    emit valuesChanged();

    // 执行前向传播动画
    animateForwardPass(input);

    // 获取原始输出和预测结果
    double rawOutput = finalOutput[0];
    bool predictedClass = rawOutput > 0.5; // true为B类，false为A类
    QString result = predictedClass ? "B类" : "A类";

    // 计算真正的置信度
    double trueConfidence = calculateTrueConfidence(rawOutput, predictedClass);

    // 输出调试信息
    qDebug() << "Raw output:" << rawOutput << "Predicted class:" << result
             << "True confidence:" << trueConfidence;

    emit predictionComplete(result, trueConfidence);
}

void MiteNetworkModel::addTrainingSample(double x, double y, bool isClassA)
{
    if (m_isTraining) return;

    QPointF newPoint(x, y);
    if (isClassA) {
        m_trainingDataA.append(QVariant::fromValue(newPoint));
        qDebug() << "Added A class sample:" << newPoint << "Total A:" << m_trainingDataA.size();
    } else {
        m_trainingDataB.append(QVariant::fromValue(newPoint));
        qDebug() << "Added B class sample:" << newPoint << "Total B:" << m_trainingDataB.size();
    }
    // 发送信号通知数据更新
    emit trainingDataChanged();
}

void MiteNetworkModel::backpropagateSample(double x, double y, bool isClassA, double /*learningRate*/)
{
    if (!m_network || m_isTraining) return;

    std::vector<double> input = {x, y};
    std::vector<double> target = {isClassA ? 0.0 : 1.0};

    // 执行反向传播动画
    animateBackwardPass(target);

    // 训练这个样本
    m_network->train(input, target);

    updateNetworkVisualization();
}

QVariantList MiteNetworkModel::getDecisionBoundary(double minX, double maxX, double minY, double maxY, int resolution)
{
    QVariantList boundary;
    if (!m_network || m_isTraining) return boundary;

    double stepX = (maxX - minX) / resolution;
    double stepY = (maxY - minY) / resolution;

    for (int i = 0; i <= resolution; ++i) {
        for (int j = 0; j <= resolution; ++j) {
            double x = minX + i * stepX;
            double y = minY + j * stepY;

            auto output = m_network->predict({x, y});
            double prediction = output[0];

            // 只保存接近决策边界的点 (prediction ≈ 0.5)
            if (qAbs(prediction - 0.5) < 0.05) {
                boundary.append(QVariant::fromValue(QPointF(x, y)));
            }
        }
    }

    return boundary;
}

void MiteNetworkModel::updateNetworkVisualization()
{
    if (!m_network) return;

    // 清空当前值
    m_weightsIH.clear();
    m_weightsHO.clear();

    // 如果没有进行预测，清空数值
    if (m_inputValues.isEmpty()) {
        m_hiddenValues.clear();
        m_outputValues.clear();

        for (int i = 0; i < 3; ++i) {
            m_hiddenValues.append(0.0);
        }
        m_outputValues.append(0.0);
    }

    // 生成示例权重值（实际应该从网络获取）
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    for (int i = 0; i < 6; ++i) { // 2x3 input to hidden weights
        m_weightsIH.append(dis(gen));
    }

    for (int i = 0; i < 3; ++i) { // 3x1 hidden to output weights
        m_weightsHO.append(dis(gen));
    }

    emit weightsChanged();
    emit valuesChanged();
}

void MiteNetworkModel::animateForwardPass(const std::vector<double>& /*input*/)
{
    if (m_isTraining) return;

    m_animationStep = 0;

    // 输入层激活
    QTimer::singleShot(100, this, [this]() { emit nodeActivated(0, 0); });
    QTimer::singleShot(200, this, [this]() { emit nodeActivated(0, 1); });

    // 连接激活 - 输入到隐藏
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            int delay = 400 + (i * 3 + j) * 100;
            QTimer::singleShot(delay, this, [this, i, j]() {
                emit connectionActivated(0, i, 1, j);
            });
        }
    }

    // 隐藏层激活
    for (int i = 0; i < 3; ++i) {
        QTimer::singleShot(1000 + i * 100, this, [this, i]() {
            emit nodeActivated(1, i);
        });
    }

    // 连接激活 - 隐藏到输出
    for (int i = 0; i < 3; ++i) {
        QTimer::singleShot(1400 + i * 100, this, [this, i]() {
            emit connectionActivated(1, i, 2, 0);
        });
    }

    // 输出层激活
    QTimer::singleShot(1800, this, [this]() { emit nodeActivated(2, 0); });
}

void MiteNetworkModel::animateBackwardPass(const std::vector<double>& /*target*/)
{
    if (m_isTraining) return;

    // 输出层错误
    QTimer::singleShot(100, this, [this]() { emit nodeError(2, 0); });

    // 连接错误 - 输出到隐藏
    for (int i = 0; i < 3; ++i) {
        QTimer::singleShot(300 + i * 100, this, [this, i]() {
            emit connectionError(2, 0, 1, i);
        });
    }

    // 隐藏层错误
    for (int i = 0; i < 3; ++i) {
        QTimer::singleShot(700 + i * 100, this, [this, i]() {
            emit nodeError(1, i);
        });
    }

    // 连接错误 - 隐藏到输入
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            int delay = 1100 + (i * 2 + j) * 100;
            QTimer::singleShot(delay, this, [this, i, j]() {
                emit connectionError(1, i, 0, j);
            });
        }
    }
}

#ifndef MNIST_CLASSIFIER_H
#define MNIST_CLASSIFIER_H

#include "bpnn.h"
#include "mnist_reader.h"
#include <chrono>

// MNIST分类器
class MNISTClassifier {
private:
    NeuralNetwork network;
    int input_size;
    int output_size;
    
public:
    MNISTClassifier(double learning_rate = 0.001);
    
    // 构建网络结构
    void buildNetwork();
    
    // 训练模型
    void train(const MNISTData& train_data, int epochs = 10, int batch_size = 32);
    
    // 测试模型
    double test(const MNISTData& test_data);
    
    // 预测单个图像
    int predict(const std::vector<double>& image);
    
    // 获取预测概率
    std::vector<double> getPredictionProbabilities(const std::vector<double>& image);
    
    // 保存模型
    bool saveModel(const std::string& filename);
    
    // 加载模型
    bool loadModel(const std::string& filename);
    
    // 打印训练进度
    void printProgress(int epoch, int total_epochs, double loss, double accuracy);
};

#endif // MNIST_CLASSIFIER_H
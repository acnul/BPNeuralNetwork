#include "mnist_classifier.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <iomanip>

MNISTClassifier::MNISTClassifier(double learning_rate) 
    : network(learning_rate, LossType::CROSS_ENTROPY), input_size(784), output_size(10) {
}

void MNISTClassifier::buildNetwork() {
    // 构建网络结构：784 -> 128 -> 64 -> 10
    network.addLayer(128, ActivationType::RELU);    // 隐藏层1
    network.addLayer(64, ActivationType::RELU);     // 隐藏层2  
    network.addLayer(10, ActivationType::SOFTMAX);  // 输出层
    
    // 使用Adam优化器
    network.setOptimizer(OptimizerType::ADAM, 0.001);
    
    std::cout << "Network architecture built:" << std::endl;
    network.printNetworkInfo();
}

void MNISTClassifier::train(const MNISTData& train_data, int epochs, int batch_size) {
    std::cout << "Starting training..." << std::endl;
    std::cout << "Training samples: " << train_data.num_images << std::endl;
    std::cout << "Epochs: " << epochs << std::endl;
    std::cout << "Batch size: " << batch_size << std::endl;
    
    // 转换标签为one-hot编码
    auto one_hot_labels = MNISTReader::labelsToOneHot(train_data.labels);
    
    // 创建训练索引
    std::vector<int> indices(train_data.num_images);
    for (int i = 0; i < train_data.num_images; ++i) {
        indices[i] = i;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 打乱训练数据
        std::shuffle(indices.begin(), indices.end(), gen);
        
        double total_loss = 0.0;
        int num_batches = (train_data.num_images + batch_size - 1) / batch_size;
        
        for (int batch = 0; batch < num_batches; ++batch) {
            int start_idx = batch * batch_size;
            int end_idx = std::min(start_idx + batch_size, train_data.num_images);
            
            std::vector<std::vector<double>> batch_images;
            std::vector<std::vector<double>> batch_labels;
            
            for (int i = start_idx; i < end_idx; ++i) {
                int idx = indices[i];
                batch_images.push_back(train_data.images[idx]);
                batch_labels.push_back(one_hot_labels[idx]);
            }
            
            double batch_loss = network.trainBatch(batch_images, batch_labels);
            total_loss += batch_loss;
        }
        
        double avg_loss = total_loss / num_batches;
        
        // 计算训练准确率（每5个epoch计算一次）
        double accuracy = 0.0;
        if (epoch % 5 == 0 || epoch == epochs - 1) {
            int correct = 0;
            int sample_size = std::min(1000, train_data.num_images); // 采样1000个样本计算准确率
            
            for (int i = 0; i < sample_size; ++i) {
                int prediction = predict(train_data.images[i]);
                if (prediction == train_data.labels[i]) {
                    correct++;
                }
            }
            accuracy = static_cast<double>(correct) / sample_size;
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        printProgress(epoch + 1, epochs, avg_loss, accuracy);
        std::cout << " [" << duration.count() << "ms]" << std::endl;
    }
    
    std::cout << "Training completed!" << std::endl;
}

double MNISTClassifier::test(const MNISTData& test_data) {
    std::cout << "Testing model..." << std::endl;
    
    int correct = 0;
    int total = test_data.num_images;
    
    for (int i = 0; i < total; ++i) {
        int prediction = predict(test_data.images[i]);
        if (prediction == test_data.labels[i]) {
            correct++;
        }
        
        // 显示进度
        if ((i + 1) % 1000 == 0 || i == total - 1) {
            std::cout << "Processed: " << (i + 1) << "/" << total 
                      << " (" << std::fixed << std::setprecision(1) 
                      << (100.0 * (i + 1)) / total << "%)" << std::endl;
        }
    }
    
    double accuracy = static_cast<double>(correct) / total;
    std::cout << "Test Results:" << std::endl;
    std::cout << "Correct predictions: " << correct << "/" << total << std::endl;
    std::cout << "Accuracy: " << std::fixed << std::setprecision(4) 
              << accuracy * 100 << "%" << std::endl;
    
    return accuracy;
}

int MNISTClassifier::predict(const std::vector<double>& image) {
    auto output = network.predict(image);
    
    // 找到最大概率的类别
    int predicted_class = 0;
    double max_prob = output[0];
    
    for (size_t i = 1; i < output.size(); ++i) {
        if (output[i] > max_prob) {
            max_prob = output[i];
            predicted_class = i;
        }
    }
    
    return predicted_class;
}

std::vector<double> MNISTClassifier::getPredictionProbabilities(const std::vector<double>& image) {
    return network.predict(image);
}

bool MNISTClassifier::saveModel(const std::string& filename) {
    return network.saveModel(filename);
}

bool MNISTClassifier::loadModel(const std::string& filename) {
    return network.loadModel(filename);
}

void MNISTClassifier::printProgress(int epoch, int total_epochs, double loss, double accuracy) {
    std::cout << "Epoch " << std::setw(3) << epoch << "/" << total_epochs
              << " - Loss: " << std::fixed << std::setprecision(6) << loss;
    
    if (accuracy > 0) {
        std::cout << " - Accuracy: " << std::fixed << std::setprecision(4) 
                  << accuracy * 100 << "%";
    }
}
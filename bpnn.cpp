#include "bpnn.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <algorithm>
#include <cassert>

// C++11兼容的make_unique实现
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// ========== 激活函数实现 ==========

double ActivationFunction::sigmoid(double x) {
    // 添加数值稳定性检查
    if (x > 500) return 1.0;
    if (x < -500) return 0.0;
    return 1.0 / (1.0 + std::exp(-x));
}

double ActivationFunction::sigmoidDerivative(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}

double ActivationFunction::relu(double x) {
    return std::max(0.0, x);
}

double ActivationFunction::reluDerivative(double x) {
    return x > 0 ? 1.0 : 0.0;
}

std::vector<double> ActivationFunction::softmax(const std::vector<double>& x) {
    if (x.empty()) return {};
    
    std::vector<double> result(x.size());
    double max_val = *std::max_element(x.begin(), x.end());
    double sum = 0.0;
    
    // 防止数值溢出
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = std::exp(std::min(x[i] - max_val, 500.0));
        sum += result[i];
    }
    
    // 防止除零和非有限数
    if (sum <= 0.0 || !std::isfinite(sum)) {
        std::fill(result.begin(), result.end(), 1.0 / x.size());
        return result;
    }
    
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] /= sum;
    }
    
    return result;
}

std::vector<double> ActivationFunction::softmaxDerivative(const std::vector<double>& x, size_t index) {
    std::vector<double> softmax_output = softmax(x);
    std::vector<double> derivative(x.size());
    
    for (size_t i = 0; i < x.size(); ++i) {
        if (i == index) {
            derivative[i] = softmax_output[i] * (1.0 - softmax_output[i]);
        } else {
            derivative[i] = -softmax_output[i] * softmax_output[index];
        }
    }
    
    return derivative;
}

std::function<double(double)> ActivationFunction::getActivation(ActivationType type) {
    switch (type) {
        case ActivationType::SIGMOID:
            return sigmoid;
        case ActivationType::RELU:
            return relu;
        default:
            return sigmoid;
    }
}

std::function<double(double)> ActivationFunction::getDerivative(ActivationType type) {
    switch (type) {
        case ActivationType::SIGMOID:
            return sigmoidDerivative;
        case ActivationType::RELU:
            return reluDerivative;
        default:
            return sigmoidDerivative;
    }
}

std::function<std::vector<double>(const std::vector<double>&)> 
ActivationFunction::getVectorActivation(ActivationType type) {
    switch (type) {
        case ActivationType::SOFTMAX:
            return softmax;
        default:
            return [type](const std::vector<double>& x) {
                auto func = getActivation(type);
                std::vector<double> result(x.size());
                for (size_t i = 0; i < x.size(); ++i) {
                    result[i] = func(x[i]);
                }
                return result;
            };
    }
}

// ========== 层实现 ==========

Layer::Layer(size_t input_size, size_t output_size, ActivationType activation)
    : activation_type(activation), timestep(0) {
    
    weights.resize(output_size, std::vector<double>(input_size));
    biases.resize(output_size);
    neurons.resize(output_size);
    weighted_sums.resize(output_size);
    errors.resize(output_size);
    
    // 初始化Adam优化器参数
    m_weights.resize(output_size, std::vector<double>(input_size, 0.0));
    v_weights.resize(output_size, std::vector<double>(input_size, 0.0));
    m_biases.resize(output_size, 0.0);
    v_biases.resize(output_size, 0.0);
    
    initializeWeights();
}

void Layer::initializeWeights() {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 改进的权重初始化
    double limit;
    if (activation_type == ActivationType::RELU) {
        // He初始化，适用于ReLU
        limit = std::sqrt(2.0 / getInputSize());
        std::normal_distribution<> dis(0.0, limit);
        for (auto& weight_row : weights) {
            for (auto& weight : weight_row) {
                weight = dis(gen);
            }
        }
    } else {
        // Xavier初始化，适用于sigmoid和tanh
        limit = std::sqrt(6.0 / (getInputSize() + getOutputSize()));
        std::uniform_real_distribution<> dis(-limit, limit);
        for (auto& weight_row : weights) {
            for (auto& weight : weight_row) {
                weight = dis(gen);
            }
        }
    }
    
    // 偏置初始化为0
    std::fill(biases.begin(), biases.end(), 0.0);
}

std::vector<double> Layer::forward(const std::vector<double>& input) {
    if (input.size() != getInputSize()) {
        throw std::invalid_argument("Input size mismatch. Expected: " + 
                                  std::to_string(getInputSize()) + 
                                  ", Got: " + std::to_string(input.size()));
    }
    
    // 计算加权和
    for (size_t i = 0; i < weights.size(); ++i) {
        weighted_sums[i] = biases[i];
        for (size_t j = 0; j < input.size(); ++j) {
            weighted_sums[i] += weights[i][j] * input[j];
        }
    }
    
    // 应用激活函数
    if (activation_type == ActivationType::SOFTMAX) {
        neurons = ActivationFunction::softmax(weighted_sums);
    } else {
        auto activation_func = ActivationFunction::getActivation(activation_type);
        for (size_t i = 0; i < weighted_sums.size(); ++i) {
            neurons[i] = activation_func(weighted_sums[i]);
        }
    }
    
    return neurons;
}

std::vector<double> Layer::backward(const std::vector<double>& gradient) {
    if (gradient.size() != getOutputSize()) {
        throw std::invalid_argument("Gradient size mismatch");
    }
    
    std::vector<double> input_gradient(getInputSize(), 0.0);
    
    // 计算误差项
    if (activation_type == ActivationType::SOFTMAX) {
        // 对于softmax+交叉熵，梯度直接是 predicted - target
        errors = gradient;
    } else {
        auto derivative_func = ActivationFunction::getDerivative(activation_type);
        for (size_t i = 0; i < errors.size(); ++i) {
            errors[i] = gradient[i] * derivative_func(weighted_sums[i]);
        }
    }
    
    // 计算输入梯度（传递给前一层）
    for (size_t i = 0; i < getInputSize(); ++i) {
        for (size_t j = 0; j < getOutputSize(); ++j) {
            input_gradient[i] += errors[j] * weights[j][i];
        }
    }
    
    return input_gradient;
}

void Layer::updateWeightsSGD(const std::vector<double>& input, double learning_rate) {
    if (input.size() != getInputSize()) {
        throw std::invalid_argument("Input size mismatch for weight update");
    }
    
    // 更新权重和偏置
    for (size_t i = 0; i < weights.size(); ++i) {
        for (size_t j = 0; j < weights[i].size(); ++j) {
            weights[i][j] -= learning_rate * errors[i] * input[j];
        }
        biases[i] -= learning_rate * errors[i];
    }
}

void Layer::updateWeightsAdam(const std::vector<double>& input, double learning_rate,
                             double beta1, double beta2, double epsilon) {
    if (input.size() != getInputSize()) {
        throw std::invalid_argument("Input size mismatch for weight update");
    }
    
    timestep++;
    
    // 更新权重
    for (size_t i = 0; i < weights.size(); ++i) {
        for (size_t j = 0; j < weights[i].size(); ++j) {
            double gradient = errors[i] * input[j];
            
            // 更新一阶和二阶动量估计
            m_weights[i][j] = beta1 * m_weights[i][j] + (1 - beta1) * gradient;
            v_weights[i][j] = beta2 * v_weights[i][j] + (1 - beta2) * gradient * gradient;
            
            // 偏差修正
            double m_corrected = m_weights[i][j] / (1 - std::pow(beta1, timestep));
            double v_corrected = v_weights[i][j] / (1 - std::pow(beta2, timestep));
            
            // 更新权重
            weights[i][j] -= learning_rate * m_corrected / (std::sqrt(v_corrected) + epsilon);
        }
        
        // 更新偏置
        double bias_gradient = errors[i];
        m_biases[i] = beta1 * m_biases[i] + (1 - beta1) * bias_gradient;
        v_biases[i] = beta2 * v_biases[i] + (1 - beta2) * bias_gradient * bias_gradient;
        
        double m_bias_corrected = m_biases[i] / (1 - std::pow(beta1, timestep));
        double v_bias_corrected = v_biases[i] / (1 - std::pow(beta2, timestep));
        
        biases[i] -= learning_rate * m_bias_corrected / (std::sqrt(v_bias_corrected) + epsilon);
    }
}

// ========== 优化器实现 ==========

void SGDOptimizer::updateLayer(Layer* layer, const std::vector<double>& input, 
                              double learning_rate) {
    layer->updateWeightsSGD(input, learning_rate);
}

void AdamOptimizer::updateLayer(Layer* layer, const std::vector<double>& input, 
                               double learning_rate) {
    layer->updateWeightsAdam(input, learning_rate, beta1, beta2, epsilon);
}

// ========== 神经网络实现 ==========

NeuralNetwork::NeuralNetwork(double lr, LossType loss) 
    : learning_rate(lr), loss_type(loss) {
    optimizer = make_unique<SGDOptimizer>();
}

NeuralNetwork::~NeuralNetwork() = default;

void NeuralNetwork::addLayer(int neurons, ActivationType activation) {
    if (neurons <= 0) {
        throw std::invalid_argument("Number of neurons must be positive");
    }
    
    size_t input_size = layers.empty() ? 0 : layers.back()->getOutputSize();
    
    if (layers.empty()) {
        // 第一层，输入大小将在第一次前向传播时确定
        // 使用占位符大小1，稍后会重新创建
        layers.push_back(make_unique<Layer>(1, neurons, activation));
    } else {
        layers.push_back(make_unique<Layer>(input_size, neurons, activation));
    }
}

void NeuralNetwork::setOptimizer(OptimizerType type, double lr) {
    learning_rate = lr;
    
    switch (type) {
        case OptimizerType::SGD:
            optimizer = make_unique<SGDOptimizer>();
            break;
        case OptimizerType::ADAM:
            optimizer = make_unique<AdamOptimizer>();
            break;
        default:
            optimizer = make_unique<SGDOptimizer>();
            break;
    }
}

std::vector<double> NeuralNetwork::forward(const std::vector<double>& input) {
    if (layers.empty()) {
        return input;
    }
    
    // 检查第一层是否需要重新初始化（仅在输入大小为1时，说明是占位符）
    if (layers[0]->getInputSize() == 1 && input.size() != 1) {
        size_t output_size = layers[0]->getOutputSize();
        ActivationType activation = ActivationType::RELU; // 默认使用ReLU作为隐藏层激活函数
        layers[0] = make_unique<Layer>(input.size(), output_size, activation);
    }
    
    std::vector<double> current_input = input;
    
    // 逐层前向传播
    for (auto& layer : layers) {
        current_input = layer->forward(current_input);
    }
    
    return current_input;
}

void NeuralNetwork::backward(const std::vector<double>& target) {
    if (layers.empty()) return;
    
    // 计算输出层梯度（均方误差）
    const auto& output = layers.back()->getNeurons();
    if (output.size() != target.size()) {
        throw std::invalid_argument("Target size mismatch");
    }
    
    std::vector<double> gradient(output.size());
    for (size_t i = 0; i < output.size(); ++i) {
        gradient[i] = output[i] - target[i];
    }
    
    // 反向传播
    performBackwardPass(gradient);
}

void NeuralNetwork::backwardCrossEntropy(const std::vector<double>& target) {
    if (layers.empty()) return;
    
    // 对于softmax + 交叉熵，梯度简化为 (predicted - target)
    const auto& output = layers.back()->getNeurons();
    if (output.size() != target.size()) {
        throw std::invalid_argument("Target size mismatch");
    }
    
    std::vector<double> gradient(output.size());
    for (size_t i = 0; i < output.size(); ++i) {
        gradient[i] = output[i] - target[i];
    }
    
    // 反向传播
    performBackwardPass(gradient);
}

void NeuralNetwork::performBackwardPass(std::vector<double> gradient) {
    // 存储每层的输入用于权重更新
    std::vector<std::vector<double>> layer_inputs(layers.size());
    
    // 第一层的输入需要在train方法中提供
    // 其他层的输入是前一层的输出
    for (size_t i = 1; i < layers.size(); ++i) {
        layer_inputs[i] = layers[i-1]->getNeurons();
    }
    
    // 从输出层开始反向传播
    for (int i = static_cast<int>(layers.size()) - 1; i >= 0; --i) {
        gradient = layers[i]->backward(gradient);
        
        // 权重更新（除了第一层，第一层在train方法中更新）
        if (i > 0) {
            optimizer->updateLayer(layers[i].get(), layer_inputs[i], learning_rate);
        }
    }
}

double NeuralNetwork::train(const std::vector<double>& input, const std::vector<double>& target) {
    // 前向传播
    auto output = forward(input);
    
    // 根据损失函数类型进行反向传播
    if (loss_type == LossType::CROSS_ENTROPY) {
        backwardCrossEntropy(target);
    } else {
        backward(target);
    }
    
    // 更新第一层权重（使用原始输入）
    if (!layers.empty()) {
        optimizer->updateLayer(layers[0].get(), input, learning_rate);
    }
    
    // 计算并返回损失
    return calculateLoss(output, target);
}

double NeuralNetwork::trainBatch(const std::vector<std::vector<double>>& inputs,
                                const std::vector<std::vector<double>>& targets) {
    if (inputs.size() != targets.size()) {
        throw std::invalid_argument("Input and target batch sizes don't match");
    }
    
    double total_loss = 0.0;
    
    for (size_t i = 0; i < inputs.size(); ++i) {
        total_loss += train(inputs[i], targets[i]);
    }
    
    return total_loss / inputs.size();
}

std::vector<double> NeuralNetwork::predict(const std::vector<double>& input) {
    return forward(input);
}

std::vector<double> NeuralNetwork::getHiddenLayerOutput(const std::vector<double>& input) {
    if (layers.empty()) return {};
    
    // 确保第一层已正确初始化
    if (layers[0]->getInputSize() == 1 && input.size() != 1) {
        size_t output_size = layers[0]->getOutputSize();
        ActivationType activation = ActivationType::RELU;
        layers[0] = make_unique<Layer>(input.size(), output_size, activation);
    }
    
    // 只通过第一层
    return layers[0]->forward(input);
}

double NeuralNetwork::calculateLoss(const std::vector<double>& predicted,
                                   const std::vector<double>& target) {
    if (loss_type == LossType::CROSS_ENTROPY) {
        return calculateCrossEntropyLoss(predicted, target);
    } else {
        // 均方误差
        double loss = 0.0;
        for (size_t i = 0; i < predicted.size(); ++i) {
            double diff = predicted[i] - target[i];
            loss += diff * diff;
        }
        return loss / (2.0 * predicted.size()); // 除以样本数量
    }
}

double NeuralNetwork::calculateCrossEntropyLoss(const std::vector<double>& predicted,
                                               const std::vector<double>& target) {
    double loss = 0.0;
    const double epsilon = 1e-15; // 防止log(0)
    
    for (size_t i = 0; i < predicted.size(); ++i) {
        // 限制预测值在[epsilon, 1-epsilon]范围内
        double p = std::max(epsilon, std::min(1.0 - epsilon, predicted[i]));
        loss -= target[i] * std::log(p);
    }
    
    return loss;
}

// 在Layer类实现中添加：
ActivationType Layer::getActivationType() const {
    return activation_type;
}

// 完整的saveModel实现
bool NeuralNetwork::saveModel(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file for saving: " << filename << std::endl;
        return false;
    }
    
    try {
        std::cout << "Saving model to: " << filename << std::endl;
        
        // 保存网络配置
        file.write(reinterpret_cast<const char*>(&learning_rate), sizeof(learning_rate));
        file.write(reinterpret_cast<const char*>(&loss_type), sizeof(loss_type));
        
        // 保存优化器类型
        OptimizerType opt_type = optimizer->getType();
        file.write(reinterpret_cast<const char*>(&opt_type), sizeof(opt_type));
        
        // 保存层数
        size_t num_layers = layers.size();
        file.write(reinterpret_cast<const char*>(&num_layers), sizeof(num_layers));
        std::cout << "Saving " << num_layers << " layers..." << std::endl;
        
        // 保存每层的详细信息
        for (size_t layer_idx = 0; layer_idx < layers.size(); ++layer_idx) {
            const auto& layer = layers[layer_idx];
            const auto& weights = layer->getWeights();
            const auto& biases = layer->getBiases();
            
            // 保存激活函数类型
            ActivationType activation = layer->getActivationType();
            file.write(reinterpret_cast<const char*>(&activation), sizeof(activation));
            
            // 保存维度
            size_t rows = weights.size();
            size_t cols = weights.empty() ? 0 : weights[0].size();
            file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
            file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
            
            std::cout << "Layer " << layer_idx << ": " << cols << "->" << rows 
                      << " (activation: " << static_cast<int>(activation) << ")" << std::endl;
            
            // 保存权重
            for (const auto& row : weights) {
                file.write(reinterpret_cast<const char*>(row.data()), 
                          row.size() * sizeof(double));
            }
            
            // 保存偏置
            file.write(reinterpret_cast<const char*>(biases.data()), 
                      biases.size() * sizeof(double));
        }
        
        file.close();
        std::cout << "Model saved successfully!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving model: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

// 完整的loadModel实现
bool NeuralNetwork::loadModel(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file for loading: " << filename << std::endl;
        return false;
    }
    
    try {
        std::cout << "Loading model from: " << filename << std::endl;
        
        // 读取网络配置
        file.read(reinterpret_cast<char*>(&learning_rate), sizeof(learning_rate));
        file.read(reinterpret_cast<char*>(&loss_type), sizeof(loss_type));
        
        // 读取优化器类型并设置
        OptimizerType opt_type;
        file.read(reinterpret_cast<char*>(&opt_type), sizeof(opt_type));
        
        // 读取层数
        size_t num_layers;
        file.read(reinterpret_cast<char*>(&num_layers), sizeof(num_layers));
        std::cout << "Loading " << num_layers << " layers..." << std::endl;
        
        layers.clear();
        
        for (size_t layer_idx = 0; layer_idx < num_layers; ++layer_idx) {
            // 读取激活函数类型
            ActivationType activation;
            file.read(reinterpret_cast<char*>(&activation), sizeof(activation));
            
            // 读取维度
            size_t rows, cols;
            file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
            file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
            
            std::cout << "Layer " << layer_idx << ": " << cols << "->" << rows 
                      << " (activation: " << static_cast<int>(activation) << ")" << std::endl;
            
            // 创建层时使用正确的激活函数类型
            auto layer = make_unique<Layer>(cols, rows, activation);
            
            // 读取权重
            std::vector<std::vector<double>> weights(rows, std::vector<double>(cols));
            for (auto& row : weights) {
                file.read(reinterpret_cast<char*>(row.data()), 
                         row.size() * sizeof(double));
            }
            
            // 读取偏置
            std::vector<double> biases(rows);
            file.read(reinterpret_cast<char*>(biases.data()), 
                     biases.size() * sizeof(double));
            
            layer->setWeights(weights);
            layer->setBiases(biases);
            
            layers.push_back(std::move(layer));
        }
        
        // 恢复优化器设置
        setOptimizer(opt_type, learning_rate);
        
        file.close();
        std::cout << "Model loaded successfully!" << std::endl;
        std::cout << "Learning rate: " << learning_rate << std::endl;
        std::cout << "Loss type: " << (loss_type == LossType::CROSS_ENTROPY ? "Cross-Entropy" : "MSE") << std::endl;
        std::cout << "Optimizer: " << (opt_type == OptimizerType::ADAM ? "Adam" : "SGD") << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

void NeuralNetwork::printNetworkInfo() const {
    std::cout << "Neural Network Information:" << std::endl;
    std::cout << "Number of layers: " << layers.size() << std::endl;
    std::cout << "Learning rate: " << learning_rate << std::endl;
    std::cout << "Loss function: " << (loss_type == LossType::CROSS_ENTROPY ? "Cross-Entropy" : "Mean Squared Error") << std::endl;
    
    if (optimizer) {
        std::cout << "Optimizer: " << (optimizer->getType() == OptimizerType::SGD ? "SGD" : "Adam") << std::endl;
    }
    
    for (size_t i = 0; i < layers.size(); ++i) {
        std::cout << "Layer " << i << ": " 
                  << layers[i]->getInputSize() << " -> " 
                  << layers[i]->getOutputSize() << " neurons" << std::endl;
    }
}
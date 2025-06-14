#ifndef BPNN_H
#define BPNN_H

#include <vector>
#include <functional>
#include <memory>
#include <chrono>

// 激活函数类型
enum class ActivationType {
    SIGMOID,
    RELU,
    SOFTMAX
};

// 损失函数类型
enum class LossType {
    MEAN_SQUARED_ERROR,
    CROSS_ENTROPY
};

// 优化器类型
enum class OptimizerType {
    SGD,
    ADAM
};

// ========== 激活函数类 ==========
class ActivationFunction {
public:
    static double sigmoid(double x);
    static double sigmoidDerivative(double x);
    static double relu(double x);
    static double reluDerivative(double x);
    static std::vector<double> softmax(const std::vector<double>& x);
    static std::vector<double> softmaxDerivative(const std::vector<double>& x, size_t index);
    
    static std::function<double(double)> getActivation(ActivationType type);
    static std::function<double(double)> getDerivative(ActivationType type);
    static std::function<std::vector<double>(const std::vector<double>&)> 
           getVectorActivation(ActivationType type);
};

// ========== 层类 ==========
class Layer {
private:
    std::vector<std::vector<double>> weights;
    std::vector<double> biases;
    std::vector<double> neurons;
    std::vector<double> weighted_sums;
    std::vector<double> errors;
    ActivationType activation_type;
    
    // Adam优化器参数
    std::vector<std::vector<double>> m_weights, v_weights;
    std::vector<double> m_biases, v_biases;
    int timestep;

public:
    Layer(size_t input_size, size_t output_size, ActivationType activation = ActivationType::SIGMOID);
    
    void initializeWeights();
    std::vector<double> forward(const std::vector<double>& input);
    std::vector<double> backward(const std::vector<double>& gradient);
    
    void updateWeightsSGD(const std::vector<double>& input, double learning_rate);
    void updateWeightsAdam(const std::vector<double>& input, double learning_rate,
                          double beta1 = 0.9, double beta2 = 0.999, double epsilon = 1e-8);
    
    // Getters
    size_t getInputSize() const { return weights.empty() ? 0 : weights[0].size(); }
    size_t getOutputSize() const { return weights.size(); }
    const std::vector<double>& getNeurons() const { return neurons; }
    const std::vector<double>& getErrors() const { return errors; }
    const std::vector<std::vector<double>>& getWeights() const { return weights; }
    const std::vector<double>& getBiases() const { return biases; }
    
    // Setters
    void setWeights(const std::vector<std::vector<double>>& w) { weights = w; }
    void setBiases(const std::vector<double>& b) { biases = b; }

    ActivationType getActivationType() const;
};

// ========== 优化器基类 ==========
class Optimizer {
public:
    virtual ~Optimizer() = default;
    virtual void updateLayer(Layer* layer, const std::vector<double>& input, double learning_rate) = 0;
    virtual OptimizerType getType() const = 0;
};

class SGDOptimizer : public Optimizer {
public:
    void updateLayer(Layer* layer, const std::vector<double>& input, double learning_rate) override;
    OptimizerType getType() const override { return OptimizerType::SGD; }
};

class AdamOptimizer : public Optimizer {
private:
    double beta1 = 0.9;
    double beta2 = 0.999;
    double epsilon = 1e-8;
    
public:
    AdamOptimizer(double b1 = 0.9, double b2 = 0.999, double eps = 1e-8) 
        : beta1(b1), beta2(b2), epsilon(eps) {}
    
    void updateLayer(Layer* layer, const std::vector<double>& input, double learning_rate) override;
    OptimizerType getType() const override { return OptimizerType::ADAM; }
};

// ========== 神经网络类 ==========
class NeuralNetwork {
private:
    std::vector<std::unique_ptr<Layer>> layers;
    std::unique_ptr<Optimizer> optimizer;
    double learning_rate;
    LossType loss_type;  // 新增：损失函数类型

public:
    NeuralNetwork(double lr = 0.01, LossType loss = LossType::MEAN_SQUARED_ERROR);
    ~NeuralNetwork();
    
    void addLayer(int neurons, ActivationType activation = ActivationType::SIGMOID);
    void setOptimizer(OptimizerType type, double lr = 0.01);
    void setLossType(LossType type) { loss_type = type; }  // 新增：设置损失函数类型
    
    std::vector<double> forward(const std::vector<double>& input);
    void backward(const std::vector<double>& target);
    void backwardCrossEntropy(const std::vector<double>& target);  // 新增：交叉熵反向传播
    
    double train(const std::vector<double>& input, const std::vector<double>& target);
    double trainBatch(const std::vector<std::vector<double>>& inputs,
                     const std::vector<std::vector<double>>& targets);
    
    std::vector<double> predict(const std::vector<double>& input);
    std::vector<double> getHiddenLayerOutput(const std::vector<double>& input);
    
    // 损失函数计算
    double calculateLoss(const std::vector<double>& predicted, const std::vector<double>& target);
    double calculateCrossEntropyLoss(const std::vector<double>& predicted, 
                                   const std::vector<double>& target);  // 新增：交叉熵损失
    
    bool saveModel(const std::string& filename) const;
    bool loadModel(const std::string& filename);
    void printNetworkInfo() const;

    void performBackwardPass(std::vector<double> gradient);
};

#endif // BPNN_H
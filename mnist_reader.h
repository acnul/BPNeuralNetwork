#ifndef MNIST_READER_H
#define MNIST_READER_H

#include <vector>
#include <string>
#include <fstream>

// MNIST数据结构
struct MNISTData {
    std::vector<std::vector<double>> images;
    std::vector<int> labels;
    int image_rows;
    int image_cols;
    int num_images;
    
    // 构造函数
    MNISTData() : image_rows(0), image_cols(0), num_images(0) {}
    
    // 清理数据
    void clear() {
        images.clear();
        labels.clear();
        image_rows = 0;
        image_cols = 0;
        num_images = 0;
    }
};

// MNIST数据读取器
class MNISTReader {
public:
    // 读取MNIST图像文件
    static bool readImages(const std::string& filename, MNISTData& data);
    
    // 读取MNIST标签文件
    static bool readLabels(const std::string& filename, MNISTData& data);
    
    // 加载完整的MNIST数据集
    static bool loadMNIST(const std::string& images_file, 
                         const std::string& labels_file, 
                         MNISTData& data);
    
    // 数据预处理：归一化
    static void normalizeImages(MNISTData& data);
    
    // 将标签转换为one-hot编码
    static std::vector<std::vector<double>> labelsToOneHot(const std::vector<int>& labels, int num_classes = 10);
    
    // 显示图像（ASCII艺术）
    static void displayImage(const std::vector<double>& image, int rows, int cols);
    
    // 检查文件是否为GZIP格式
    static bool isGzipFile(const std::string& filename);
    
    // 验证MNIST文件格式
    static bool validateMNISTFormat(const std::string& filename, bool is_images);

private:
    // 字节序转换函数
    static int reverseInt(int i);
    
    // 检查魔数
    static bool checkMagicNumber(int magic, bool is_images);
    
    // 安全的内存分配检查
    static bool checkMemoryRequirement(int num_images, int rows, int cols);
};

#endif // MNIST_READER_H
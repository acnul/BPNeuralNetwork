#include "mnist_reader.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>

int MNISTReader::reverseInt(int i) {
    unsigned char c1, c2, c3, c4;
    c1 = i & 255;
    c2 = (i >> 8) & 255;
    c3 = (i >> 16) & 255;
    c4 = (i >> 24) & 255;
    return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
}

bool MNISTReader::checkMagicNumber(int magic, bool is_images) {
    if (is_images) {
        return magic == 2051; // MNIST图像文件的魔数
    } else {
        return magic == 2049; // MNIST标签文件的魔数
    }
}

bool MNISTReader::checkMemoryRequirement(int num_images, int rows, int cols) {
    // 检查是否为合理的MNIST数据尺寸
    if (num_images < 0 || num_images > 100000) {
        std::cerr << "Error: Invalid number of images: " << num_images << std::endl;
        return false;
    }
    
    if (rows != 28 || cols != 28) {
        std::cerr << "Error: Invalid image dimensions: " << rows << "x" << cols << std::endl;
        std::cerr << "Expected: 28x28" << std::endl;
        return false;
    }
    
    // 估算内存需求（每个像素8字节，加上一些额外开销）
    long long memory_needed = static_cast<long long>(num_images) * rows * cols * sizeof(double);
    long long memory_mb = memory_needed / (1024 * 1024);
    
    std::cout << "Estimated memory requirement: " << memory_mb << " MB" << std::endl;
    
    if (memory_mb > 5000) { // 如果需要超过5GB内存，警告用户
        std::cerr << "Warning: This will require more than 5GB of memory!" << std::endl;
        return false;
    }
    
    return true;
}

bool MNISTReader::isGzipFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    unsigned char header[2];
    file.read(reinterpret_cast<char*>(header), 2);
    file.close();
    
    // GZIP文件的前两个字节是 0x1f 0x8b
    return (header[0] == 0x1f && header[1] == 0x8b);
}

bool MNISTReader::validateMNISTFormat(const std::string& filename, bool is_images) {
    if (isGzipFile(filename)) {
        std::cerr << "Error: File " << filename << " appears to be a GZIP compressed file." << std::endl;
        std::cerr << "Please extract the file first using: gunzip " << filename << std::endl;
        return false;
    }
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    int magic_number;
    file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
    magic_number = reverseInt(magic_number);
    
    if (!checkMagicNumber(magic_number, is_images)) {
        std::cerr << "Error: Invalid magic number in " << filename << std::endl;
        std::cerr << "Expected: " << (is_images ? "2051" : "2049") 
                  << ", Got: " << magic_number << std::endl;
        std::cerr << "This file may be corrupted or in wrong format." << std::endl;
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

bool MNISTReader::readImages(const std::string& filename, MNISTData& data) {
    std::cout << "Validating image file format..." << std::endl;
    if (!validateMNISTFormat(filename, true)) {
        return false;
    }
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    
    int magic_number = 0;
    int number_of_images = 0;
    int n_rows = 0;
    int n_cols = 0;
    
    file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
    magic_number = reverseInt(magic_number);
    
    file.read(reinterpret_cast<char*>(&number_of_images), sizeof(number_of_images));
    number_of_images = reverseInt(number_of_images);
    
    file.read(reinterpret_cast<char*>(&n_rows), sizeof(n_rows));
    n_rows = reverseInt(n_rows);
    
    file.read(reinterpret_cast<char*>(&n_cols), sizeof(n_cols));
    n_cols = reverseInt(n_cols);
    
    std::cout << "Image file info:" << std::endl;
    std::cout << "  Magic number: " << magic_number << std::endl;
    std::cout << "  Number of images: " << number_of_images << std::endl;
    std::cout << "  Rows: " << n_rows << std::endl;
    std::cout << "  Cols: " << n_cols << std::endl;
    
    // 验证数据合理性
    if (!checkMemoryRequirement(number_of_images, n_rows, n_cols)) {
        file.close();
        return false;
    }
    
    data.num_images = number_of_images;
    data.image_rows = n_rows;
    data.image_cols = n_cols;
    
    try {
        data.images.clear();
        data.images.reserve(number_of_images); // 预分配内存
        
        std::cout << "Loading images..." << std::endl;
        
        for (int i = 0; i < number_of_images; ++i) {
            std::vector<double> image;
            image.reserve(n_rows * n_cols);
            
            for (int r = 0; r < n_rows; ++r) {
                for (int c = 0; c < n_cols; ++c) {
                    unsigned char temp = 0;
                    file.read(reinterpret_cast<char*>(&temp), sizeof(temp));
                    if (file.eof()) {
                        std::cerr << "Error: Unexpected end of file at image " << i << std::endl;
                        file.close();
                        return false;
                    }
                    image.push_back(static_cast<double>(temp));
                }
            }
            data.images.push_back(std::move(image));
            
            // 显示进度
            if ((i + 1) % 10000 == 0 || i == number_of_images - 1) {
                std::cout << "Loaded " << (i + 1) << "/" << number_of_images 
                          << " images (" << ((i + 1) * 100 / number_of_images) << "%)" << std::endl;
            }
        }
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate memory for images: " << e.what() << std::endl;
        file.close();
        return false;
    }
    
    file.close();
    std::cout << "Successfully loaded " << number_of_images << " images." << std::endl;
    return true;
}

bool MNISTReader::readLabels(const std::string& filename, MNISTData& data) {
    std::cout << "Validating label file format..." << std::endl;
    if (!validateMNISTFormat(filename, false)) {
        return false;
    }
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    
    int magic_number = 0;
    int number_of_labels = 0;
    
    file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
    magic_number = reverseInt(magic_number);
    
    file.read(reinterpret_cast<char*>(&number_of_labels), sizeof(number_of_labels));
    number_of_labels = reverseInt(number_of_labels);
    
    std::cout << "Label file info:" << std::endl;
    std::cout << "  Magic number: " << magic_number << std::endl;
    std::cout << "  Number of labels: " << number_of_labels << std::endl;
    
    if (number_of_labels < 0 || number_of_labels > 100000) {
        std::cerr << "Error: Invalid number of labels: " << number_of_labels << std::endl;
        file.close();
        return false;
    }
    
    try {
        data.labels.clear();
        data.labels.reserve(number_of_labels);
        
        std::cout << "Loading labels..." << std::endl;
        
        for (int i = 0; i < number_of_labels; ++i) {
            unsigned char temp = 0;
            file.read(reinterpret_cast<char*>(&temp), sizeof(temp));
            if (file.eof()) {
                std::cerr << "Error: Unexpected end of file at label " << i << std::endl;
                file.close();
                return false;
            }
            
            if (temp > 9) {
                std::cerr << "Error: Invalid label value: " << static_cast<int>(temp) << std::endl;
                file.close();
                return false;
            }
            
            data.labels.push_back(static_cast<int>(temp));
            
            // 显示进度
            if ((i + 1) % 10000 == 0 || i == number_of_labels - 1) {
                std::cout << "Loaded " << (i + 1) << "/" << number_of_labels 
                          << " labels (" << ((i + 1) * 100 / number_of_labels) << "%)" << std::endl;
            }
        }
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate memory for labels: " << e.what() << std::endl;
        file.close();
        return false;
    }
    
    file.close();
    std::cout << "Successfully loaded " << number_of_labels << " labels." << std::endl;
    return true;
}

bool MNISTReader::loadMNIST(const std::string& images_file, 
                           const std::string& labels_file, 
                           MNISTData& data) {
    // 清理之前的数据
    data.clear();
    
    std::cout << "Loading MNIST dataset..." << std::endl;
    std::cout << "Images file: " << images_file << std::endl;
    std::cout << "Labels file: " << labels_file << std::endl;
    
    if (!readImages(images_file, data)) {
        std::cerr << "Failed to read images!" << std::endl;
        return false;
    }
    
    if (!readLabels(labels_file, data)) {
        std::cerr << "Failed to read labels!" << std::endl;
        return false;
    }
    
    if (data.images.size() != data.labels.size()) {
        std::cerr << "Error: Number of images (" << data.images.size() 
                  << ") and labels (" << data.labels.size() << ") don't match!" << std::endl;
        return false;
    }
    
    std::cout << "Successfully loaded MNIST dataset!" << std::endl;
    std::cout << "Total samples: " << data.images.size() << std::endl;
    return true;
}

void MNISTReader::normalizeImages(MNISTData& data) {
    std::cout << "Normalizing images..." << std::endl;
    for (auto& image : data.images) {
        for (auto& pixel : image) {
            pixel /= 255.0; // 归一化到[0,1]范围
        }
    }
    std::cout << "Images normalized." << std::endl;
}

std::vector<std::vector<double>> MNISTReader::labelsToOneHot(const std::vector<int>& labels, int num_classes) {
    std::cout << "Converting labels to one-hot encoding..." << std::endl;
    std::vector<std::vector<double>> one_hot(labels.size(), std::vector<double>(num_classes, 0.0));
    
    for (size_t i = 0; i < labels.size(); ++i) {
        if (labels[i] >= 0 && labels[i] < num_classes) {
            one_hot[i][labels[i]] = 1.0;
        }
    }
    
    std::cout << "One-hot encoding completed." << std::endl;
    return one_hot;
}

void MNISTReader::displayImage(const std::vector<double>& image, int rows, int cols) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double pixel = image[r * cols + c];
            if (pixel > 0.5) {
                std::cout << "##";
            } else if (pixel > 0.25) {
                std::cout << "..";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << std::endl;
    }
}
#include "mnistmodel.h"
#include <QDebug>
#include <QPainter>
#include <QBuffer>
#include <QDataStream>
#include <QtMath>

int MnistModel::s_imageCounter = 0;

MnistModel::MnistModel(QObject *parent)
    : QObject(parent)
    , m_classifier(nullptr)
    , m_isModelLoaded(false)
    , m_modelStatus("未加载")
    , m_predictedDigit(-1)
    , m_confidence(0.0)
{
    m_classifier = std::make_unique<MNISTClassifier>(0.001);
}

void MnistModel::loadModel(const QString& modelPath)
{
    m_modelStatus = "加载中...";
    emit modelStatusChanged();

    qDebug() << "Attempting to load model from:" << modelPath;

    if (QFile::exists(modelPath)) {
        if (m_classifier->loadModel(modelPath.toStdString())) {
            m_isModelLoaded = true;
            m_modelStatus = "模型已加载";
            qDebug() << "MNIST model loaded successfully";
        } else {
            m_isModelLoaded = false;
            m_modelStatus = "加载失败";
            qDebug() << "Failed to load MNIST model";
        }
    } else {
        m_isModelLoaded = false;
        m_modelStatus = "模型文件不存在";
        qDebug() << "Model file does not exist:" << modelPath;
    }

    emit modelStatusChanged();
}

void MnistModel::processCanvasImage(const QVariant& imageVariant)
{
    if (!m_isModelLoaded) {
        qDebug() << "Model not loaded, cannot predict";
        return;
    }

    qDebug() << "Processing canvas image...";

    // 从QVariant转换为QImage
    QImage image = imageVariant.value<QImage>();
    if (image.isNull()) {
        qDebug() << "Invalid image received";
        return;
    }

    qDebug() << "Original image size:" << image.size() << "Format:" << image.format();

    // 使用改进的预处理算法
    QImage processedImage = preprocessImageAdvanced(image);
    saveProcessedImage(processedImage);

    // 转换为向量
    std::vector<double> imageVector = imageToVector(processedImage);

    // 调试：检查图像是否有内容
    double pixelSum = 0.0;
    for (double val : imageVector) {
        pixelSum += val;
    }
    qDebug() << "Total pixel sum:" << pixelSum << "Average:" << (pixelSum / imageVector.size());

    // 检查是否有足够的内容
    if (pixelSum < 0.01) {
        qDebug() << "Warning: Very little content detected in image";
    }

    // 进行预测
    m_predictedDigit = m_classifier->predict(imageVector);
    auto probVector = m_classifier->getPredictionProbabilities(imageVector);

    // 计算置信度
    m_confidence = probVector[m_predictedDigit];

    // 转换概率向量为QVariantList
    m_probabilities.clear();
    for (int i = 0; i < 10; ++i) {
        m_probabilities.append(probVector[i]);
    }

    qDebug() << "Predicted digit:" << m_predictedDigit << "Confidence:" << m_confidence;

    emit predictionChanged();
}

void MnistModel::clearPrediction()
{
    m_predictedDigit = -1;
    m_confidence = 0.0;
    m_probabilities.clear();
    m_processedImageUrl.clear();
    emit predictionChanged();
    emit imageProcessed();
}

QImage MnistModel::preprocessImageAdvanced(const QImage& originalImage)
{
    qDebug() << "Advanced preprocessing, original size:" << originalImage.size();

    if (originalImage.isNull()) {
        qDebug() << "Original image is null!";
        return QImage();
    }

    // 1. 转换为32位ARGB格式
    QImage convertedImage = originalImage.convertToFormat(QImage::Format_ARGB32);

    // 2. 转换为灰度并提取内容
    QImage grayImage(convertedImage.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < convertedImage.height(); ++y) {
        for (int x = 0; x < convertedImage.width(); ++x) {
            QRgb pixel = convertedImage.pixel(x, y);
            int alpha = qAlpha(pixel);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);

            // 检测白色笔迹（与Python代码类似的逻辑）
            int grayValue = 0;
            if (alpha > 50 && (red > 50 || green > 50 || blue > 50)) {
                // 计算灰度值，白色笔迹转为高亮度
                grayValue = qMax(qMax(red, green), blue);
            }

            grayImage.setPixel(x, y, qRgb(grayValue, grayValue, grayValue));
        }
    }

    // 3. 找到边界框（模拟Python中的边界框检测）
    QRect boundingBox = findContentBoundingBox(grayImage);

    if (boundingBox.isEmpty()) {
        qDebug() << "No content found, returning empty 28x28 image";
        QImage emptyImage(28, 28, QImage::Format_Grayscale8);
        emptyImage.fill(Qt::black);
        return emptyImage;
    }

    qDebug() << "Content bounding box:" << boundingBox;

    // 4. 添加边距（模拟Python中的margin = 20）
    int margin = 20;
    QRect expandedBox(
        qMax(0, boundingBox.x() - margin),
        qMax(0, boundingBox.y() - margin),
        qMin(grayImage.width() - boundingBox.x() + margin, boundingBox.width() + 2 * margin),
        qMin(grayImage.height() - boundingBox.y() + margin, boundingBox.height() + 2 * margin)
        );

    // 5. 裁剪数字区域
    QImage croppedImage = grayImage.copy(expandedBox);

    // 6. 创建正方形图像（模拟Python的square_img逻辑）
    int maxDim = qMax(croppedImage.width(), croppedImage.height());
    maxDim = qMax(maxDim, 20); // 确保至少20像素

    QImage squareImage(maxDim, maxDim, QImage::Format_Grayscale8);
    squareImage.fill(Qt::black);

    // 将裁剪的图像居中放置
    int xOffset = (maxDim - croppedImage.width()) / 2;
    int yOffset = (maxDim - croppedImage.height()) / 2;

    QPainter painter(&squareImage);
    painter.drawImage(xOffset, yOffset, croppedImage);
    painter.end();

    // 7. 缩放到20x20（模拟Python的resize to 20x20）
    QImage resized20 = squareImage.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // 8. 在28x28图像中央放置20x20图像（模拟Python的final_img[4:24, 4:24]）
    QImage finalImage(28, 28, QImage::Format_Grayscale8);
    finalImage.fill(Qt::black);

    QPainter finalPainter(&finalImage);
    finalPainter.drawImage(4, 4, resized20);
    finalPainter.end();

    // 9. 应用高斯模糊（模拟Python的gaussian_filter）
    QImage blurredImage = applyGaussianBlur(finalImage, 0.5);

    qDebug() << "Advanced preprocessing complete, final size:" << blurredImage.size();
    return blurredImage;
}

QRect MnistModel::findContentBoundingBox(const QImage& image)
{
    // 找到非零像素的边界框（模拟Python的np.where(img_array > 50)）
    int minX = image.width();
    int maxX = -1;
    int minY = image.height();
    int maxY = -1;

    bool foundContent = false;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            int grayValue = qGray(image.pixel(x, y));
            if (grayValue > 50) { // 阈值与Python代码一致
                foundContent = true;
                minX = qMin(minX, x);
                maxX = qMax(maxX, x);
                minY = qMin(minY, y);
                maxY = qMax(maxY, y);
            }
        }
    }

    if (!foundContent) {
        return QRect();
    }

    return QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

QImage MnistModel::applyGaussianBlur(const QImage& image, double sigma)
{
    // 简化的高斯模糊实现（模拟Python的ndimage.gaussian_filter）
    if (sigma <= 0) {
        return image;
    }

    QImage result(image.size(), image.format());

    // 高斯核大小
    int kernelSize = static_cast<int>(ceil(3.0 * sigma)) * 2 + 1;
    int halfKernel = kernelSize / 2;

    // 创建高斯核
    std::vector<std::vector<double>> kernel(kernelSize, std::vector<double>(kernelSize));
    double sum = 0.0;

    for (int i = 0; i < kernelSize; ++i) {
        for (int j = 0; j < kernelSize; ++j) {
            int x = i - halfKernel;
            int y = j - halfKernel;
            double value = exp(-(x*x + y*y) / (2.0 * sigma * sigma));
            kernel[i][j] = value;
            sum += value;
        }
    }

    // 归一化核
    for (int i = 0; i < kernelSize; ++i) {
        for (int j = 0; j < kernelSize; ++j) {
            kernel[i][j] /= sum;
        }
    }

    // 应用卷积
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            double newValue = 0.0;

            for (int ky = 0; ky < kernelSize; ++ky) {
                for (int kx = 0; kx < kernelSize; ++kx) {
                    int imageX = x + kx - halfKernel;
                    int imageY = y + ky - halfKernel;

                    // 边界处理
                    if (imageX >= 0 && imageX < image.width() &&
                        imageY >= 0 && imageY < image.height()) {
                        int pixelValue = qGray(image.pixel(imageX, imageY));
                        newValue += pixelValue * kernel[ky][kx];
                    }
                }
            }

            int finalValue = qBound(0, static_cast<int>(newValue), 255);
            result.setPixel(x, y, qRgb(finalValue, finalValue, finalValue));
        }
    }

    return result;
}

QImage MnistModel::preprocessImage(const QImage& originalImage)
{
    // 保留原有方法作为备用
    return preprocessImageAdvanced(originalImage);
}

std::vector<double> MnistModel::imageToVector(const QImage& image)
{
    std::vector<double> vector;
    vector.reserve(28 * 28);

    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            QRgb pixel = image.pixel(x, y);
            double value = qGray(pixel) / 255.0; // 归一化到0-1
            vector.push_back(value);
        }
    }

    return vector;
}

void MnistModel::saveProcessedImage(const QImage& image)
{
    // 放大图像以便更好地显示
    QImage displayImage = image.scaled(112, 112, Qt::KeepAspectRatio, Qt::FastTransformation);

    // 保存处理后的图像到临时文件
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = QString("processed_image_%1.png").arg(++s_imageCounter);
    QString filePath = QDir(tempDir).absoluteFilePath(fileName);

    if (displayImage.save(filePath)) {
        m_processedImageUrl = QUrl::fromLocalFile(filePath).toString();
        qDebug() << "Processed image saved to:" << filePath;
        emit imageProcessed();
    } else {
        qDebug() << "Failed to save processed image";
    }
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import HandwritingRecognition 1.0

Rectangle {
    id: root
    color: "#f5f5f5"

    signal backToMain()

    // 创建MNIST模型实例
    MnistModel {
        id: mnistModel

        Component.onCompleted: {
            loadModel("mnist_model.bin")
        }
    }

    // 顶部控制栏
    Rectangle {
        id: topBar
        width: parent.width
        height: 60
        color: "#2c3e50"

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            Rectangle {
                width: 120
                height: 40
                color: backMouseArea.containsMouse ? "#34495e" : "#2c3e50"
                border.color: "white"
                border.width: 1
                radius: 4

                Text {
                    anchors.centerIn: parent
                    text: "返回主菜单"
                    color: "white"
                }

                MouseArea {
                    id: backMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.backToMain()
                }
            }

            Text {
                text: "手写数字识别演示"
                font.pixelSize: 24
                font.bold: true
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // 主要内容区域
    Row {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        spacing: 20

        // 左侧：手写画板区域
        Rectangle {
            width: parent.width * 0.5
            height: parent.height
            color: "white"
            radius: 8
            border.color: "#ddd"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15

                Text {
                    text: "手写画板"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#2c3e50"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // 画板区域
                Rectangle {
                    width: 280
                    height: 280
                    color: "#f0f0f0"
                    border.color: "#ccc"
                    border.width: 2
                    radius: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    DrawingCanvas {
                        id: drawingCanvas
                        anchors.centerIn: parent

                        onDrawingChanged: {
                            console.log("Drawing changed event received, hasDrawing:", drawingCanvas.hasDrawing)
                            // 清除之前的预测结果
                            mnistModel.clearPrediction()
                        }

                        // 连接图像捕获信号
                        onImageGrabbed: function(image) {
                            console.log("Image grabbed, processing...");
                            mnistModel.processCanvasImage(image);
                        }
                    }
                }

                // 控制按钮
                Row {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        width: 120
                        height: 45
                        // 修改启用条件：只要模型加载了就可以点击
                        property bool buttonEnabled: mnistModel.isModelLoaded && drawingCanvas.canvasReady
                        color: !buttonEnabled ? "#cccccc" : (recognizeMouseArea.containsMouse ? "#4CAF50" : "#45a049")
                        radius: 6

                        Text {
                            anchors.centerIn: parent
                            text: "识别数字"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 14
                        }

                        // 调试文本
                        Text {
                            anchors.top: parent.bottom
                            anchors.topMargin: 5
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Model: " + mnistModel.isModelLoaded + " Canvas: " + drawingCanvas.canvasReady + " Drawing: " + drawingCanvas.hasDrawing
                            font.pixelSize: 8
                            color: "#666"
                            visible: false // 设为true以显示调试信息
                        }

                        MouseArea {
                            id: recognizeMouseArea
                            anchors.fill: parent
                            enabled: parent.buttonEnabled
                            hoverEnabled: true
                            onClicked: {
                                console.log("Recognize button clicked - Model loaded:", mnistModel.isModelLoaded, "Canvas ready:", drawingCanvas.canvasReady, "Has drawing:", drawingCanvas.hasDrawing);
                                if (drawingCanvas.hasDrawing) {
                                    drawingCanvas.captureImage();
                                } else {
                                    console.log("No drawing to recognize");
                                    // 即使没有绘制内容也尝试捕获，用于测试
                                    drawingCanvas.captureImage();
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 120
                        height: 45
                        color: clearMouseArea.containsMouse ? "#f44336" : "#e53935"
                        radius: 6

                        Text {
                            anchors.centerIn: parent
                            text: "清除画板"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 14
                        }

                        MouseArea {
                            id: clearMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                console.log("Clear button clicked")
                                drawingCanvas.clearCanvas()
                                mnistModel.clearPrediction()
                            }
                        }
                    }
                }

                // 使用说明
                Rectangle {
                    width: parent.width - 20
                    height: 100 // 增加高度以容纳调试信息
                    color: "#e3f2fd"
                    radius: 6
                    border.color: "#2196f3"
                    border.width: 1
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        anchors.centerIn: parent
                        spacing: 5

                        Text {
                            text: "使用说明"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#1976d2"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "• 在画板上手写0-9的数字"
                            font.pixelSize: 12
                            color: "#1976d2"
                        }

                        Text {
                            text: "• 点击\"识别数字\"进行预测"
                            font.pixelSize: 12
                            color: "#1976d2"
                        }

                        Text {
                            text: "• 点击\"清除画板\"重新开始"
                            font.pixelSize: 12
                            color: "#1976d2"
                        }
                    }
                }
            }
        }

        // 右侧：结果显示区域
        Column {
            width: parent.width * 0.5 - 20
            height: parent.height
            spacing: 15

            // 模型状态显示
            Rectangle {
                width: parent.width
                height: 80
                color: mnistModel.isModelLoaded ? "#d4edda" : "#f8d7da"
                radius: 8
                border.color: mnistModel.isModelLoaded ? "#c3e6cb" : "#f1aeb5"
                border.width: 1

                Column {
                    anchors.centerIn: parent
                    Text {
                        text: "模型状态"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: mnistModel.modelStatus
                        font.pixelSize: 16
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // 预处理图像显示
            Rectangle {
                width: parent.width
                height: 200
                color: "white"
                radius: 8
                border.color: "#ddd"
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    Text {
                        text: "预处理图像 (28×28)"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Rectangle {
                        width: 140
                        height: 140
                        color: "#f0f0f0"
                        border.color: "#ccc"
                        border.width: 1
                        radius: 4
                        anchors.horizontalCenter: parent.horizontalCenter

                        Image {
                            id: processedImage
                            anchors.centerIn: parent
                            width: 112
                            height: 112
                            fillMode: Image.PreserveAspectFit
                            source: mnistModel.processedImageUrl
                            cache: false

                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width
                                height: parent.height
                                color: "transparent"
                                border.color: "#999"
                                border.width: 1
                                visible: processedImage.source == ""

                                Text {
                                    anchors.centerIn: parent
                                    text: "等待图像"
                                    color: "#999"
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
            }

            // 识别结果显示
            Rectangle {
                width: parent.width
                height: 120
                color: "#e8f5e8"
                radius: 8
                border.color: "#4CAF50"
                border.width: 1
                visible: mnistModel.predictedDigit >= 0

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "识别结果"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 10

                        Text {
                            text: "数字："
                            font.pixelSize: 18
                            color: "#2c3e50"
                        }

                        Text {
                            text: mnistModel.predictedDigit >= 0 ? mnistModel.predictedDigit.toString() : "无"
                            font.pixelSize: 32
                            font.bold: true
                            color: "#4CAF50"
                        }
                    }

                    Text {
                        text: "置信度：" + (mnistModel.confidence * 100).toFixed(1) + "%"
                        font.pixelSize: 16
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // 概率分布显示
            Rectangle {
                width: parent.width
                height: parent.height - 425
                color: "white"
                radius: 8
                border.color: "#ddd"
                border.width: 1
                visible: mnistModel.probabilities.length > 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    Text {
                        text: "所有数字概率"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    ListView {
                        width: parent.width
                        height: parent.height - 40
                        model: 10
                        delegate: Rectangle {
                            width: parent ? parent.width : 0
                            height: 30
                            color: index === mnistModel.predictedDigit ? "#e8f5e8" : "transparent"
                            radius: 4

                            Row {
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 10

                                Text {
                                    text: "数字 " + index + ":"
                                    font.pixelSize: 14
                                    color: "#2c3e50"
                                    width: 60
                                }

                                Rectangle {
                                    width: 150
                                    height: 15
                                    color: "#f0f0f0"
                                    radius: 7
                                    border.color: "#ddd"
                                    border.width: 1

                                    Rectangle {
                                        width: parent.width * (mnistModel.probabilities.length > index ? mnistModel.probabilities[index] : 0)
                                        height: parent.height
                                        color: index === mnistModel.predictedDigit ? "#4CAF50" : "#2196F3"
                                        radius: parent.radius
                                    }
                                }

                                Text {
                                    text: mnistModel.probabilities.length > index ?
                                          (mnistModel.probabilities[index] * 100).toFixed(1) + "%" : "0.0%"
                                    font.pixelSize: 12
                                    color: "#666"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

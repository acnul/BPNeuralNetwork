import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: "BP神经网络演示系统"

    // 完全隐藏系统标题栏
    flags: Qt.Window | Qt.FramelessWindowHint

    // 禁止调整窗口大小
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height

    property bool inMiteClassification: false
    property bool inHandwritingRecognition: false

    // 自定义标题栏
    Rectangle {
        id: customTitleBar
        width: parent.width
        height: 40
        color: "#2c3e50"
        z: 1000

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Text {
                text: "BP神经网络演示系统"
                color: "white"
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            // 最小化按钮
            Rectangle {
                width: 30
                height: 30
                color: minimizeMouseArea.containsMouse ? "#34495e" : "transparent"
                radius: 3

                Text {
                    anchors.centerIn: parent
                    text: "−"
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                }

                MouseArea {
                    id: minimizeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: mainWindow.showMinimized()
                }
            }

            // 关闭按钮
            Rectangle {
                width: 30
                height: 30
                color: closeMouseArea.containsMouse ? "#e74c3c" : "transparent"
                radius: 3

                Text {
                    anchors.centerIn: parent
                    text: "×"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: Qt.quit()
                }
            }
        }

        // 拖拽区域
        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: 70 // 为按钮留空间
            property point lastPosition

            onPressed: function(mouse) {
                lastPosition = Qt.point(mouse.x, mouse.y)
            }

            onMouseXChanged: {
                if (pressed) {
                    mainWindow.x += mouseX - lastPosition.x
                }
            }

            onMouseYChanged: {
                if (pressed) {
                    mainWindow.y += mouseY - lastPosition.y
                }
            }
        }
    }

    // 主要内容区域
    Rectangle {
        anchors.top: customTitleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#667eea" }
            GradientStop { position: 1.0; color: "#764ba2" }
        }

        visible: !mainWindow.inMiteClassification && !mainWindow.inHandwritingRecognition

        Column {
            anchors.centerIn: parent
            spacing: 25

            // Logo图片 - 添加抗锯齿和质量优化
            Item {
                width: 190
                height: 190
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: logoImage
                    anchors.centerIn: parent
                    width: 190
                    height: 190
                    source: "qrc:/logo_w.png"
                    fillMode: Image.PreserveAspectFit

                    // 启用抗锯齿
                    antialiasing: true
                    smooth: true

                    // 使用mipmap以获得更好的缩放质量
                    mipmap: true

                    // 缓存优化
                    cache: true

                    // 异步加载
                    asynchronous: true

                    // 图片加载状态处理
                    onStatusChanged: {
                        if (status === Image.Error) {
                            console.warn("Logo image failed to load")
                        } else if (status === Image.Ready) {
                            console.log("Logo image loaded successfully")
                        }
                    }
                }

                // 如果图片加载失败，显示备用内容
                Rectangle {
                    anchors.centerIn: parent
                    width: 120
                    height: 120
                    radius: 60
                    color: "white"
                    opacity: 0.2
                    visible: logoImage.status === Image.Error

                    Text {
                        anchors.centerIn: parent
                        text: "LOGO"
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }
                }
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 8

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "BP神经网络演示系统"
                    font.pixelSize: 36
                    font.bold: true
                    color: "white"

                    // 启用文字抗锯齿
                    antialiasing: true
                    renderType: Text.QtRendering

                    // 添加文字阴影
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.implicitWidth
                        height: parent.implicitHeight
                        color: "transparent"
                        z: -1

                        Text {
                            anchors.centerIn: parent
                            text: parent.text
                            font: parent.font
                            color: "black"
                            opacity: 0.3
                            x: 2
                            y: 2
                            antialiasing: true
                            renderType: Text.QtRendering
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "合肥工业大学 2024218482 李昊"
                    font.pixelSize: 14
                    color: "white"
                    opacity: 0.85
                    antialiasing: true
                    renderType: Text.QtRendering
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "选择要演示的功能"
                font.pixelSize: 18
                color: "white"
                opacity: 0.8
                antialiasing: true
                renderType: Text.QtRendering
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                Rectangle {
                    width: 300
                    height: 60
                    color: miteMouseArea.containsMouse ? "#4CAF50" : "#45a049"
                    border.width: 2
                    border.color: "white"
                    radius: 8
                    antialiasing: true

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: miteMouseArea.containsMouse ? "#5CBF60" : "#50B858"
                        }
                        GradientStop {
                            position: 1.0
                            color: miteMouseArea.containsMouse ? "#4CAF50" : "#45a049"
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "螨虫分类"
                        font.pixelSize: 18
                        color: "white"
                        font.bold: true
                        antialiasing: true
                        renderType: Text.QtRendering
                    }

                    MouseArea {
                        id: miteMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            mainWindow.inMiteClassification = true
                        }
                    }
                }

                Rectangle {
                    width: 300
                    height: 60
                    color: handwritingMouseArea.containsMouse ? "#FF9800" : "#FF8C00"
                    border.width: 2
                    border.color: "white"
                    radius: 8
                    antialiasing: true

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: handwritingMouseArea.containsMouse ? "#FFB74D" : "#FFA726"
                        }
                        GradientStop {
                            position: 1.0
                            color: handwritingMouseArea.containsMouse ? "#FF9800" : "#FF8C00"
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "手写数字识别"
                        font.pixelSize: 18
                        color: "white"
                        font.bold: true
                        antialiasing: true
                        renderType: Text.QtRendering
                    }

                    MouseArea {
                        id: handwritingMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            mainWindow.inHandwritingRecognition = true
                        }
                    }
                }
            }

            // 版权信息
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "© 2025 合肥工业大学 数据结构 课程设计 学生作品"
                font.pixelSize: 12
                color: "white"
                opacity: 0.6
                antialiasing: true
                renderType: Text.QtRendering
            }
        }
    }

    // 螨虫分类窗口
    MiteClassificationWindow {
        id: miteWindow
        visible: mainWindow.inMiteClassification
        anchors.top: customTitleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        onBackToMain: {
            mainWindow.inMiteClassification = false
        }
    }

    // 手写数字识别窗口
    HandwritingRecognitionWindow {
        id: handwritingWindow
        visible: mainWindow.inHandwritingRecognition
        anchors.top: customTitleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        onBackToMain: {
            mainWindow.inHandwritingRecognition = false
        }
    }
}

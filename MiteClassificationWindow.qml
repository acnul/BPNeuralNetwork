import QtQuick 2.15
import QtQuick.Controls 2.15
import MiteClassification 1.0

Rectangle {
    id: root
    color: "#f5f5f5"

    signal backToMain()

    property bool showDecisionBoundary: false
    property point testPoint: Qt.point(-1, -1)
    property string currentPrediction: ""

    MiteNetworkModel {
        id: networkModel

        onPredictionComplete: function(result, confidence) {
            root.currentPrediction = result + " (置信度: " + (confidence * 100).toFixed(1) + "%)"
        }

        onTrainingComplete: {
            if (root.showDecisionBoundary) {
                coordinateAxis.updateDecisionBoundary()
            }
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
                    enabled: !networkModel.isTraining
                    onClicked: root.backToMain()
                }
            }

            Text {
                text: "螨虫分类演示"
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

        // 左侧：坐标轴和数据点
        Rectangle {
            width: parent.width * 0.6
            height: parent.height
            color: "white"
            radius: 8
            border.color: "#ddd"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: "数据分布图"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#2c3e50"
                }

                CoordinateAxis {
                    id: coordinateAxis
                    width: parent.width - 20
                    height: parent.height - 40

                    trainingDataA: networkModel.trainingDataA
                    trainingDataB: networkModel.trainingDataB
                    testPoint: root.testPoint
                    showDecisionBoundary: root.showDecisionBoundary

                    onLeftClicked: function(dataX, dataY) {
                        if (!networkModel.isTraining) {
                            root.testPoint = Qt.point(dataX, dataY)
                        }
                    }

                    onRightClicked: function(dataX, dataY) {
                        if (!networkModel.isTraining) {
                            sampleDialog.dataX = dataX
                            sampleDialog.dataY = dataY
                            sampleDialog.open()
                        }
                    }

                    function updateDecisionBoundary() {
                        if (root.showDecisionBoundary && networkModel.isInitialized && !networkModel.isTraining) {
                            decisionBoundaryPoints = networkModel.getDecisionBoundary(1.0, 1.7, 1.2, 2.2, 50)
                        }
                    }
                }
            }
        }

        // 右侧：控制面板和神经网络
        Column {
            width: parent.width * 0.4 - 20
            height: parent.height
            spacing: 15

            // 控制面板
            Rectangle {
                width: parent.width
                height: 280  // 减少高度
                color: "white"
                radius: 8
                border.color: "#ddd"
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10  // 减少间距

                    Text {
                        text: "控制面板"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#2c3e50"
                    }

                    Rectangle {
                        width: parent.width
                        height: 45  // 稍微减少高度
                        color: (networkModel.isInitialized && !networkModel.isTraining) ? "#cccccc" : (trainMouseArea.containsMouse ? "#45a049" : "#4CAF50")
                        radius: 4

                        Column {
                            anchors.centerIn: parent
                            Text {
                                text: "Step1: 初始化并训练"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 13  // 稍微减小字体
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "(6000 epochs)"
                                color: "white"
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        MouseArea {
                            id: trainMouseArea
                            anchors.fill: parent
                            enabled: !networkModel.isInitialized && !networkModel.isTraining
                            hoverEnabled: true
                            onClicked: {
                                networkModel.initializeAndTrain()
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 35  // 减少高度
                        color: (!networkModel.isInitialized || root.testPoint.x < 0 || networkModel.isTraining) ? "#cccccc" : (predictMouseArea.containsMouse ? "#FF8C00" : "#FF9800")
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: "Step2: 预测"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 13
                        }

                        MouseArea {
                            id: predictMouseArea
                            anchors.fill: parent
                            enabled: networkModel.isInitialized && root.testPoint.x >= 0 && !networkModel.isTraining
                            hoverEnabled: true
                            onClicked: {
                                networkModel.predict(root.testPoint.x, root.testPoint.y)
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 8

                        Rectangle {
                            width: parent.width * 0.7 - 4
                            height: 35
                            color: (!networkModel.isInitialized || root.testPoint.x < 0 || networkModel.isTraining) ? "#cccccc" : (backpropMouseArea.containsMouse ? "#7B1FA2" : "#9C27B0")
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: "Step3: 反向传播"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 13
                            }

                            MouseArea {
                                id: backpropMouseArea
                                anchors.fill: parent
                                enabled: networkModel.isInitialized && root.testPoint.x >= 0 && !networkModel.isTraining
                                hoverEnabled: true
                                onClicked: {
                                    networkModel.backpropagateSample(root.testPoint.x, root.testPoint.y, false, 0.1)
                                }
                            }
                        }

                        Rectangle {
                            width: parent.width * 0.3 - 4
                            height: 35
                            color: "#f0f0f0"
                            border.color: "#ccc"
                            border.width: 1
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: "0.10"
                                color: "#333"
                                font.pixelSize: 12
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 35
                        color: (!networkModel.isInitialized || networkModel.isTraining) ? "#cccccc" : (boundaryMouseArea.containsMouse ? "#6A1B9A" : "#9C27B0")
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: "Step4: " + (root.showDecisionBoundary ? "隐藏决策边界" : "显示决策边界")
                            color: "white"
                            font.bold: true
                            font.pixelSize: 13
                        }

                        MouseArea {
                            id: boundaryMouseArea
                            anchors.fill: parent
                            enabled: networkModel.isInitialized && !networkModel.isTraining
                            hoverEnabled: true
                            onClicked: {
                                root.showDecisionBoundary = !root.showDecisionBoundary
                                coordinateAxis.updateDecisionBoundary()
                            }
                        }
                    }
                }
            }

            // 合并的状态和结果显示面板
            Rectangle {
                width: parent.width
                height: 100  // 增加高度以容纳两种信息
                color: {
                    if (networkModel.isTraining) return "#fff3cd"
                    if (networkModel.trainingStatus === "训练完毕") return "#d4edda"
                    if (root.currentPrediction !== "") return "#e8f5e8"
                    return "#e2e3e5"
                }
                radius: 8
                border.color: {
                    if (networkModel.isTraining) return "#ffeaa7"
                    if (networkModel.trainingStatus === "训练完毕") return "#c3e6cb"
                    if (root.currentPrediction !== "") return "#4CAF50"
                    return "#ced4da"
                }
                border.width: 1
                visible: networkModel.trainingStatus !== "未开始" || root.currentPrediction !== ""

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    // 训练状态部分
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 12
                        visible: networkModel.trainingStatus !== "未开始"

                        // 旋转的加载指示器
                        Item {
                            width: 18
                            height: 18
                            visible: networkModel.isTraining
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                id: loadingIndicator
                                width: 18
                                height: 18
                                radius: 9
                                color: "#007bff"

                                Rectangle {
                                    width: 3
                                    height: 3
                                    radius: 1.5
                                    color: "white"
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: parent.top
                                    anchors.topMargin: 2
                                }
                            }

                            RotationAnimation {
                                target: loadingIndicator
                                property: "rotation"
                                from: 0
                                to: 360
                                duration: 1000
                                loops: Animation.Infinite
                                running: networkModel.isTraining
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                text: "训练状态"
                                font.pixelSize: 12
                                font.bold: true
                                color: "#2c3e50"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: networkModel.trainingStatus
                                font.pixelSize: 14
                                color: "#2c3e50"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    // 分隔线
                    Rectangle {
                        width: parent.parent.width * 0.8
                        height: 1
                        color: "#ddd"
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: networkModel.trainingStatus !== "未开始" && root.currentPrediction !== ""
                    }

                    // 预测结果部分
                    Column {
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: root.currentPrediction !== ""
                        Text {
                            text: "预测结果"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#2c3e50"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: root.currentPrediction
                            font.pixelSize: 14
                            color: "#2c3e50"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }

            // 神经网络可视化 - 增加高度
            Rectangle {
                width: parent.width
                height: parent.height - 395  // 大幅增加高度
                color: "#f8f9fa"
                radius: 8
                border.color: "#ddd"
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 10

                    Text {
                        text: "神经网络结构"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#2c3e50"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Item {
                        id: networkContainer
                        width: parent.width
                        height: parent.height - 30

                        readonly property int nodeSize: 50
                        readonly property int nodeRadius: nodeSize / 2

                        readonly property int inputLayerX: 50
                        readonly property int inputLayerY: 60
                        readonly property int inputSpacing: 90

                        readonly property int hiddenLayerX: 170
                        readonly property int hiddenLayerY: 40
                        readonly property int hiddenSpacing: 70

                        readonly property int outputLayerX: 290
                        readonly property int outputLayerY: 130

                        // 层标题 - 调整位置与节点对齐
                        Text {
                            x: networkContainer.inputLayerX + networkContainer.nodeSize/2 - width/2
                            y: 20
                            text: "输入层"
                            font.bold: true
                            font.pixelSize: 12
                            color: "#2d3748"
                        }
                        Text {
                            x: networkContainer.hiddenLayerX + networkContainer.nodeSize/2 - width/2
                            y: 20
                            text: "隐藏层"
                            font.bold: true
                            font.pixelSize: 12
                            color: "#2d3748"
                        }
                        Text {
                            x: networkContainer.outputLayerX + networkContainer.nodeSize/2 - width/2
                            y: 20
                            text: "输出层"
                            font.bold: true
                            font.pixelSize: 12
                            color: "#2d3748"
                        }

                        // 输入层节点
                        Repeater {
                            model: 2
                            NetworkNode {
                                id: inputNode
                                x: networkContainer.inputLayerX
                                y: networkContainer.inputLayerY + index * networkContainer.inputSpacing
                                width: networkContainer.nodeSize
                                height: networkContainer.nodeSize
                                nodeValue: networkModel.inputValues[index] || 0
                                nodeId: "input_" + index

                                Connections {
                                    target: networkModel
                                    function onNodeActivated(layer, nodeIndex) {
                                        if (layer === 0 && nodeIndex === index) {
                                            inputNode.activateNode()
                                        }
                                    }
                                }
                            }
                        }

                        // 隐藏层节点
                        Repeater {
                            model: 3
                            NetworkNode {
                                id: hiddenNode
                                x: networkContainer.hiddenLayerX
                                y: networkContainer.hiddenLayerY + index * networkContainer.hiddenSpacing
                                width: networkContainer.nodeSize
                                height: networkContainer.nodeSize
                                nodeValue: networkModel.hiddenValues[index] || 0
                                nodeId: "hidden_" + index

                                Connections {
                                    target: networkModel
                                    function onNodeActivated(layer, nodeIndex) {
                                        if (layer === 1 && nodeIndex === index) {
                                            hiddenNode.activateNode()
                                        }
                                    }
                                    function onNodeError(layer, nodeIndex) {
                                        if (layer === 1 && nodeIndex === index) {
                                            hiddenNode.errorNode()
                                        }
                                    }
                                }
                            }
                        }

                        // 输出层节点
                        NetworkNode {
                            id: outputNode
                            x: networkContainer.outputLayerX
                            y: networkContainer.outputLayerY
                            width: networkContainer.nodeSize
                            height: networkContainer.nodeSize
                            nodeValue: networkModel.outputValues[0] || 0
                            nodeId: "output_0"

                            Connections {
                                target: networkModel
                                function onNodeActivated(layer, nodeIndex) {
                                    if (layer === 2 && nodeIndex === 0) {
                                        outputNode.activateNode()
                                    }
                                }
                                function onNodeError(layer, nodeIndex) {
                                    if (layer === 2 && nodeIndex === 0) {
                                        outputNode.errorNode()
                                    }
                                }
                            }
                        }

                        // 连接线层
                        Item {
                            anchors.fill: parent
                            z: -1

                            // 输入到隐藏层连接
                            Repeater {
                                model: 6
                                NetworkConnection {
                                    id: ihConnection
                                    property int inputIndex: Math.floor(index / 3)
                                    property int hiddenIndex: index % 3

                                    anchors.fill: parent
                                    startX: networkContainer.inputLayerX + networkContainer.nodeRadius
                                    startY: networkContainer.inputLayerY + networkContainer.nodeRadius + inputIndex * networkContainer.inputSpacing
                                    endX: networkContainer.hiddenLayerX + networkContainer.nodeRadius
                                    endY: networkContainer.hiddenLayerY + networkContainer.nodeRadius + hiddenIndex * networkContainer.hiddenSpacing
                                    weightValue: networkModel.weightsIH[index] || 0
                                    connectionId: "ih_" + inputIndex + "_" + hiddenIndex

                                    Connections {
                                        target: networkModel
                                        function onConnectionActivated(fromLayer, fromIndex, toLayer, toIndex) {
                                            if (fromLayer === 0 && fromIndex === ihConnection.inputIndex &&
                                                toLayer === 1 && toIndex === ihConnection.hiddenIndex) {
                                                ihConnection.activateConnection()
                                            }
                                        }
                                        function onConnectionError(fromLayer, fromIndex, toLayer, toIndex) {
                                            if (fromLayer === 0 && fromIndex === ihConnection.inputIndex &&
                                                toLayer === 1 && toIndex === ihConnection.hiddenIndex) {
                                                ihConnection.errorConnection()
                                            }
                                        }
                                    }
                                }
                            }

                            // 隐藏到输出层连接
                            Repeater {
                                model: 3
                                NetworkConnection {
                                    id: hoConnection
                                    anchors.fill: parent
                                    startX: networkContainer.hiddenLayerX + networkContainer.nodeRadius
                                    startY: networkContainer.hiddenLayerY + networkContainer.nodeRadius + index * networkContainer.hiddenSpacing
                                    endX: networkContainer.outputLayerX + networkContainer.nodeRadius
                                    endY: networkContainer.outputLayerY + networkContainer.nodeRadius
                                    weightValue: networkModel.weightsHO[index] || 0
                                    connectionId: "ho_" + index

                                    Connections {
                                        target: networkModel
                                        function onConnectionActivated(fromLayer, fromIndex, toLayer, toIndex) {
                                            if (fromLayer === 1 && fromIndex === index && toLayer === 2 && toIndex === 0) {
                                                hoConnection.activateConnection()
                                            }
                                        }
                                        function onConnectionError(fromLayer, fromIndex, toLayer, toIndex) {
                                            if (fromLayer === 1 && fromIndex === index && toLayer === 2 && toIndex === 0) {
                                                hoConnection.errorConnection()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 训练遮罩层
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.5
        visible: networkModel.isTraining
        z: 1000

        MouseArea {
            anchors.fill: parent
            // 阻止所有鼠标事件
        }
    }

    // 样本添加对话框
    Rectangle {
        id: sampleDialog
        width: 300
        height: 220
        color: "white"
        border.color: "#ccc"
        border.width: 2
        radius: 8
        visible: false
        z: 1001

        anchors.centerIn: parent

        property real dataX: 0
        property real dataY: 0

        function open() {
            if (!networkModel.isTraining) {
                visible = true
            }
        }

        function close() {
            visible = false
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "添加训练样本"
                font.pixelSize: 16
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "坐标: (" + sampleDialog.dataX.toFixed(3) + ", " + sampleDialog.dataY.toFixed(3) + ")"
                font.pixelSize: 14
            }

            Text {
                text: "请选择蠓虫类别:"
                font.pixelSize: 14
            }

            Row {
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    width: 80
                    height: 40
                    color: aMouseArea.containsMouse ? "#d32f2f" : "#e74c3c"
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "A类"
                        color: "white"
                        font.bold: true
                    }

                    MouseArea {
                        id: aMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            networkModel.addTrainingSample(sampleDialog.dataX, sampleDialog.dataY, true)
                            sampleDialog.close()
                        }
                    }
                }

                Rectangle {
                    width: 80
                    height: 40
                    color: bMouseArea.containsMouse ? "#1976D2" : "#2196F3"
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "B类"
                        color: "white"
                        font.bold: true
                    }

                    MouseArea {
                        id: bMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            networkModel.addTrainingSample(sampleDialog.dataX, sampleDialog.dataY, false)
                            sampleDialog.close()
                        }
                    }
                }
            }

            Rectangle {
                width: 80
                height: 40
                color: cancelMouseArea.containsMouse ? "#666" : "#999"
                radius: 4
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    anchors.centerIn: parent
                    text: "取消"
                    color: "white"
                    font.bold: true
                }

                MouseArea {
                    id: cancelMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: sampleDialog.close()
                }
            }
        }
    }
}

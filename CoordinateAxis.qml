import QtQuick 2.15

Rectangle {
    id: root
    color: "#fafafa"
    border.color: "#ddd"
    border.width: 1

    property var trainingDataA: []
    property var trainingDataB: []
    property point testPoint: Qt.point(-1, -1)
    property bool showDecisionBoundary: false
    property var decisionBoundaryPoints: []

    signal leftClicked(real dataX, real dataY)
    signal rightClicked(real dataX, real dataY)

    // 坐标系参数
    readonly property real marginLeft: 60
    readonly property real marginBottom: 50
    readonly property real marginTop: 30
    readonly property real marginRight: 30

    readonly property real plotWidth: width - marginLeft - marginRight
    readonly property real plotHeight: height - marginTop - marginBottom

    // 数据范围
    readonly property real minX: 1.0
    readonly property real maxX: 1.7
    readonly property real minY: 1.2
    readonly property real maxY: 2.2

    // 坐标转换函数
    function dataToScreenX(dataX) {
        return marginLeft + (dataX - minX) / (maxX - minX) * plotWidth
    }

    function dataToScreenY(dataY) {
        return height - marginBottom - (dataY - minY) / (maxY - minY) * plotHeight
    }

    function screenToDataX(screenX) {
        return minX + (screenX - marginLeft) / plotWidth * (maxX - minX)
    }

    function screenToDataY(screenY) {
        return minY + (height - marginBottom - screenY) / plotHeight * (maxY - minY)
    }

    // 鼠标区域
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) {
            if (mouse.x >= marginLeft && mouse.x <= width - marginRight &&
                mouse.y >= marginTop && mouse.y <= height - marginBottom) {

                var dataX = screenToDataX(mouse.x)
                var dataY = screenToDataY(mouse.y)

                if (mouse.button === Qt.LeftButton) {
                    root.leftClicked(dataX, dataY)
                } else if (mouse.button === Qt.RightButton) {
                    root.rightClicked(dataX, dataY)
                }
            }
        }
    }

    // 绘制坐标轴
    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // 绘制坐标轴
            ctx.strokeStyle = "#333"
            ctx.lineWidth = 2
            ctx.beginPath()

            // X轴
            ctx.moveTo(marginLeft, height - marginBottom)
            ctx.lineTo(width - marginRight, height - marginBottom)

            // Y轴
            ctx.moveTo(marginLeft, marginTop)
            ctx.lineTo(marginLeft, height - marginBottom)

            ctx.stroke()

            // 绘制网格线
            ctx.strokeStyle = "#e0e0e0"
            ctx.lineWidth = 1

            // 垂直网格线
            for (var x = minX; x <= maxX; x += 0.1) {
                var screenX = dataToScreenX(x)
                ctx.beginPath()
                ctx.moveTo(screenX, marginTop)
                ctx.lineTo(screenX, height - marginBottom)
                ctx.stroke()
            }

            // 水平网格线
            for (var y = minY; y <= maxY; y += 0.1) {
                var screenY = dataToScreenY(y)
                ctx.beginPath()
                ctx.moveTo(marginLeft, screenY)
                ctx.lineTo(width - marginRight, screenY)
                ctx.stroke()
            }

            // 绘制刻度和标签
            ctx.strokeStyle = "#333"
            ctx.lineWidth = 2
            ctx.font = "12px Arial"
            ctx.fillStyle = "#666"
            ctx.textAlign = "center"

            // X轴刻度
            for (x = minX; x <= maxX; x += 0.1) {
                screenX = dataToScreenX(x)
                ctx.beginPath()
                ctx.moveTo(screenX, height - marginBottom)
                ctx.lineTo(screenX, height - marginBottom + 5)
                ctx.stroke()

                if (Math.abs(x - Math.round(x * 10) / 10) < 0.01) {
                    ctx.fillText(x.toFixed(1), screenX, height - marginBottom + 20)
                }
            }

            // Y轴刻度
            ctx.textAlign = "right"
            for (y = minY; y <= maxY; y += 0.1) {
                screenY = dataToScreenY(y)
                ctx.beginPath()
                ctx.moveTo(marginLeft - 5, screenY)
                ctx.lineTo(marginLeft, screenY)
                ctx.stroke()

                if (Math.abs(y - Math.round(y * 10) / 10) < 0.01) {
                    ctx.fillText(y.toFixed(1), marginLeft - 10, screenY + 4)
                }
            }

            // 轴标签
            ctx.textAlign = "center"
            ctx.font = "14px Arial"
            ctx.fillStyle = "#333"
            ctx.fillText("触角长度", width / 2, height - 10)

            ctx.save()
            ctx.translate(15, height / 2)
            ctx.rotate(-Math.PI / 2)
            ctx.fillText("翅膀长度", 0, 0)
            ctx.restore()
        }
    }

    // A类数据点
    Repeater {
        model: (trainingDataA && trainingDataA.length) ? trainingDataA.length : 0
        DataPoint {
            property var pointData: (trainingDataA && index < trainingDataA.length) ? trainingDataA[index] : null
            x: pointData ? dataToScreenX(pointData.x) - width/2 : 0
            y: pointData ? dataToScreenY(pointData.y) - height/2 : 0
            pointColor: "#e74c3c"
            pointLabel: "A"
            visible: pointData !== null && pointData !== undefined
            z: 10  // 确保数据点在网格线上方
        }
    }

    // B类数据点
    Repeater {
        model: (trainingDataB && trainingDataB.length) ? trainingDataB.length : 0
        DataPoint {
            property var pointData: (trainingDataB && index < trainingDataB.length) ? trainingDataB[index] : null
            x: pointData ? dataToScreenX(pointData.x) - width/2 : 0
            y: pointData ? dataToScreenY(pointData.y) - height/2 : 0
            pointColor: "#3498db"
            pointLabel: "B"
            visible: pointData !== null && pointData !== undefined
            z: 10  // 确保数据点在网格线上方
        }
    }

    // 测试点
    DataPoint {
        visible: testPoint.x >= 0
        x: visible ? dataToScreenX(testPoint.x) - width/2 : 0
        y: visible ? dataToScreenY(testPoint.y) - height/2 : 0
        pointColor: "#f39c12"
        pointLabel: "?"
        isTestPoint: true
        z: 15  // 测试点在最上层
    }

    // 决策边界
    Canvas {
        id: boundaryCanvas
        anchors.fill: parent
        visible: showDecisionBoundary && decisionBoundaryPoints && decisionBoundaryPoints.length > 0
        z: 5  // 在网格线上方，数据点下方

        onPaint: {
            if (!visible) return

            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (decisionBoundaryPoints && decisionBoundaryPoints.length > 0) {
                ctx.strokeStyle = "#9b59b6"
                ctx.lineWidth = 4
                ctx.setLineDash([8, 4])

                ctx.beginPath()
                for (var i = 0; i < decisionBoundaryPoints.length; i++) {
                    var point = decisionBoundaryPoints[i]
                    var screenX = dataToScreenX(point.x)
                    var screenY = dataToScreenY(point.y)

                    if (i === 0) {
                        ctx.moveTo(screenX, screenY)
                    } else {
                        ctx.lineTo(screenX, screenY)
                    }
                }
                ctx.stroke()
            }
        }
    }

    // 监听决策边界点变化
    onDecisionBoundaryPointsChanged: {
        boundaryCanvas.requestPaint()
    }

    // 调试信息
    Component.onCompleted: {
        console.log("CoordinateAxis initialized")
        console.log("Training data A count:", trainingDataA ? trainingDataA.length : "undefined")
        console.log("Training data B count:", trainingDataB ? trainingDataB.length : "undefined")
    }

    onTrainingDataAChanged: {
        console.log("Training data A updated, count:", trainingDataA ? trainingDataA.length : "undefined")
    }

    onTrainingDataBChanged: {
        console.log("Training data B updated, count:", trainingDataB ? trainingDataB.length : "undefined")
    }

    // 图例 - 调整图例中的圆点大小
    Rectangle {
        width: 160
        height: 90
        color: "white"
        border.color: "#ddd"
        border.width: 1
        radius: 6
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        z: 20

        Column {
            anchors.centerIn: parent
            spacing: 10

            Text {
                text: "图例"
                font.bold: true
                font.pixelSize: 14
                color: "#2c3e50"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    width: 18  // 从12增加到18
                    height: 18
                    radius: 9
                    color: "#e74c3c"
                    border.color: "white"
                    border.width: 2
                }
                Text {
                    text: "A类蠓虫"
                    font.pixelSize: 12
                    color: "#2c3e50"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    width: 18  // 从12增加到18
                    height: 18
                    radius: 9
                    color: "#3498db"
                    border.color: "white"
                    border.width: 2
                }
                Text {
                    text: "B类蠓虫"
                    font.pixelSize: 12
                    color: "#2c3e50"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}

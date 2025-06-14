import QtQuick 2.15

Item {
    id: root

    property real startX: 0
    property real startY: 0
    property real endX: 100
    property real endY: 100
    property real weightValue: 0.0
    property string connectionId: ""
    property bool isActive: false
    property bool isError: false

    Timer {
        id: activeResetTimer
        interval: 800
        repeat: false
        onTriggered: {
            root.isActive = false
            flowDot.visible = false
        }
    }

    Timer {
        id: errorResetTimer
        interval: 800
        repeat: false
        onTriggered: {
            root.isError = false
            flowDot.visible = false
        }
    }

    // 连接线
    Rectangle {
        id: connectionLine
        width: Math.sqrt(Math.pow(endX - startX, 2) + Math.pow(endY - startY, 2))
        height: isActive || isError ? 3 : 2
        color: isError ? "#f56565" : (isActive ? "#4299e1" : "#a0aec0")
        antialiasing: true

        x: startX
        y: startY - height / 2
        transformOrigin: Item.Left
        rotation: Math.atan2(endY - startY, endX - startX) * 180 / Math.PI

        Behavior on color { ColorAnimation { duration: 300 } }
        Behavior on height { NumberAnimation { duration: 300 } }
    }

    // 权重标签
    Rectangle {
        id: weightLabel
        width: Math.max(weightText.implicitWidth + 8, 35)
        height: weightText.implicitHeight + 4

        x: (startX + endX) / 2 - width / 2
        y: (startY + endY) / 2 - height / 2 - 15

        color: isError ? "#f56565" : (isActive ? "#4299e1" : "white")
        border.color: isError ? "#f56565" : (isActive ? "#4299e1" : "#cbd5e0")
        border.width: 1
        radius: 3
        antialiasing: true

        // 简单阴影
        Rectangle {
            width: parent.width
            height: parent.height
            color: "black"
            opacity: 0.08
            radius: parent.radius
            x: 1
            y: 1
            z: -1
        }

        Text {
            id: weightText
            anchors.centerIn: parent
            text: root.weightValue.toFixed(2)
            font.pixelSize: 9
            font.bold: true
            color: root.isActive || root.isError ? "white" : "#2d3748"
        }

        Behavior on color { ColorAnimation { duration: 300 } }
        Behavior on border.color { ColorAnimation { duration: 300 } }
    }

    // 流动点
    Rectangle {
        id: flowDot
        width: 8
        height: 8
        radius: 4
        color: isError ? "#f56565" : "#4299e1"
        visible: false
        antialiasing: true

        // 添加发光效果
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 4
            height: parent.height + 4
            radius: (parent.width + 4) / 2
            color: parent.color
            opacity: 0.4
        }

        x: startX - width / 2
        y: startY - height / 2

        Behavior on x { NumberAnimation { duration: 600; easing.type: Easing.InOutQuad } }
        Behavior on y { NumberAnimation { duration: 600; easing.type: Easing.InOutQuad } }
    }

    function activateConnection() {
        isActive = true
        flowDot.visible = true

        // 从起点流向终点
        flowDot.x = startX - flowDot.width / 2
        flowDot.y = startY - flowDot.height / 2

        // 动画到终点
        flowDot.x = endX - flowDot.width / 2
        flowDot.y = endY - flowDot.height / 2

        activeResetTimer.start()
    }

    function errorConnection() {
        isError = true
        flowDot.visible = true

        // 从终点流向起点（反向传播）
        flowDot.x = endX - flowDot.width / 2
        flowDot.y = endY - flowDot.height / 2

        // 动画到起点
        flowDot.x = startX - flowDot.width / 2
        flowDot.y = startY - flowDot.height / 2

        errorResetTimer.start()
    }
}
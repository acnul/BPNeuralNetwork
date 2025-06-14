import QtQuick 2.15

Rectangle {
    id: root

    property real nodeValue: 0.0
    property string nodeId: ""
    property bool isActive: false
    property bool isError: false

    width: 60
    height: 60
    radius: 30
    color: isError ? "#f56565" : (isActive ? "#4299e1" : "#e2e8f0")
    border.width: 3
    border.color: "#cbd5e0"

    Behavior on color {
        ColorAnimation { duration: 300 }
    }

    Behavior on scale {
        NumberAnimation { duration: 300 }
    }

    Text {
        anchors.centerIn: parent
        text: root.nodeValue.toFixed(3)
        font.bold: true
        font.pixelSize: 10
        color: root.isActive || root.isError ? "white" : "black"
    }

    // 脉搏动画
    SequentialAnimation {
        id: pulseAnimation
        NumberAnimation { target: root; property: "scale"; to: 1.15; duration: 250 }
        NumberAnimation { target: root; property: "scale"; to: 1.0; duration: 250 }
    }

    // 重置Timer-用于激活状态
    Timer {
        id: activeResetTimer
        interval: 800
        repeat: false
        onTriggered: {
            root.isActive = false
        }
    }

    // 重置Timer-用于错误状态
    Timer {
        id: errorResetTimer
        interval: 800
        repeat: false
        onTriggered: {
            root.isError = false
        }
    }

    // 发光效果
    Rectangle {
        id: glowEffect
        anchors.centerIn: parent
        width: parent.width + 10
        height: parent.height + 10
        radius: (parent.width + 10) / 2
        color: "transparent"
        border.width: 0
        border.color: root.isError ? "#f56565" : "#4299e1"
        opacity: 0

        // 发光透明度动画
        NumberAnimation {
            id: glowAnimation
            target: glowEffect
            property: "opacity"
            from: 0
            to: 0.6
            duration: 300

            onFinished: {
                glowFadeAnimation.start()
            }
        }

        // 发光消失动画
        NumberAnimation {
            id: glowFadeAnimation
            target: glowEffect
            property: "opacity"
            to: 0
            duration: 300
        }

        // 边框宽度动画
        NumberAnimation {
            id: glowBorderAnimation
            target: glowEffect
            property: "border.width"
            from: 0
            to: 3
            duration: 300
        }
    }

    function activateNode() {
        isActive = true
        pulseAnimation.start()
        glowAnimation.start()
        glowBorderAnimation.start()
        activeResetTimer.start()
    }

    function errorNode() {
        isError = true
        pulseAnimation.start()
        glowAnimation.start()
        glowBorderAnimation.start()
        errorResetTimer.start()
    }
}
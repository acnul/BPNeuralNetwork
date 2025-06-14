import QtQuick 2.15

Item {
    width: 40  // 从20增加到40
    height: 40

    property color pointColor: "#e74c3c"
    property string pointLabel: "A"
    property bool isTestPoint: false

    Rectangle {
        id: point
        anchors.centerIn: parent
        width: isTestPoint ? 36 : 28  // 从18/14增加到36/28
        height: width
        radius: width / 2
        color: pointColor
        border.width: isTestPoint ? 4 : 3  // 稍微增加边框宽度
        border.color: isTestPoint ? "#2c3e50" : "white"

        // 添加阴影效果
        Rectangle {
            id: shadow
            anchors.centerIn: parent
            width: parent.width + 4
            height: parent.height + 4
            radius: width / 2
            color: "black"
            opacity: 0.2
            z: -1
            x: 2
            y: 2
        }

        // 脉搏动画（仅测试点）
        SequentialAnimation {
            running: isTestPoint
            loops: Animation.Infinite
            NumberAnimation {
                target: point
                property: "scale"
                to: 1.3
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: point
                property: "scale"
                to: 1.0
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }
    }

    Text {
        anchors.centerIn: point
        text: pointLabel
        font.pixelSize: isTestPoint ? 18 : 16  // 从12/10增加到18/16
        font.bold: true
        color: "white"
        visible: !isTestPoint
    }

    // 改进的工具提示
    Rectangle {
        id: tooltip
        visible: mouseArea.containsMouse
        width: tooltipText.implicitWidth + 16
        height: tooltipText.implicitHeight + 10
        color: "#2c3e50"
        radius: 8
        x: point.width + 12
        y: -height / 2 + point.height / 2
        z: 100

        // 工具提示阴影
        Rectangle {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            radius: parent.radius
            color: "black"
            opacity: 0.2
            x: 2
            y: 2
            z: -1
        }

        Text {
            id: tooltipText
            anchors.centerIn: parent
            text: pointLabel + " 类"
            color: "white"
            font.pixelSize: 13
            font.bold: true
        }

        // 工具提示箭头
        Rectangle {
            width: 10
            height: 10
            color: "#2c3e50"
            rotation: 45
            anchors.right: parent.left
            anchors.rightMargin: -5
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    // 添加点击时的涟漪效果
    Rectangle {
        id: ripple
        anchors.centerIn: point
        width: 0
        height: 0
        radius: width / 2
        color: pointColor
        opacity: 0
        z: -1

        NumberAnimation {
            id: rippleAnimation
            target: ripple
            properties: "width,height"
            to: 80  // 从40增加到80
            duration: 300
            easing.type: Easing.OutQuad
        }

        NumberAnimation {
            id: rippleOpacity
            target: ripple
            property: "opacity"
            from: 0.6
            to: 0
            duration: 300
        }

        function triggerRipple() {
            ripple.width = 0
            ripple.height = 0
            ripple.opacity = 0.6
            rippleAnimation.start()
            rippleOpacity.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: ripple.triggerRipple()
    }
}

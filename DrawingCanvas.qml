import QtQuick 2.15

Canvas {
    id: canvas
    width: 280
    height: 280

    property bool isDrawing: false
    property point lastPoint
    property int brushSize: 20
    property color brushColor: "white"
    property bool hasDrawing: false
    property bool canvasReady: false

    signal drawingChanged()
    signal imageGrabbed(var image)

    // 等待Canvas完全准备好
    onAvailableChanged: {
        console.log("Canvas available changed:", available)
        if (available) {
            canvasReady = true
            initializeCanvas()
        }
    }

    Component.onCompleted: {
        console.log("Canvas completed, available:", available)
        if (available) {
            canvasReady = true
            initializeCanvas()
        }
    }

    function initializeCanvas() {
        if (!canvasReady) {
            console.log("Canvas not ready for initialization")
            return
        }

        try {
            var ctx = getContext("2d");
            if (ctx) {
                ctx.fillStyle = "black";
                ctx.fillRect(0, 0, width, height);
                requestPaint();
                console.log("Canvas initialized successfully");
            }
        } catch (e) {
            console.log("Canvas initialization error:", e.toString());
        }
    }

    onPaint: {
        if (!canvasReady) return

        if (!hasDrawing) {
            try {
                var ctx = getContext("2d");
                if (ctx) {
                    ctx.fillStyle = "black";
                    ctx.fillRect(0, 0, width, height);
                }
            } catch (e) {
                console.log("Paint error:", e.toString());
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onPressed: function(mouse) {
            console.log("Mouse pressed, canvas ready:", canvas.canvasReady)
            if (!canvas.canvasReady) return

            canvas.isDrawing = true;
            canvas.lastPoint = Qt.point(mouse.x, mouse.y);
            drawPoint(mouse.x, mouse.y);
            canvas.hasDrawing = true;
            console.log("Drawing started, hasDrawing:", canvas.hasDrawing)
        }

        onPositionChanged: function(mouse) {
            if (canvas.isDrawing && canvas.canvasReady) {
                drawLine(canvas.lastPoint.x, canvas.lastPoint.y, mouse.x, mouse.y);
                canvas.lastPoint = Qt.point(mouse.x, mouse.y);
            }
        }

        onReleased: {
            canvas.isDrawing = false;
            if (canvas.canvasReady) {
                console.log("Drawing finished, hasDrawing:", canvas.hasDrawing)
                canvas.drawingChanged();
            }
        }
    }

    function drawPoint(x, y) {
        if (!canvasReady) return

        try {
            var ctx = getContext("2d");
            if (ctx) {
                ctx.globalCompositeOperation = "source-over";
                ctx.fillStyle = canvas.brushColor;
                ctx.beginPath();
                ctx.arc(x, y, canvas.brushSize / 2, 0, Math.PI * 2);
                ctx.fill();
                canvas.requestPaint();
            }
        } catch (e) {
            console.log("Draw point error:", e.toString());
        }
    }

    function drawLine(x1, y1, x2, y2) {
        if (!canvasReady) return

        try {
            var ctx = getContext("2d");
            if (ctx) {
                ctx.globalCompositeOperation = "source-over";
                ctx.strokeStyle = canvas.brushColor;
                ctx.lineWidth = canvas.brushSize;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                ctx.beginPath();
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);
                ctx.stroke();
                canvas.requestPaint();
            }
        } catch (e) {
            console.log("Draw line error:", e.toString());
        }
    }

    function clearCanvas() {
        if (!canvasReady) return

        try {
            var ctx = getContext("2d");
            if (ctx) {
                ctx.clearRect(0, 0, width, height);
                ctx.fillStyle = "black";
                ctx.fillRect(0, 0, width, height);
                canvas.requestPaint();
                canvas.hasDrawing = false;
                console.log("Canvas cleared, hasDrawing:", canvas.hasDrawing)
                canvas.drawingChanged();
            }
        } catch (e) {
            console.log("Clear canvas error:", e.toString());
        }
    }

    function captureImage() {
        console.log("Capture image called, canvas ready:", canvasReady, "has drawing:", hasDrawing)

        if (!canvasReady) {
            console.log("Canvas not ready for capture");
            return
        }

        console.log("Starting image capture...");

        // 确保画布已渲染
        canvas.requestPaint();

        // 使用grabToImage获取图像
        canvas.grabToImage(function(result) {
            console.log("Image capture result:", result.image ? "success" : "failed");
            if (result.image) {
                canvas.imageGrabbed(result.image);
            } else {
                console.log("Failed to capture canvas image");
            }
        }, Qt.size(280, 280));
    }
}

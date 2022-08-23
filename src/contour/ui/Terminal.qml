import Contour.Terminal 1.0
import QtQuick 2.6
import QtQuick.Controls 2.6
import QtMultimedia 5.15
import QtQuick.Window 2.6

ContourTerminal
{
    property url bellSoundSource: "qrc:/contour/bell.wav"

    signal showNotification(string title, string content)

    id: vtWidget
    visible: true
    focus: true

    session: terminalSessions.createSession()

    ScrollBar {
        id: vbar

        hoverEnabled: true
        active: hovered || pressed
        width: 20
        minimumSize: 0.1
        orientation: Qt.Vertical
        size: vtWidget.session.pageLineCount / (vtWidget.session.pageLineCount + vtWidget.session.historyLineCount)
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    SoundEffect {
        id: bellSoundEffect
        source: vtWidget.bellSoundSource
    }

    RequestPermission {
        id: requestFontChangeDialog
        text: "The host application is requesting to change the display font."
        onYes: {
            const rememberChoice = requestFontChangeDialog.clickedRememberChoice();
            console.log("[Terminal] Allowing font change request.", rememberChoice ? "to all" : "single", vtWidget.session)
            vtWidget.session.applyPendingFontChange(true, rememberChoice);
        }
        onNo: {
            const rememberChoice = requestFontChangeDialog.clickedRememberChoice();
            console.log("[Terminal] Denying font change request.", rememberChoice ? "to all" : "single", vtWidget.session)
            vtWidget.session.applyPendingFontChange(false, rememberChoice);
        }
        onRejected: {
            console.log("[Terminal] font change request rejected.", vtWidget.session)
            if (vtWidget.session !== null)
                vtWidget.session.applyPendingFontChange(false, false);
        }
    }

    RequestPermission {
        id: requestBufferCaptureDialog
        text: "The host application is requesting to capture the termainl buffer."
        onYes: {
            const rememberChoice = requestBufferCaptureDialog.clickedRememberChoice();
            console.log("[Terminal] Allowing font change request.", rememberChoice ? "to all" : "single")
            vtWidget.session.executePendingBufferCapture(true, rememberChoice);
        }
        onNo: {
            const rememberChoice = requestBufferCaptureDialog.clickedRememberChoice();
            console.log("[Terminal] Denying font change request.", rememberChoice ? "to all" : "single")
            vtWidget.session.executePendingBufferCapture(false, rememberChoice);
        }
        onRejected: {
            console.log("[Terminal] Buffer capture request rejected.")
            vtWidget.session.executePendingBufferCapture(false, false);
        }
    }

    // Callback, to be invoked whenever the GUI scrollbar has been changed.
    // This will update the VT's viewport respectively.
    function onScrollBarPositionChanged() {
        let vt = vtWidget.session;
        let totalLineCount = (vt.pageLineCount + vt.historyLineCount);

        vt.scrollOffset = vt.historyLineCount - vbar.position * totalLineCount;
    }

    // Callback to be invoked whenever the VT's viewport is changing.
    // This will update the GUI (vertical) scrollbar respectively.
    function updateScrollBarPosition() {
        let vt = vtWidget.session;
        let totalLineCount = (vt.pageLineCount + vt.historyLineCount);

        vbar.position = (vt.historyLineCount - vt.scrollOffset) / totalLineCount;
    }

    onTerminated: {
        console.log("Client process terminated. Closing the window.");
        Window.window.close(); // https://stackoverflow.com/a/53829662/386670
    }

    onSessionChanged: (s) => {
        let vt = vtWidget.session;

        // Connect bell control code with an actual sound effect.
        vt.onBell.connect(bellSoundEffect.play);

        // Link showNotification signal.
        vt.onShowNotification.connect(vtWidget.showNotification);

        // Update the VT's viewport whenever the scrollbar's position changes.
        vbar.onPositionChanged.connect(onScrollBarPositionChanged);

        // Update the scrollbar position whenever the scrollbar size changes, because
        // the position is calculated based on scrollbar's size.
        vbar.onSizeChanged.connect(updateScrollBarPosition);

        // Update the scrollbar's position whenever the VT's viewport changes.
        vt.onScrollOffsetChanged.connect(updateScrollBarPosition);

        // Permission-wall related hooks.
        vt.requestPermissionForFontChange.connect(requestFontChangeDialog.open);
        vt.requestPermissionForBufferCapture.connect(requestBufferCaptureDialog.open);
    }

    // {{{ playground: animation
    // transform: Rotation {
    //     id: rotation
    //     origin { x: vtWidget.width * Screen.devicePixelRatio / 2;
    //              y: vtWidget.height * Screen.devicePixelRatio / 2;
    //              z: 0}
    //     angle: 35
    // }
    // Timer {
    //     interval: 50
    //     running: true
    //     repeat: true
    //     onTriggered: {
    //         rotation.angle += 0.5;
    //     }
    // }
    // }}}
}


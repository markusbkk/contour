// Qt 5
// import QtQuick 2.2
// import QtQuick.Dialogs 1.1

import QtQuick
// import QtQuick.Dialogs
import Qt.labs.platform

MessageDialog {
    id: messageDialog
    // icon: StandardIcon.Question
    // TODO: which permissions exactly? Fill me in!
    title: "Host Application is Requesting Permissions."
    text: "The host application is requesting special permissions. %1".arg("TODO: What perms?")
    buttons: MessageDialog.Yes | MessageDialog.YesToAll | MessageDialog.No | MessageDialog.NoToAll | MessageDialog.Abort

    function clickedRememberChoice() {
        return messageDialog.clickedButton == MessageDialog.YesToAll
            || messageDialog.clickedButton == MessageDialog.NoToAll;
    }

    // onYes: console.log("clicked: yes (to all?)")
    // onNo: console.log("clicked: no (to all?)")
    // onRejected: console.log("clicked: reject")
}

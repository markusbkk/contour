import QtQuick 2.2
import QtQuick.Dialogs 1.1

MessageDialog {
    id: messageDialog
    icon: StandardIcon.Question
    // TODO: which permissions exactly? Fill me in!
    title: "Host Application is Requesting Permissions."
    text: "The host application is requesting special permissions. %1".arg("TODO: What perms?")
    standardButtons: StandardButton.Yes | StandardButton.YesToAll | StandardButton.No | StandardButton.NoToAll | StandardButton.Abort

    function clickedRememberChoice() {
        return messageDialog.clickedButton == StandardButton.YesToAll
            || messageDialog.clickedButton == StandardButton.NoToAll;
    }

    // onYes: console.log("clicked: yes (to all?)")
    // onNo: console.log("clicked: no (to all?)")
    // onRejected: console.log("clicked: reject")
}

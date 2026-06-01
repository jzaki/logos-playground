import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Logos.ForumBackend 1.0

Item {
    id: root
    implicitWidth: 900
    implicitHeight: 650
    Layout.fillWidth: true
    Layout.fillHeight: true

    readonly property color bgPrimary:   "#0A0A0A"
    readonly property color bgSecondary: "#111111"
    readonly property color bgPanel:     "#161616"
    readonly property color border:      "#2a2a2a"
    readonly property color textPrimary: "#FAFAFA"
    readonly property color textSecond:  "#6B7280"
    readonly property color textTertiary:"#4B5563"
    readonly property color accent:      "#7C3AED"
    readonly property color accentHover: "#8B5CF6"
    readonly property color accentPress: "#6D28D9"
    readonly property color unreadBadge: "#EF4444"
    readonly property color postBg:      "#1A1A2E"

    QtObject {
        id: d
        readonly property var backend:    typeof logos !== "undefined" && logos
                                          ? logos.module("forum_ui") : null
        readonly property var topicModel: typeof logos !== "undefined" && logos
                                          ? logos.model("forum_ui", "topicModel") : null
        readonly property var postModel:  typeof logos !== "undefined" && logos
                                          ? logos.model("forum_ui", "postModel") : null

        function statusText() {
            if (!backend) return "No backend"
            switch (backend.forumStatus) {
            case ForumBackend.Disconnected: return "Disconnected"
            case ForumBackend.Connecting:   return "Connecting..."
            case ForumBackend.Connected:    return "Connected"
            case ForumBackend.Stopping:     return "Stopping..."
            default: return ""
            }
        }

        function isConnected() {
            return backend && backend.forumStatus === ForumBackend.Connected
        }

        function shortId(id) {
            return id ? id.substring(0, 8) : "?"
        }
    }

    // ── Backend signal connections ───────────────────────────────────────
    Connections {
        target: d.backend
        ignoreUnknownSignals: true

        function onError(message) {
            errorToast.text = message
            errorToast.visible = true
            errorToastTimer.restart()
        }

        function onTopicCreated(topicId, topicTitle) {
            // Auto-select freshly created topics the user started
        }
    }

    // ── Main layout ──────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: root.bgPrimary

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // ── App header ───────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: root.bgPanel

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 8

                    Text {
                        text: "> λ forum"
                        color: root.accent
                        font.family: "JetBrains Mono"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Rectangle { width: 8; height: 8; radius: 4
                        color: d.isConnected() ? root.accent : root.textTertiary
                    }

                    Text {
                        text: d.backend ? d.backend.currentForumName : ""
                        color: root.textSecond
                        font.family: "JetBrains Mono"
                        font.pixelSize: 12
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: d.backend ? "ID: " + d.backend.myIdentity : ""
                        color: root.textTertiary
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.border }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // ── Left panel: Topics ───────────────────────────────
                Rectangle {
                    Layout.preferredWidth: 280
                    Layout.minimumWidth: 200
                    Layout.fillHeight: true
                    color: root.bgSecondary

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Topics header + new topic button
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            color: root.bgPanel

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12

                                Text {
                                    text: "Topics"
                                    color: root.textPrimary
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 13
                                    font.bold: true
                                    Layout.fillWidth: true
                                }

                                Button {
                                    Layout.preferredWidth: 60
                                    text: "+ new"
                                    enabled: d.isConnected()
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 11

                                    background: Rectangle {
                                        radius: 4
                                        color: parent.enabled
                                            ? (parent.pressed  ? root.accentPress
                                               : parent.hovered ? root.accentHover : root.accent)
                                            : root.textTertiary
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "#FFFFFF" : root.bgPrimary
                                        font: parent.font
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: newTopicDialog.open()
                                }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.border }

                        // Topic list
                        ListView {
                            id: topicList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: d.topicModel

                            delegate: Rectangle {
                                width: topicList.width
                                height: 62
                                color: model.topicId === (d.backend ? d.backend.currentTopicId : "")
                                       ? Qt.rgba(124/255, 58/255, 237/255, 0.12)
                                       : topicArea.containsMouse
                                         ? Qt.rgba(255,255,255,0.03) : "transparent"

                                MouseArea {
                                    id: topicArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (d.backend)
                                            d.backend.selectTopic(model.topicId)
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 10
                                    spacing: 6

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 3

                                        Text {
                                            text: model.topicTitle || "(untitled)"
                                            color: root.textPrimary
                                            font.family: "JetBrains Mono"
                                            font.pixelSize: 12
                                            font.bold: true
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                        RowLayout {
                                            spacing: 6
                                            Text {
                                                text: model.authorId ? d.shortId(model.authorId) : ""
                                                color: root.textTertiary
                                                font.family: "JetBrains Mono"
                                                font.pixelSize: 10
                                            }
                                            Text {
                                                text: model.postCount > 0 ? model.postCount + " posts" : ""
                                                color: root.textTertiary
                                                font.family: "JetBrains Mono"
                                                font.pixelSize: 10
                                            }
                                        }
                                    }

                                    // Unread badge
                                    Rectangle {
                                        visible: (model.unreadCount || 0) > 0
                                        width: 20; height: 20; radius: 10
                                        color: root.unreadBadge
                                        Text {
                                            anchors.centerIn: parent
                                            text: model.unreadCount || ""
                                            color: "white"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }
                                    }
                                }

                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    width: parent.width; height: 1
                                    color: root.border
                                }
                            }

                            // Empty state
                            Text {
                                anchors.centerIn: parent
                                visible: topicList.count === 0
                                text: d.isConnected() ? "No topics yet\nBe the first to post!"
                                                      : "Waiting for connection..."
                                color: root.textTertiary
                                font.family: "JetBrains Mono"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                // Vertical separator
                Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: root.border }

                // ── Right panel: Posts ───────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: root.bgPrimary

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Topic header
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            color: root.bgPanel
                            visible: d.backend && d.backend.currentTopicId !== ""

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16

                                Text {
                                    id: topicTitleLabel
                                    text: {
                                        if (!d.topicModel || !d.backend) return ""
                                        const idx = d.backend.currentTopicId
                                        // Walk the model to find the title
                                        for (let i = 0; i < d.topicModel.rowCount(); i++) {
                                            const item = d.topicModel.get ? d.topicModel.get(i) : null
                                            if (item && item.topicId === idx)
                                                return item.topicTitle
                                        }
                                        return d.backend.currentTopicId.substring(0, 16) + "..."
                                    }
                                    color: root.textPrimary
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 13
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 1
                            color: root.border
                            visible: d.backend && d.backend.currentTopicId !== ""
                        }

                        // Posts list
                        ListView {
                            id: postList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: d.postModel
                            spacing: 10

                            onCountChanged: Qt.callLater(function() { postList.positionViewAtEnd() })

                            header: Item { height: 12 }
                            footer: Item { height: 12 }

                            delegate: Item {
                                width: postList.width
                                height: postCard.height + 4
                                readonly property bool alignRight: model.isMe === true

                                Rectangle {
                                    id: postCard
                                    x: alignRight ? parent.width - width - 16 : 16
                                    width: Math.min(postContent.implicitWidth + 24,
                                                    postList.width * 0.75)
                                    height: authorRow.height + postContent.height + tsLabel.height + 24
                                    radius: 6
                                    color: alignRight ? Qt.rgba(124/255, 58/255, 237/255, 0.25) : root.postBg
                                    border.color: alignRight ? root.accent : root.border
                                    border.width: 1

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 4

                                        // Author + timestamp row
                                        RowLayout {
                                            id: authorRow
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Text {
                                                text: model.isMe ? "You" : d.shortId(model.authorId || "")
                                                color: model.isMe ? root.accentHover : root.accent
                                                font.family: "JetBrains Mono"
                                                font.pixelSize: 11
                                                font.bold: true
                                            }

                                            Text {
                                                id: tsLabel
                                                text: model.timestamp
                                                      ? Qt.formatDateTime(model.timestamp, "hh:mm · dd MMM")
                                                      : ""
                                                color: root.textTertiary
                                                font.family: "JetBrains Mono"
                                                font.pixelSize: 10
                                                Layout.fillWidth: true
                                                horizontalAlignment: alignRight ? Text.AlignRight : Text.AlignLeft
                                            }
                                        }

                                        // Post content
                                        Text {
                                            id: postContent
                                            text: model.content || ""
                                            color: root.textPrimary
                                            font.family: "JetBrains Mono"
                                            font.pixelSize: 13
                                            wrapMode: Text.Wrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }

                            // Empty / no-topic state
                            Text {
                                anchors.centerIn: parent
                                visible: postList.count === 0
                                text: d.backend && d.backend.currentTopicId !== ""
                                      ? "No posts yet — be the first to reply!"
                                      : "Select a topic to read posts"
                                color: root.textTertiary
                                font.family: "JetBrains Mono"
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.border }

                        // Reply input bar
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 52
                            color: root.bgSecondary

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8

                                TextField {
                                    id: replyInput
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    placeholderText: {
                                        if (!d.isConnected()) return "Not connected"
                                        if (!d.backend || d.backend.currentTopicId === "") return "Select a topic to reply"
                                        return "Write a reply..."
                                    }
                                    enabled: d.isConnected()
                                             && d.backend
                                             && d.backend.currentTopicId !== ""
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 13
                                    color: root.textPrimary

                                    background: Rectangle {
                                        radius: 4
                                        color: root.bgPanel
                                        border.color: replyInput.activeFocus ? root.accent : root.border
                                    }

                                    onAccepted: sendReplyBtn.sendReply()
                                }

                                Button {
                                    id: sendReplyBtn
                                    Layout.preferredWidth: 44
                                    Layout.fillHeight: true
                                    text: ">>"
                                    enabled: replyInput.enabled && replyInput.text.trim() !== ""
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 14
                                    font.bold: true

                                    function sendReply() {
                                        const txt = replyInput.text.trim()
                                        if (txt === "" || !d.backend) return
                                        d.backend.postReply(txt)
                                        replyInput.text = ""
                                    }

                                    background: Rectangle {
                                        radius: 4
                                        color: parent.enabled
                                               ? (parent.pressed  ? root.accentPress
                                                  : parent.hovered ? root.accentHover : root.accent)
                                               : root.textTertiary
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "#FFFFFF"
                                        font: parent.font
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: sendReply()
                                }
                            }
                        }
                    }
                }
            }

            // ── Status bar ───────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                color: root.bgPanel

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    Text {
                        text: d.backend ? d.backend.statusMessage : "No backend"
                        color: root.textTertiary
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Text {
                        text: d.statusText()
                        color: d.isConnected() ? root.accent : root.textTertiary
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    // ── New Topic dialog ─────────────────────────────────────────────────
    Dialog {
        id: newTopicDialog
        anchors.centerIn: parent
        width: 500
        modal: true
        padding: 10
        standardButtons: Dialog.Ok | Dialog.Cancel

        background: Rectangle {
            color: root.bgPanel
            border.color: root.border
            radius: 8
        }

        header: Rectangle {
            color: "transparent"
            height: 44
            Text {
                anchors.centerIn: parent
                text: "Start New Topic"
                color: root.textPrimary
                font.family: "JetBrains Mono"
                font.pixelSize: 14
                font.bold: true
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width; height: 1
                color: root.border
            }
        }

        footer: DialogButtonBox {
            background: Rectangle { color: root.bgPanel }
            Component.onCompleted: {
                const okBtn = newTopicDialog.standardButton(Dialog.Ok)
                if (okBtn) {
                    okBtn.text = "Post"
                    okBtn.font.family = "JetBrains Mono"
                    okBtn.font.pixelSize = 12
                    okBtn.palette.button = root.accent
                    okBtn.palette.buttonText = "#FFFFFF"
                }
                const cancelBtn = newTopicDialog.standardButton(Dialog.Cancel)
                if (cancelBtn) {
                    cancelBtn.font.family = "JetBrains Mono"
                    cancelBtn.font.pixelSize = 12
                    cancelBtn.palette.button = root.bgPanel
                    cancelBtn.palette.buttonText = root.textPrimary
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Text {
                text: "Topic title:"
                color: root.textSecond
                font.family: "JetBrains Mono"
                font.pixelSize: 12
            }

            TextField {
                id: topicTitleInput
                Layout.fillWidth: true
                placeholderText: "Enter topic title"
                font.family: "JetBrains Mono"
                font.pixelSize: 12
                color: root.textPrimary
                background: Rectangle {
                    radius: 4
                    color: root.bgPrimary
                    border.color: topicTitleInput.activeFocus ? root.accent : root.border
                }
            }

            Text {
                text: "Opening post:"
                color: root.textSecond
                font.family: "JetBrains Mono"
                font.pixelSize: 12
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                implicitHeight: 120

                TextArea {
                    id: topicContentInput
                    placeholderText: "Write the opening post..."
                    placeholderTextColor: root.textTertiary
                    font.family: "JetBrains Mono"
                    font.pixelSize: 12
                    color: root.textPrimary
                    wrapMode: TextEdit.Wrap
                    background: Rectangle {
                        color: root.bgPrimary
                        border.color: topicContentInput.activeFocus ? root.accent : root.border
                        radius: 4
                    }
                }
            }
        }

        onAccepted: {
            const title   = topicTitleInput.text.trim()
            const content = topicContentInput.text.trim()
            if (title !== "" && content !== "" && d.backend)
                d.backend.startNewTopic(title, content)
            topicTitleInput.text   = ""
            topicContentInput.text = ""
        }
        onRejected: {
            topicTitleInput.text   = ""
            topicContentInput.text = ""
        }
    }

    // ── Error toast ──────────────────────────────────────────────────────
    Rectangle {
        id: errorToast
        visible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        width: toastText.implicitWidth + 32
        height: 36
        radius: 6
        color: "#5c1a1a"
        border.color: "#C62828"

        property alias text: toastText.text

        Text {
            id: toastText
            anchors.centerIn: parent
            color: "#EF4444"
            font.family: "JetBrains Mono"
            font.pixelSize: 12
        }

        Timer {
            id: errorToastTimer
            interval: 4000
            onTriggered: errorToast.visible = false
        }
    }
}

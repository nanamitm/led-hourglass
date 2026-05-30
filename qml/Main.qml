import QtQuick
import QtQuick.Window

Window {
    id: root
    width: 360
    height: 580
    visible: true
    title: "LED Hourglass"
    color: "#0a0a0a"

    // 完了演出
    property bool blinkOn: false
    SequentialAnimation on blinkOn {
        id: blinkAnim
        running: false
        loops: Animation.Infinite
        PropertyAction { value: true  }
        PauseAnimation { duration: 400 }
        PropertyAction { value: false }
        PauseAnimation { duration: 400 }
    }
    Connections {
        target: HourglassSim
        function onCompletionTriggered() { blinkAnim.start() }
        function onStateChanged() { if (!HourglassSim.complete) blinkAnim.stop() }
    }

    // 砂時計 (上端まで広げてステータスバーと重ならない表示はダイヤ角に配置)
    Item {
        id: hourglassArea
        anchors { top: parent.top; left: parent.left; right: parent.right; bottom: bottomBar.top }

        HourglassView { anchors.fill: parent }

        // 完了演出オーバーレイ
        Rectangle {
            anchors.fill: parent; color: "#ffee00"
            opacity: HourglassSim.complete && root.blinkOn ? 0.10 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        // 残り時間 — 上チャンバーの右上コーナー (ダイヤの外側の空き部分)
        Text {
            anchors { top: parent.top; right: parent.right
                      topMargin: parent.height * 0.04
                      rightMargin: parent.width * 0.04 }
            text: {
                if (HourglassSim.complete) return "完了!"
                let s = HourglassSim.remainingSec
                return Math.floor(s/60) + ":" + (s%60 < 10 ? "0" : "") + s%60
            }
            color: HourglassSim.complete ? (root.blinkOn ? "#ffee44" : "#886600")
                                         : (HourglassSim.remainingSec <= 10 ? "#ff6644" : "#aaccff")
            font.pixelSize: 22; font.bold: true
        }

        // 周回カウント — 上チャンバーの左上コーナー (タップでリセット)
        Rectangle {
            visible: HourglassSim.lapCount > 0
            anchors { top: parent.top; left: parent.left
                      topMargin: parent.height * 0.04
                      leftMargin: parent.width * 0.04 }
            width: lapText.width + 14; height: 26; radius: 13
            color: lapMouse.pressed ? "#0a1a2a" : "#1a2a3a"
            border.color: "#2a4a6a"; border.width: 1
            Text {
                id: lapText
                anchors.centerIn: parent
                text: "×" + HourglassSim.lapCount
                color: "#44aaff"; font.pixelSize: 13; font.bold: true
            }
            MouseArea { id: lapMouse; anchors.fill: parent
                        onClicked: HourglassSim.resetLapCount() }
        }
    }

    // 下部バー
    Rectangle {
        id: bottomBar
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 58; color: "#111820"

        Row {
            anchors.centerIn: parent; spacing: 16

            Rectangle {
                width: 44; height: 44; radius: 8
                color: resetMouse.pressed ? "#1a3a1a" : "#162016"
                border.color: "#2a4a2a"; border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                Text { anchors.centerIn: parent; text: "↺"; color: "#66bb66"; font.pixelSize: 22 }
                MouseArea { id: resetMouse; anchors.fill: parent
                            onClicked: { HourglassSim.reset(); blinkAnim.stop() } }
            }

            // 設定ボタン (下部バーに移動: ステータスバーと重ならない)
            Rectangle {
                width: 44; height: 44; radius: 8
                color: gearMouse.pressed ? "#1a1a3a" : "#162030"
                border.color: "#2a2a4a"; border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                Text { anchors.centerIn: parent; text: "⚙"; color: "#88aacc"; font.pixelSize: 22 }
                MouseArea { id: gearMouse; anchors.fill: parent
                            onClicked: settingsPanel.open = !settingsPanel.open }
            }

            Column {
                spacing: 4; anchors.verticalCenter: parent.verticalCenter
                Rectangle {
                    width: 180; height: 8; radius: 4; color: "#1a2030"
                    Rectangle {
                        width: parent.width * HourglassSim.bottomGrains / HourglassSim.totalGrains
                        height: parent.height; radius: 4
                        color: HourglassSim.complete ? "#ffcc00" : "#4488ff"
                        Behavior on width { NumberAnimation { duration: 200 } }
                    }
                }
                Text {
                    text: HourglassSim.bottomGrains + " / " + HourglassSim.totalGrains + " grains"
                    color: "#446688"; font.pixelSize: 11
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // 設定パネル
    Rectangle {
        id: settingsPanel
        property bool open: false
        width: 240
        anchors { top: parent.top; bottom: bottomBar.top }
        anchors.right: parent.right
        opacity: open ? 1.0 : 0.0
        visible: opacity > 0
        z: 20; color: "#111827"; border.color: "#2a3a4a"; border.width: 1; clip: true
        Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

        // タイトル行 (スクロール対象外で常に表示)
        Row {
            id: panelTitle
            anchors { top: parent.top; left: parent.left; right: parent.right; margins: 16 }
            height: 36
            Text { text: "設定"; color: "#aaccff"; font.pixelSize: 16; font.bold: true
                   width: parent.width - 28; anchors.verticalCenter: parent.verticalCenter }
            Rectangle {
                width: 26; height: 26; radius: 13; anchors.verticalCenter: parent.verticalCenter
                color: closeMouse.pressed ? "#334" : "transparent"
                Text { anchors.centerIn: parent; text: "✕"; color: "#88aacc"; font.pixelSize: 14 }
                MouseArea { id: closeMouse; anchors.fill: parent
                            onClicked: settingsPanel.open = false }
            }
        }

        // スクロール可能なコンテンツ
        Flickable {
            id: settingsFlickable
            anchors { top: panelTitle.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

            contentWidth: width
            contentHeight: settingsContent.implicitHeight + 20
            clip: true

            Column {
            id: settingsContent
            width: settingsFlickable.width
            topPadding: 8; bottomPadding: 8
            leftPadding: 16; rightPadding: 16
            spacing: 16

            Rectangle { width: parent.width; height: 1; color: "#2a3a4a" }

            // カラー選択
            Column { width: parent.width; spacing: 8
                Text { text: "カラー"; color: "#88aacc"; font.pixelSize: 13 }

                // プリセット
                Row { spacing: 8; anchors.horizontalCenter: parent.horizontalCenter
                    Repeater {
                        model: ListModel {
                            ListElement { clabel: "赤"; chue: 0   }
                            ListElement { clabel: "金"; chue: 45  }
                            ListElement { clabel: "緑"; chue: 120 }
                            ListElement { clabel: "水"; chue: 200 }
                            ListElement { clabel: "紫"; chue: 270 }
                        }
                        delegate: Rectangle {
                            width: 38; height: 38; radius: 8
                            color: Qt.hsva(chue/360, 0.9, 0.9, 1)
                            opacity: HourglassSim.colorHue === chue ? 1.0 : 0.4
                            border.color: HourglassSim.colorHue === chue ? "white" : "transparent"
                            border.width: 2
                            Text { anchors.centerIn: parent; text: clabel; color: "white"; font.pixelSize: 11 }
                            MouseArea { anchors.fill: parent
                                        onClicked: HourglassSim.colorHue = chue }
                        }
                    }
                }

                // 色相スライダー
                Item { width: parent.width; height: 28
                    Rectangle {
                        id: hueTrack
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width; height: 8; radius: 4
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.000; color: "#ff0000" }
                            GradientStop { position: 0.167; color: "#ffff00" }
                            GradientStop { position: 0.333; color: "#00ff00" }
                            GradientStop { position: 0.500; color: "#00ffff" }
                            GradientStop { position: 0.667; color: "#0000ff" }
                            GradientStop { position: 0.833; color: "#ff00ff" }
                            GradientStop { position: 1.000; color: "#ff0000" }
                        }
                    }
                    Rectangle {
                        id: hueHandle
                        anchors.verticalCenter: hueTrack.verticalCenter
                        width: 20; height: 20; radius: 10
                        color: Qt.hsva(HourglassSim.colorHue / 360, 0.9, 1.0, 1.0)
                        border.color: "white"; border.width: 2
                        x: (HourglassSim.colorHue / 359) * (hueTrack.width - width)
                        MouseArea {
                            anchors.fill: parent
                            drag.target: hueHandle; drag.axis: Drag.XAxis
                            drag.minimumX: 0; drag.maximumX: hueTrack.width - hueHandle.width
                            onPositionChanged: if (pressed)
                                HourglassSim.colorHue =
                                    Math.round(hueHandle.x / (hueTrack.width - hueHandle.width) * 359)
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: "#2a3a4a" }

            // センサー感度
            Column { width: parent.width; spacing: 8
                Text { text: "センサー感度"; color: "#88aacc"; font.pixelSize: 13 }
                Row { spacing: 8; width: parent.width
                    Text { text: "低"; color: "#556677"; font.pixelSize: 12
                           anchors.verticalCenter: parent.verticalCenter }
                    Item {
                        width: parent.width - 36; height: 28
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            id: sensTrack
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width; height: 5; radius: 2; color: "#333355"
                            Rectangle {
                                width: sensHandle.x + sensHandle.width / 2
                                height: parent.height; radius: 2; color: "#44aaff"
                            }
                        }
                        Rectangle {
                            id: sensHandle
                            anchors.verticalCenter: sensTrack.verticalCenter
                            width: 20; height: 20; radius: 10
                            color: sensMa.pressed ? "#fff" : "#99ccff"
                            border.color: "#44aaff"; border.width: 2
                            x: ((HourglassSim.sensitivity - 0.5) / 1.5) * (sensTrack.width - width)
                            MouseArea {
                                id: sensMa; anchors.fill: parent
                                drag.target: sensHandle; drag.axis: Drag.XAxis
                                drag.minimumX: 0; drag.maximumX: sensTrack.width - sensHandle.width
                                onPositionChanged: if (pressed)
                                    HourglassSim.sensitivity =
                                        0.5 + (sensHandle.x / (sensTrack.width - sensHandle.width)) * 1.5
                            }
                            Connections {
                                target: HourglassSim
                                function onSensitivityChanged() {
                                    if (!sensMa.pressed)
                                        sensHandle.x = ((HourglassSim.sensitivity - 0.5) / 1.5)
                                                       * (sensTrack.width - sensHandle.width)
                                }
                            }
                        }
                    }
                    Text { text: "高"; color: "#556677"; font.pixelSize: 12
                           anchors.verticalCenter: parent.verticalCenter }
                }
                Text { text: HourglassSim.sensitivity.toFixed(1) + "x"
                       color: "#446688"; font.pixelSize: 11 }
            }

            Rectangle { width: parent.width; height: 1; color: "#2a3a4a" }

            // 自動ループ
            Column { width: parent.width; spacing: 8
                Text { text: "自動ループ"; color: "#88aacc"; font.pixelSize: 13 }
                Row { spacing: 10
                    Rectangle {
                        width: 54; height: 28; radius: 14
                        color: HourglassSim.autoLoop ? "#1a5a2a" : "#1a1a2a"
                        border.color: HourglassSim.autoLoop ? "#44cc66" : "#334"; border.width: 1
                        Text { anchors.centerIn: parent
                               text: HourglassSim.autoLoop ? "ON" : "OFF"
                               color: HourglassSim.autoLoop ? "#44cc66" : "#556677"
                               font.pixelSize: 13; font.bold: true }
                        MouseArea { anchors.fill: parent
                                    onClicked: HourglassSim.autoLoop = !HourglassSim.autoLoop }
                    }
                    Text { text: "完了後2秒でリスタート"; color: "#446688"; font.pixelSize: 11
                           anchors.verticalCenter: parent.verticalCenter }
                }
            }

            Rectangle { width: parent.width; height: 1; color: "#2a3a4a" }

            // 砂の量
            Column { width: parent.width; spacing: 8
                Text { text: "砂の量"; color: "#88aacc"; font.pixelSize: 13 }
                Row { spacing: 6; width: parent.width
                    Repeater {
                        model: ListModel {
                            ListElement { dlabel: "少\n120粒"; dval: 1 }
                            ListElement { dlabel: "普\n200粒"; dval: 2 }
                            ListElement { dlabel: "多\n350粒"; dval: 3 }
                        }
                        delegate: Rectangle {
                            width: (parent.width - 12) / 3; height: 44; radius: 6
                            color: HourglassSim.grainDensity === dval ? "#1a3a5a" : "#162030"
                            border.color: HourglassSim.grainDensity === dval ? "#4488ff" : "#2a3a4a"
                            border.width: 1
                            Text { anchors.centerIn: parent; text: dlabel
                                   color: HourglassSim.grainDensity === dval ? "#88ccff" : "#446688"
                                   font.pixelSize: 11; font.bold: true
                                   horizontalAlignment: Text.AlignHCenter }
                            MouseArea { anchors.fill: parent
                                        onClicked: HourglassSim.grainDensity = dval }
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: "#2a3a4a" }

            // 計測時間
            Column { width: parent.width; spacing: 8
                Text { text: "計測時間"; color: "#88aacc"; font.pixelSize: 13 }
                Grid { columns: 2; spacing: 6; width: parent.width
                    Repeater {
                        model: ListModel {
                            ListElement { label: "1分";  sec: 60   }
                            ListElement { label: "3分";  sec: 180  }
                            ListElement { label: "5分";  sec: 300  }
                            ListElement { label: "10分"; sec: 600  }
                        }
                        delegate: Rectangle {
                            width: (parent.width-6)/2; height: 34; radius: 6
                            color: HourglassSim.durationSec===sec ? "#1a3a5a" : "#162030"
                            border.color: HourglassSim.durationSec===sec ? "#4488ff" : "#2a3a4a"; border.width: 1
                            Text { anchors.centerIn: parent; text: label
                                   color: HourglassSim.durationSec===sec ? "#88ccff" : "#446688"
                                   font.pixelSize: 14; font.bold: true }
                            MouseArea { anchors.fill: parent
                                onClicked: {
                                    HourglassSim.durationSec = sec
                                    HourglassSim.reset()
                                    blinkAnim.stop()
                                    settingsPanel.open = false
                                }
                            }
                        }
                    }
                }
            }
        }  // Column (settingsContent)
        }  // Flickable
    }

    MouseArea {
        anchors { top: parent.top; left: parent.left; bottom: bottomBar.top }
        width: root.width - (settingsPanel.open ? settingsPanel.width : 0)
        enabled: settingsPanel.open; z: 15
        onClicked: settingsPanel.open = false
    }
}

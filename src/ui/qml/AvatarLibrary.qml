import QtQuick
import "."

QtObject {
    // Simple built-in avatar presets. In future this can be replaced
    // by real image assets, but for now we use colors and shapes.
    property var avatars: [
        { id: "indigo", label: qsTr("靛蓝"), color: "#6366F1" },
        { id: "emerald", label: qsTr("绿色"), color: "#10B981" },
        { id: "amber", label: qsTr("琥珀"), color: "#F59E0B" },
        { id: "rose", label: qsTr("玫红"), color: "#F97373" },
        { id: "sky", label: qsTr("天空"), color: "#0EA5E9" },
        { id: "slate", label: qsTr("石板"), color: "#64748B" }
    ]

    function colorFor(id) {
        for (var i = 0; i < avatars.length; ++i) {
            if (avatars[i].id === id)
                return avatars[i].color;
        }
        return Style.primaryLight;
    }

    // Derive a stable avatar from a display name (used for peers)
    function avatarForName(name) {
        if (!name || name.length === 0)
            return avatars[0];

        var code = name.charCodeAt(0);
        var index = Math.abs(code) % avatars.length;
        return avatars[index];
    }

    function colorForName(name) {
        var a = avatarForName(name);
        return a.color;
    }

    function initialForName(name) {
        if (!name || name.length === 0)
            return "";
        return name.charAt(0).toUpperCase();
    }
}

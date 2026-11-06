import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtGraphs

import SolTrace

Item {
    id: root

    property var model: App.sun.shape.current_distribution
    property real maxAngle: 1.0
    property real maxIntensity: 1.0
    property bool wireframeVisible: false
    property alias orthographicProjection: graph.orthoProjection
    property alias gridVisible: graphTheme.gridVisible

    readonly property int radialSampleCount: 41
    readonly property int angularSampleCount: 49

    function distributionSamples() {
        const samples = []
        if (!model) {
            return samples
        }

        for (let i = 0; i < model.rowCount(); ++i) {
            samples.push({
                "angle": Math.abs(Number(model.data(model.index(i, 0)))),
                "intensity": Math.max(0.0, Number(model.data(model.index(i, 1))))
            })
        }

        samples.sort(function(a, b) { return a.angle - b.angle })

        const merged = []
        for (let j = 0; j < samples.length; ++j) {
            const previous = merged[merged.length - 1]
            if (previous && Math.abs(previous.angle - samples[j].angle) < 1.0e-6) {
                previous.intensity = Math.max(previous.intensity, samples[j].intensity)
            } else {
                merged.push(samples[j])
            }
        }
        return merged
    }

    function interpolatedIntensity(samples, angle) {
        if (samples.length === 0) {
            return 0.0
        }
        if (angle <= samples[0].angle) {
            return samples[0].intensity
        }

        for (let i = 1; i < samples.length; ++i) {
            if (angle > samples[i].angle) {
                continue
            }

            const left = samples[i - 1]
            const right = samples[i]
            const width = right.angle - left.angle
            if (width <= 1.0e-12) {
                return right.intensity
            }

            const fraction = (angle - left.angle) / width
            return left.intensity
                    + fraction * (right.intensity - left.intensity)
        }
        return samples[samples.length - 1].intensity
    }

    function rebuildSurface() {
        surfaceData.clear()

        const samples = distributionSamples()
        if (samples.length === 0) {
            maxAngle = 1.0
            maxIntensity = 1.0
            return
        }

        maxAngle = Math.max(samples[samples.length - 1].angle, 1.0e-6)
        var peak = 0.0
        for (let i = 0; i < samples.length; ++i) {
            peak = Math.max(peak, samples[i].intensity)
        }
        maxIntensity = Math.max(peak, 1.0e-6)

        // ItemModelSurfaceDataProxy expects a regular row/column grid. Each
        // radial sample is repeated around the full azimuth to form the
        // rotationally symmetric angular emission profile.
        for (let radiusIndex = 0; radiusIndex < radialSampleCount; ++radiusIndex) {
            const radius = maxAngle * radiusIndex / (radialSampleCount - 1)
            const intensity = interpolatedIntensity(samples, radius)

            for (let angleIndex = 0; angleIndex < angularSampleCount; ++angleIndex) {
                surfaceData.append({
                    "radius": radius,
                    "azimuth": 360.0 * angleIndex / (angularSampleCount - 1),
                    "intensity": intensity
                })
            }
        }
    }

    onModelChanged: rebuildSurface()
    Component.onCompleted: rebuildSurface()

    Connections {
        target: root.model

        function onChanged() {
            root.rebuildSurface()
        }
    }

    Connections {
        target: App.sun.shape

        function onChanged() {
            root.rebuildSurface()
        }
    }

    ListModel {
        id: surfaceData
    }

    Gradient {
        id: intensityGradient

        GradientStop { position: 0.0; color: "#1a33b3" }
        GradientStop { position: 0.35; color: "#3f8f91" }
        GradientStop { position: 0.7; color: "#a9d969" }
        GradientStop { position: 1.0; color: "#fff066" }
    }

    Value3DAxis {
        id: azimuthAxis
        min: 0
        max: 360
        segmentCount: 4
        labelFormat: "%.0f°"
        title: "Azimuth"
        titleVisible: true
    }

    Value3DAxis {
        id: intensityAxis
        min: 0
        max: root.maxIntensity * 1.08
        segmentCount: 2
        labelFormat: "%.2f"
        title: "Relative intensity"
        titleVisible: true
    }

    Value3DAxis {
        id: radiusAxis
        min: 0
        max: root.maxAngle
        segmentCount: 2
        labelFormat: "%.2f"
        title: "Angular error (mrad)"
        titleVisible: true
    }

    GraphsTheme {
        id: graphTheme

        colorScheme: GraphsTheme.ColorScheme.Dark
        backgroundVisible: false
        plotAreaBackgroundVisible: false
        labelTextColor: App.theme.fontColor
        labelBorderVisible: false
        labelFont.pointSize: 32
        //grid.mainColor: Qt.alpha(App.theme.fontColor, 0.24)
        //grid.subColor: Qt.alpha(App.theme.fontColor, 0.12)
        //axisX.mainColor: Qt.alpha(App.theme.fontColor, 0.55)
        //axisX.labelTextColor: App.theme.fontColor
        //axisY.mainColor: Qt.alpha(App.theme.fontColor, 0.55)
        //axisY.labelTextColor: App.theme.fontColor
        //axisZ.mainColor: Qt.alpha(App.theme.fontColor, 0.55)
        //axisZ.labelTextColor: App.theme.fontColor
    }

    Surface3D {
        id: graph
        anchors.fill: parent

        theme: graphTheme
        axisX: azimuthAxis
        axisY: intensityAxis
        axisZ: radiusAxis

        polar: true
        aspectRatio: 1.25
        horizontalAspectRatio: 1.0
        cameraPreset: Graphs3D.CameraPreset.IsometricRight
        cameraZoomLevel: 115
        shadowQuality: Graphs3D.ShadowQuality.None
        msaaSamples: 4
        selectionMode: Graphs3D.SelectionFlag.Item

        lightStrength: 0.25
        ambientLightStrength: 0.75

        Surface3DSeries {
            shading: Surface3DSeries.Shading.Smooth
            drawMode: root.wireframeVisible
                      ? Surface3DSeries.DrawSurfaceAndWireframe
                      : Surface3DSeries.DrawSurface
            baseGradient: intensityGradient
            colorStyle: GraphsTheme.ColorStyle.RangeGradient
            itemLabelFormat: "Angular error: @zLabel mrad\nIntensity: @yLabel"

            wireframeColor: Material.dividerColor

            ItemModelSurfaceDataProxy {
                itemModel: surfaceData
                rowRole: "radius"
                columnRole: "azimuth"
                yPosRole: "intensity"
            }
        }
    }

}

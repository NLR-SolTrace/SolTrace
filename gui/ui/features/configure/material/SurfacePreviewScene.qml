import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

import QtQuick.Controls

import SolTrace

View3D {
    id: root

    property real light_scale: 0.001

    function fitGeometry() {
        const geometry = App.materials.geometry_edit.surface_geometry
        if (!geometry || geometry.vertex_count === 0) {
            return
        }

        const bounds = geometry.bounding_box
        let minX = Infinity
        let minY = Infinity
        let minZ = Infinity
        let maxX = -Infinity
        let maxY = -Infinity
        let maxZ = -Infinity

        for (let x = 0; x < 2; ++x) {
            for (let y = 0; y < 2; ++y) {
                for (let z = 0; z < 2; ++z) {
                    const localPoint = Qt.vector3d(x ? bounds.max.x : bounds.min.x,
                                                   y ? bounds.max.y : bounds.min.y,
                                                   z ? bounds.max.z : bounds.min.z)
                    const point = geometry_node.mapPositionToScene(localPoint)
                    minX = Math.min(minX, point.x)
                    minY = Math.min(minY, point.y)
                    minZ = Math.min(minZ, point.z)
                    maxX = Math.max(maxX, point.x)
                    maxY = Math.max(maxY, point.y)
                    maxZ = Math.max(maxZ, point.z)
                }
            }
        }

        camera_controller.fit_bounds(Qt.vector3d(minX, minY, minZ),
                                     Qt.vector3d(maxX, maxY, maxZ))
    }

    Connections {
        target: App.materials.geometry_edit.surface_geometry

        function onVertex_count_changed() {
            if (target.vertex_count > 0) {
                Qt.callLater(root.fitGeometry)
            }
        }
    }

    environment: SceneEnvironment {
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.VeryHigh

        aoStrength: 20
        aoDistance: 10
        aoSoftness: 90
        aoBias: 0.01
        aoSampleRate: 4

        probeExposure: 0.8

        tonemapMode: SceneEnvironment.TonemapModeAces

        backgroundMode: SceneEnvironment.Transparent
        lightProbe: Texture {
            textureData: ProceduralSkyTextureData {
                sunColor: Qt.rgba(0, 0, 0, 0)
                skyTopColor: Qt.rgba(0.55, 0.58, 0.62, 1.0)
                skyHorizonColor: Qt.rgba(0.78, 0.80, 0.84, 1.0)
                groundHorizonColor: Qt.rgba(0.52, 0.54, 0.58, 1.0)
                groundBottomColor: Qt.rgba(0.30, 0.31, 0.34, 1.0)
            }
            mappingMode: Texture.LightProbe
        }

        InfiniteGrid {
            id: infiniteGrid
            visible: true
            gridInterval: 1
        }
    }

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(3,3,3)
        clipNear: 0.1
        clipFar: 1000

        eulerRotation.x: -45
        eulerRotation.y: 45
    }


    DirectionalLight {
        ambientColor: "#50545A"
        eulerRotation: Qt.vector3d(-35, 35, 0)

        color: "#FFAAAA"

        brightness: 120 * light_scale
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-20, -55, 0)

        color: "#AAAAFF"

        brightness: 55 * light_scale
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-15, 160, 0)

        color: "#AAFFAA"

        brightness: 35 * light_scale
    }

    PointLight {
        position: Qt.vector3d(0, 3.5, 3)

        color: "#FFFFFF"

        brightness: 120 * light_scale
        quadraticFade: 0.35
    }

    PointLight {
        position: Qt.vector3d(-3, -2, 2)

        color: "#E7F0FF"

        brightness: 65 * light_scale
        quadraticFade: 0.45
    }

    CameraController {
        id: camera_controller
        anchors.fill: parent
        perspective_camera: camera

        rotation_target: camera_target
    }

    Node {
        id: geometry_node
        rotation: Quaternion.fromEulerAngles(-90,0,0)

        Model {
            geometry: App.materials.geometry_edit.surface_geometry

            castsShadows: true

            materials : [
                PrincipledMaterial {
                    metalness: 0.0
                    roughness: 0.0
                    baseColor: "white"
                }
            ]
        }

        Node {
            id: camera_target

            property var bb: App.materials.geometry_edit.surface_geometry.bounding_box

            property vector3d center: bb.max.minus(bb.min).times(0.5).plus(bb.min)

            position: center
        }

    }
}

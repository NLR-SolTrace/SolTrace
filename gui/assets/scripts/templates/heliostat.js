/*{
  "schema_version": 1,
  "title": "Heliostat",
  "description": [
    "Creates or updates a tracked, rectangular heliostat divided into independently canted facets.",
    "Reuse the same scope ID during the current session to update the existing heliostat, or choose another ID to create a separate instance."
  ],
  "parameters": [
    {
      "id": "scope_id",
      "name": "Scope ID",
      "description": "Stable session identifier used to find this heliostat on later runs.",
      "type": "string",
      "default": "org.nlr.soltrace.heliostat.default"
    },
    {
      "id": "heliostat_name",
      "name": "Name",
      "description": "Display-name prefix for the heliostat and its generated resources.",
      "type": "string",
      "default": "Heliostat"
    },
    {
      "id": "position",
      "name": "Position",
      "description": "Global position of the heliostat tracking origin.",
      "type": "vector3d",
      "default": [0, 0, 0],
      "step": 0.1,
      "decimals": 3
    },
    {
      "id": "target_position",
      "name": "Target position",
      "description": "Global point toward which reflected sunlight is directed.",
      "type": "vector3d",
      "default": [0, 0, 100],
      "step": 0.1,
      "decimals": 3
    },
    {
      "id": "sun_azimuth",
      "name": "Sun azimuth",
      "description": "Current solar azimuth in degrees, clockwise from north.",
      "type": "real",
      "default": 180,
      "minimum": -180,
      "maximum": 180,
      "step": 1,
      "decimals": 2
    },
    {
      "id": "sun_elevation",
      "name": "Sun elevation",
      "description": "Current solar elevation above the horizon in degrees.",
      "type": "real",
      "default": 45,
      "minimum": 0,
      "maximum": 90,
      "step": 1,
      "decimals": 2
    },
    {
      "id": "aperture_width",
      "name": "Aperture width",
      "description": "Total heliostat width before facet gaps are removed.",
      "type": "real",
      "default": 12,
      "minimum": 0.001,
      "step": 0.1,
      "decimals": 3
    },
    {
      "id": "aperture_height",
      "name": "Aperture height",
      "description": "Total heliostat height before facet gaps are removed.",
      "type": "real",
      "default": 10,
      "minimum": 0.001,
      "step": 0.1,
      "decimals": 3
    },
    {
      "id": "panels_x",
      "name": "Facet columns",
      "description": "Number of facet columns across the heliostat.",
      "type": "integer",
      "default": 2,
      "minimum": 1,
      "maximum": 100
    },
    {
      "id": "panels_y",
      "name": "Facet rows",
      "description": "Number of facet rows across the heliostat.",
      "type": "integer",
      "default": 2,
      "minimum": 1,
      "maximum": 100
    },
    {
      "id": "gap_x",
      "name": "Horizontal gap",
      "description": "Horizontal clearance between adjacent facets.",
      "type": "real",
      "default": 0.05,
      "minimum": 0,
      "step": 0.01,
      "decimals": 4
    },
    {
      "id": "gap_y",
      "name": "Vertical gap",
      "description": "Vertical clearance between adjacent facets.",
      "type": "real",
      "default": 0.05,
      "minimum": 0,
      "step": 0.01,
      "decimals": 4
    },
    {
      "id": "focal_length_x",
      "name": "Focal length X",
      "description": "Facet focal length along X. Set both focal lengths to zero for flat facets.",
      "type": "real",
      "default": 0,
      "minimum": 0,
      "step": 1,
      "decimals": 3
    },
    {
      "id": "focal_length_y",
      "name": "Focal length Y",
      "description": "Facet focal length along Y. Set both focal lengths to zero for flat facets.",
      "type": "real",
      "default": 0,
      "minimum": 0,
      "step": 1,
      "decimals": 3
    },
    {
      "id": "canting_method",
      "name": "Canting method",
      "description": "How individual facet normals are oriented in the heliostat frame.",
      "type": "string",
      "default": "NONE",
      "choices": ["NONE", "ON_AXIS", "OFF_AXIS"]
    },
    {
      "id": "canting_distance",
      "name": "On-axis distance",
      "description": "Canting distance used by the ON_AXIS method.",
      "type": "real",
      "default": 100,
      "minimum": 0.001,
      "step": 1,
      "decimals": 3
    },
    {
      "id": "canting_sun_azimuth",
      "name": "Canting sun azimuth",
      "description": "Design-point solar azimuth used by OFF_AXIS canting.",
      "type": "real",
      "default": 180,
      "minimum": 0,
      "maximum": 360,
      "step": 1,
      "decimals": 2
    },
    {
      "id": "canting_sun_zenith",
      "name": "Canting sun zenith",
      "description": "Design-point solar zenith angle used by OFF_AXIS canting.",
      "type": "real",
      "default": 0,
      "minimum": 0,
      "maximum": 90,
      "step": 1,
      "decimals": 2
    },
    {
      "id": "reflectivity",
      "name": "Reflectivity",
      "description": "Front-face facet reflectivity from zero to one.",
      "type": "real",
      "default": 0.95,
      "minimum": 0,
      "maximum": 1,
      "step": 0.01,
      "decimals": 4
    },
    {
      "id": "slope_error",
      "name": "Slope error",
      "description": "Gaussian facet slope error.",
      "type": "real",
      "default": 0,
      "minimum": 0,
      "step": 0.01,
      "decimals": 4
    },
    {
      "id": "specularity_error",
      "name": "Specularity error",
      "description": "Gaussian facet specularity error.",
      "type": "real",
      "default": 0,
      "minimum": 0,
      "step": 0.01,
      "decimals": 4
    }
  ]
}*/

const EPSILON = 1.0e-10
const FACET_PREFIX = "facet:"

function clamp(value, minimum, maximum) {
    return Math.max(minimum, Math.min(maximum, value))
}

function add(a, b) {
    return db.vec3_add(a, b)
}

function subtract(a, b) {
    return db.vec3_sub(a, b)
}

function scale(value, factor) {
    return db.vec3_scale(value, factor)
}

function normalize(value, description) {
    if (db.vec3_length(value) < EPSILON) {
        throw new Error(description + " must have a non-zero direction")
    }
    return db.vec3_normalize(value)
}

function rotate(axis, value, radians) {
    return db.quat_rotate_vec3(
        db.quat_from_axis_angle(axis, radians * 180.0 / Math.PI),
        value
    )
}

function sunVector(azimuthDegrees, elevationDegrees) {
    const azimuth = azimuthDegrees * Math.PI / 180.0
    const elevation = elevationDegrees * Math.PI / 180.0
    return [
        Math.sin(azimuth) * Math.cos(elevation),
        Math.cos(azimuth) * Math.cos(elevation),
        Math.sin(elevation),
    ]
}

// Convert local x/y/z basis vectors, expressed in the parent frame, into the
// quaternion representation used by the GUI database.
function quaternionFromAxes(xAxis, yAxis, zAxis) {
    const m00 = xAxis[0]
    const m01 = yAxis[0]
    const m02 = zAxis[0]
    const m10 = xAxis[1]
    const m11 = yAxis[1]
    const m12 = zAxis[1]
    const m20 = xAxis[2]
    const m21 = yAxis[2]
    const m22 = zAxis[2]

    let w
    let x
    let y
    let z
    const trace = m00 + m11 + m22

    if (trace > 0.0) {
        const s = 2.0 * Math.sqrt(trace + 1.0)
        w = 0.25 * s
        x = (m21 - m12) / s
        y = (m02 - m20) / s
        z = (m10 - m01) / s
    } else if (m00 > m11 && m00 > m22) {
        const s = 2.0 * Math.sqrt(1.0 + m00 - m11 - m22)
        w = (m21 - m12) / s
        x = 0.25 * s
        y = (m01 + m10) / s
        z = (m02 + m20) / s
    } else if (m11 > m22) {
        const s = 2.0 * Math.sqrt(1.0 + m11 - m00 - m22)
        w = (m02 - m20) / s
        x = (m01 + m10) / s
        y = 0.25 * s
        z = (m12 + m21) / s
    } else {
        const s = 2.0 * Math.sqrt(1.0 + m22 - m00 - m11)
        w = (m10 - m01) / s
        x = (m02 + m20) / s
        y = (m12 + m21) / s
        z = 0.25 * s
    }

    return db.quat_normalize(db.quat(w, x, y, z))
}

// Match the Spencer/Murty direction and z-rotation convention used by the
// simulation element coordinate system.
function quaternionFromDirection(direction, gamma) {
    const aim = normalize(direction, "Aim vector")
    const alpha = Math.atan2(aim[0], aim[2])
    const beta = Math.asin(clamp(aim[1], -1.0, 1.0))
    const cosAlpha = Math.cos(alpha)
    const sinAlpha = Math.sin(alpha)
    const cosBeta = Math.cos(beta)
    const sinBeta = Math.sin(beta)
    const cosGamma = Math.cos(gamma)
    const sinGamma = Math.sin(gamma)

    const xAxis = [
        cosAlpha * cosGamma + sinAlpha * sinBeta * sinGamma,
        -cosBeta * sinGamma,
        -sinAlpha * cosGamma + cosAlpha * sinBeta * sinGamma,
    ]
    const yAxis = [
        cosAlpha * sinGamma - sinAlpha * sinBeta * cosGamma,
        cosBeta * cosGamma,
        -sinAlpha * sinGamma - cosAlpha * sinBeta * cosGamma,
    ]

    return quaternionFromAxes(xAxis, yAxis, aim)
}

function trackingRotation(origin, target, azimuth, elevation) {
    const targetDirection = normalize(
        subtract(target, origin),
        "Target position relative to the heliostat"
    )
    const aim = normalize(
        add(targetDirection, sunVector(azimuth, elevation)),
        "Tracking bisector"
    )

    const aimAzimuth = Math.atan2(aim[0], aim[1])
    const elevationAxis = rotate(
        [0.0, 0.0, 1.0],
        [1.0, 0.0, 0.0],
        -aimAzimuth
    )
    const heliostatYAxis = normalize(
        db.vec3_cross(aim, elevationAxis),
        "Heliostat y axis"
    )

    let gamma = 0.0
    if (Math.abs(Math.abs(aim[1]) - 1.0) > EPSILON) {
        gamma = -Math.atan2(elevationAxis[1], heliostatYAxis[1])
    }
    return quaternionFromDirection(aim, gamma)
}

function ensureSingle(scope, name, factory) {
    const reference = scope.single(name)
    let entity = reference.get()
    if (!db.valid(entity)) {
        entity = factory()
        if (!reference.set(entity)) {
            throw new Error("Could not store scene reference " + name)
        }
    }
    return entity
}

scope_id = String(scope_id).trim()
if (scope_id.length === 0) {
    throw new Error("Scope ID cannot be empty")
}

const canting = String(canting_method).trim().toUpperCase()
if (canting !== "NONE" && canting !== "ON_AXIS" && canting !== "OFF_AXIS") {
    throw new Error("Canting method must be NONE, ON_AXIS, or OFF_AXIS")
}
if ((focal_length_x === 0.0) !== (focal_length_y === 0.0)) {
    throw new Error(
        "Focal lengths must either both be zero (flat) or both be positive"
    )
}

const panelWidth = (
    aperture_width - gap_x * (panels_x - 1)
) / panels_x
const panelHeight = (
    aperture_height - gap_y * (panels_y - 1)
) / panels_y
if (panelWidth <= 0.0 || panelHeight <= 0.0) {
    throw new Error("Panel gaps leave no positive facet area")
}

const field = refs.scope(scope_id)
const root = ensureSingle(field, "root", () => db.create())
const material = ensureSingle(field, "material", () => db.create_material())
const geometry = ensureSingle(field, "geometry", () => db.create_geometry())
const facets = field.group("facets")

db.set_identity(root, heliostat_name)
db.set_identity(material, heliostat_name + " Mirror Material")
db.set_identity(geometry, heliostat_name + " Facet Geometry")

const mirrorFront = {
    my_type: "REFLECTION",
    error_distribution_type: "GAUSSIAN",
    transmissivity: 0.0,
    reflectivity: reflectivity,
    slope_error: slope_error,
    specularity_error: specularity_error,
    refraction_index_front: 1.0,
    refraction_index_back: 1.0,
}
const mirrorBack = {
    my_type: "REFLECTION",
    error_distribution_type: "GAUSSIAN",
    transmissivity: 0.0,
    reflectivity: 0.0,
    slope_error: 0.0,
    specularity_error: 0.0,
    refraction_index_front: 1.0,
    refraction_index_back: 1.0,
}
db.set_material_properties(material, {
    front: mirrorFront,
    back: mirrorBack,
})

const surface = focal_length_x === 0.0
    ? { surface_type: "FLAT" }
    : {
        surface_type: "PARABOLA",
        focal_length_x: focal_length_x,
        focal_length_y: focal_length_y,
    }
db.set_geometry_properties(geometry, {
    aperture: {
        aperture_type: "RECTANGLE",
        x_length: panelWidth,
        y_length: panelHeight,
        x_coord: -0.5 * panelWidth,
        y_coord: -0.5 * panelHeight,
    },
    surface: surface,
})

db.set_transform(root, {
    position: position,
    rotation: trackingRotation(
        position,
        target_position,
        sun_azimuth,
        sun_elevation
    ),
})

const totalPanels = panels_x * panels_y

// Remove only facets previously assigned a numbered role by this template.
// Other scripts may intentionally place additional entities in the group.
for (const name of field.single_names()) {
    if (name.indexOf(FACET_PREFIX) !== 0) {
        continue
    }
    const index = Number(name.substring(FACET_PREFIX.length))
    if (!Number.isInteger(index) || index < totalPanels) {
        continue
    }

    const reference = field.single(name)
    const facet = reference.get()
    reference.clear()
    if (db.valid(facet)) {
        facets.remove(facet)
        db.destroy(facet)
    }
}

let designSun = null
let designDeltaAzimuth = 0.0
let designDeltaElevation = 0.0
let designElevationAxis = [1.0, 0.0, 0.0]

if (canting === "OFF_AXIS") {
    designSun = sunVector(
        canting_sun_azimuth,
        90.0 - canting_sun_zenith
    )
    const targetDirection = normalize(
        subtract(target_position, position),
        "Target position relative to the heliostat"
    )
    const designAim = normalize(
        add(targetDirection, designSun),
        "Off-axis canting bisector"
    )
    const trackingAzimuth = Math.atan2(designAim[0], designAim[1])
    const trackingElevation = Math.asin(clamp(designAim[2], -1.0, 1.0))
    designDeltaAzimuth = trackingAzimuth - Math.PI
    designDeltaElevation = trackingElevation - Math.PI / 2.0
    designElevationAxis = rotate(
        [0.0, 0.0, 1.0],
        [1.0, 0.0, 0.0],
        -designDeltaAzimuth
    )
}

let panelY = -0.5 * (aperture_height - panelHeight)
for (let row = 0; row < panels_y; ++row) {
    let panelX = -0.5 * (aperture_width - panelWidth)

    for (let column = 0; column < panels_x; ++column) {
        const index = row * panels_x + column
        const role = FACET_PREFIX + String(index)
        const facet = ensureSingle(field, role, () => db.create())
        let localPosition
        let localAim

        if (canting === "NONE") {
            localPosition = [panelX, panelY, 0.0]
            localAim = [panelX, panelY, 1000.0]
        } else if (canting === "ON_AXIS") {
            const curvature = 0.5 / canting_distance
            const height = 0.5 * curvature * (
                panelX * panelX + panelY * panelY
            )
            localPosition = [panelX, panelY, height]
            localAim = [0.0, 0.0, 2.0 * canting_distance]
        } else {
            localPosition = [panelX, panelY, 0.0]

            let panelGlobal = rotate(
                [0.0, 0.0, 1.0],
                localPosition,
                -designDeltaAzimuth
            )
            panelGlobal = rotate(
                designElevationAxis,
                panelGlobal,
                -designDeltaElevation
            )
            panelGlobal = add(position, panelGlobal)

            const targetDirection = normalize(
                subtract(target_position, panelGlobal),
                "Target position relative to facet " + String(index + 1)
            )
            let panelNormal = normalize(
                add(targetDirection, designSun),
                "Off-axis facet bisector"
            )
            panelNormal = rotate(
                designElevationAxis,
                panelNormal,
                designDeltaElevation
            )
            panelNormal = rotate(
                [0.0, 0.0, 1.0],
                panelNormal,
                designDeltaAzimuth
            )

            const scaleToTarget = 2.0 * db.vec3_distance(
                target_position,
                panelGlobal
            )
            localAim = add(
                localPosition,
                scale(panelNormal, scaleToTarget)
            )
        }

        db.set_identity(
            facet,
            heliostat_name + " Facet " + String(index + 1)
        )
        db.set_material_of(facet, material)
        db.set_geometry_of(facet, geometry)
        if (!db.set_parent(facet, root)) {
            throw new Error("Could not parent facet " + String(index + 1))
        }
        db.set_transform(facet, {
            position: localPosition,
            rotation: quaternionFromDirection(
                subtract(localAim, localPosition),
                0.0
            ),
        })
        facets.add(facet)

        panelX += panelWidth + gap_x
    }
    panelY += panelHeight + gap_y
}

console.log(
    "Updated",
    heliostat_name,
    "in scope",
    scope_id,
    "with",
    totalPanels,
    "facets"
)

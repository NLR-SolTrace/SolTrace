---
title: "Scripting"
---

Scripts can automate SolTrace workflows, expose parameters, and report output in the panel below. Load a built-in script or open a local script file, edit it if needed, then run it from the selected working directory.

New scripts should begin with a JSON object embedded in a block comment. The manifest supplies the script title, description, and an ordered `parameters` array. Each parameter declares an `id`, `type`, and `default`, with optional display `name`, `description`, bounds, step size, decimal precision, string choices, or vector unit. Supported types are `integer`, `real`, `string`, `vec3`/`vector3d`, and `position`. A `position` is passed to JavaScript as the same three-number array as a vector, while also offering a non-modal numeric editor and a translation widget in the 3D viewport. The older TITLE/DESC/PROPERTY header remains supported for existing scripts.

Use Script API when you need the available commands and objects exposed to scripts.

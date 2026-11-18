/**
 * @file json_helpers.hpp
 * @brief Lightweight helpers for extracting values from JSON nodes.
 *
 * Defines utility functions used to read specific typed values from
 * nlohmann::ordered_json objects while providing simple fallback behavior.
 * Intended for internal parsing of simulation configuration structures
 * (not full file serialization).
 *
 * Add additional typed accessors here as needed.
 */

#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include <optional>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include <glm/vec3.hpp>

#include "element_id.hpp"

namespace SolTrace::Data {

    const int kJsonIndentSpaces = 4;

    /// @brief Read the element ID saved in an element's JSON node.
    /// @return The ID, or nullopt if `my_id` is missing or null, which means
    ///         the element gets a new ID.
    /// @throws std::invalid_argument if `my_id` is not an integer of 1 or more.
    inline std::optional<element_id>
    json_get_saved_element_id(const nlohmann::ordered_json& jnode)
    {
        auto it = jnode.find("my_id");
        if (it == jnode.end() || it->is_null())
        {
            return std::nullopt;
        }
        if (!it->is_number_integer() || it->get<element_id>() < 1)
        {
            throw std::invalid_argument(
                "Element my_id must be an integer of 1 or more, got " +
                it->dump());
        }
        return it->get<element_id>();
    }

    inline double json_get_double(const nlohmann::ordered_json& jnode, std::string key)
    {
        auto jval = jnode.at(key);
        if (jval.is_null())
            return std::numeric_limits<double>::quiet_NaN();
        else
            return jval;
    }

    inline
    std::array<double, 3> to_array(glm::dvec3 v)
    {
        return {v.x, v.y, v.z};
    }

    inline
    glm::dvec3 from_array(std::array<double, 3> array)
    {
        return {array[0], array[1], array[2]};
    }


} // namespace SolTrace::Data

#endif


#include "element.hpp"

#include <math.h>

#include "constants.hpp"
#include "json_helpers.hpp"
#include "matvec.hpp"

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

namespace SolTrace::Data {

// ElementContainer ElementBase::empty_container;

ElementBase::ElementBase(ElementKey key) : Element(),
                                           active(true),
                                           virtual_flag(false),
                                           my_id(key.id()),
                                           model_context(key.context()),
                                           model_token(key.token()),
                                           coordinates_initialized(true),
                                           stage(-1),
                                           zrot(0.0),
                                           reference_element(nullptr)
{
    // Default local coordinates to match with the reference coordinates
    this->aim = {0.0, 0.0, 1.0};
    this->origin = glm::dvec3(0.0);

    this->euler_angles = glm::dvec3(0.0);
    this->reference_to_local = glm::identity<glm::dmat3>();
    this->local_to_reference = glm::identity<glm::dmat3>();

    return;
}

ElementBase::ElementBase(ElementKey key, const nlohmann::ordered_json& jnode)
    : ElementBase(key)
{
    if (jnode.at("active").get<bool>())
        this->enable();
    else
        this->disable();

    if (jnode.at("virtual_flag").get<bool>())
        this->mark_virtual();
    else
        this->unmark_virtual();

    this->set_stage(jnode.at("stage"));
    this->set_name(jnode.at("my_name"));

    std::array<double, 3> orig_arr = jnode.at("origin").get<std::array<double, 3>>();
    glm::dvec3 orig_vec = from_array(orig_arr);
    this->set_origin(orig_vec);

    std::array<double, 3> aim_arr = jnode.at("aim").get<std::array<double, 3>>();
    glm::dvec3 aim_vec = from_array(aim_arr);
    this->set_aim_vector(aim_vec);

    this->set_zrot(jnode.at("zrot"));

    this->coordinates_initialized = false;
    //this->compute_coordinate_rotations();
}

ElementBase::~ElementBase()
{
    this->reference_element = nullptr;
    return;
}

// ElementContainer::iterator ElementBase::get_iterator()
// {
//     return empty_container.get_iterator();
// }

// ElementContainer::const_iterator ElementBase::get_const_iterator()
// {
//     return empty_container.get_const_iterator();
// }

glm::dvec3 ElementBase::get_origin_stage() const
{
    glm::dvec3 origin_stage;
    auto ref_el = this->reference_element;
    if (this->is_stage())
    {
        // This is the stage element so the stage origin is zero.
        origin_stage = glm::dvec3(0.0);
    }
    else if (ref_el == nullptr)
    {
        // We hit the end of the chain without finding a stage element.
        // Assume we are not using stages and return the global origin.
        origin_stage = this->origin;
    }
    else
    {
        ref_el->convert_local_to_stage(origin_stage, this->origin);
    }
    return origin_stage;
}

glm::dvec3 ElementBase::get_origin_global() const
{
    glm::dvec3 origin_global;
    auto ref_el = this->reference_element;
    if (ref_el == nullptr)
    {
        origin_global = this->origin;
    }
    else
    {
        ref_el->convert_local_to_global(origin_global, this->origin);
    }
    return origin_global;
}

glm::dvec3 ElementBase::get_aim_vector_stage() const
{
    glm::dvec3 aim_stage;
    if (this->reference_element == nullptr)
    {
        aim_stage = this->aim;
    }
    else
    {
        this->reference_element->convert_local_to_stage(
            aim_stage, this->aim);
    }
    return aim_stage;
}

glm::dvec3 ElementBase::get_aim_vector_global() const
{
    glm::dvec3 aim_global;
    if (this->reference_element == nullptr)
    {
        aim_global = this->aim;
    }
    else
    {
        this->reference_element->convert_local_to_global(
            aim_global, this->aim);
    }
    return aim_global;
}

glm::dmat3 ElementBase::get_reference_to_local() const
{
    return this->reference_to_local;
}

glm::dmat3 ElementBase::get_stage_to_local() const
{
    glm::dmat3 stage_to_local = glm::dmat3(0.0);
    if (this->is_stage())
    {
        // stage_to_local.set_value(0, 0, 1.0);
        // stage_to_local.set_value(1, 1, 1.0);
        // stage_to_local.set_value(2, 2, 1.0);
        stage_to_local = glm::identity<glm::dmat3>();
    }
    else if (this->reference_element == nullptr)
    {
        stage_to_local = this->reference_to_local;
    }
    else
    {
        glm::dmat3 R = this->reference_element->get_stage_to_local();
        stage_to_local = this->reference_to_local * R;
    }
    return stage_to_local;
}

glm::dmat3 ElementBase::get_global_to_local() const
{
    glm::dmat3 global_to_local = glm::dmat3(0.0);
    if (this->reference_element == nullptr) {
        global_to_local = this->reference_to_local;
    } else {
        glm::dmat3 R = this->reference_element->get_global_to_local();
        global_to_local = this->reference_to_local * R;
    }
    return global_to_local;
}

glm::dmat3 ElementBase::get_local_to_reference() const
{
    return this->local_to_reference;
}

glm::dmat3 ElementBase::get_local_to_stage() const
{
    glm::dmat3 local_to_stage = glm::dmat3(0.0);
    if (this->is_stage())
    {
        local_to_stage = glm::identity<glm::dmat3>();
    }
    else if (this->reference_element == nullptr)
    {
        local_to_stage = this->local_to_reference;
    }
    else
    {
        glm::dmat3 R = this->reference_element->get_local_to_stage();
        local_to_stage = R * this->local_to_reference;
    }
    return local_to_stage;
}

glm::dmat3 ElementBase::get_local_to_global() const
{
    glm::dmat3 local_to_global = glm::dmat3(0.0);
    if (this->reference_element == nullptr)
    {
        local_to_global = this->local_to_reference;
    }
    else
    {
        glm::dmat3 R = this->reference_element->get_local_to_global();
        local_to_global = R * this->local_to_reference;
    }
    return local_to_global;
}

int ElementBase::compute_coordinate_rotations()
{
    int sts = 0;

    if (!this->coordinates_initialized)
    {
        glm::dvec3 dr = glm::normalize(this->aim - this->origin);

        this->euler_angles[0] = atan2(dr[0], dr[2]);
        this->euler_angles[1] = asin(dr[1]);
        this->euler_angles[2] = this->zrot * D2R;

        CalculateTransformMatrices(this->euler_angles,
                                   this->reference_to_local,
                                   this->local_to_reference);

        this->coordinates_initialized = true;
    }

    return sts;
}

int ElementBase::set_reference_frame_geometry(const glm::dvec3 &origin,
                                              const glm::dvec3 &aim,
                                              double zrot)
{
    this->coordinates_initialized = false;
    this->origin = origin;
    this->aim = aim;
    this->zrot = zrot;
    return this->compute_coordinate_rotations();
}

int ElementBase::convert_reference_to_local(glm::dvec3 &local,
                                            const glm::dvec3 &ref)
{
    glm::dvec3 temp = ref - this->origin;
    local = this->reference_to_local * temp;
    return 0;
}

int ElementBase::convert_stage_to_local(glm::dvec3 &local,
                                        const glm::dvec3 &stage)
{
    if (this->is_stage())
    {
        // We are in the stage coordinate frame so the local coordinates
        // are the stage coordinates
        local = stage;
    }
    else if (this->reference_element == nullptr)
    {
        // No stage frame found so assume we are not using stages
        // and return the same answer as convert_global_to_local
        this->convert_reference_to_local(local, stage);
    }
    else
    {
        glm::dvec3 ref;
        this->reference_element->convert_stage_to_local(ref, stage);
        convert_reference_to_local(local, ref);
    }
    return 0;
}

int ElementBase::convert_global_to_local(glm::dvec3 &local,
                                         const glm::dvec3 &global)
{
    auto ref_el = this->reference_element;
    if (ref_el == nullptr)
    {
        // We are at the global coordinate frame
        this->convert_reference_to_local(local, global);
    }
    else
    {
        glm::dvec3 ref;
        ref_el->convert_global_to_local(ref, global);
        this->convert_reference_to_local(local, ref);
    }
    return 0;
}

int ElementBase::convert_local_to_reference(glm::dvec3 &ref,
                                            const glm::dvec3 &local)
{
    glm::dvec3 temp = this->local_to_reference * local;
    ref = temp + this->origin;
    return 0;
}

int ElementBase::convert_local_to_stage(glm::dvec3 &stage,
                                        const glm::dvec3 &local)
{
    if (this->is_stage())
    {
        // We are in the stage coordinate frame so the local coordinates
        // are the stage coordinates
        stage = local;
    }
    else if (this->reference_element == nullptr)
    {
        // No stage has been found so assume we are not using stages
        // and convert to global coordinates
        this->convert_local_to_reference(stage, local);
        // TODO: This should probably return something other than 0...
    }
    else
    {
        glm::dvec3 ref;
        this->convert_local_to_reference(ref, local);
        this->reference_element->convert_local_to_stage(stage, ref);
    }
    return 0;
}

int ElementBase::convert_local_to_global(glm::dvec3 &global,
                                         const glm::dvec3 &local)
{
    auto ref_el = this->reference_element;
    if (ref_el == nullptr)
    {
        this->convert_local_to_reference(global, local);
    }
    else
    {
        glm::dvec3 ref;
        this->convert_local_to_reference(ref, local);
        ref_el->convert_local_to_global(global, ref);
    }
    return 0;
}

int ElementBase::convert_global_to_reference(glm::dvec3 &ref,
                                             const glm::dvec3 &global)
{
    if (this->reference_element == nullptr)
    {
        ref = global;
        return 0;
    }
    else
    {
        return this->reference_element->convert_global_to_local(ref, global);
    }
}

int ElementBase::convert_reference_to_global(glm::dvec3 &global,
                                             const glm::dvec3 &ref)
{
    if (this->reference_element == nullptr)
    {
        global = ref;
        return 0;
    }
    else
    {
        return this->reference_element->convert_local_to_global(global, ref);
    }
}

int ElementBase::convert_vector_reference_to_local(glm::dvec3 &local,
                                                   const glm::dvec3 &ref)
{
    local = this->reference_to_local * ref;
    return 0;
}

int ElementBase::convert_vector_stage_to_local(glm::dvec3 &local,
                                               const glm::dvec3 &stage)
{
    if (this->is_stage())
    {
        // We are in the stage coordinate frame so the local coordinates
        // are the stage coordinates
        local = stage;
    }
    else if (this->reference_element == nullptr)
    {
        // No stage frame found so assume we are not using stages
        // and return the same answer as convert_global_to_local
        this->convert_vector_reference_to_local(local, stage);
    }
    else
    {
        glm::dvec3 ref;
        this->reference_element->convert_vector_stage_to_local(ref, stage);
        convert_vector_reference_to_local(local, ref);
    }
    return 0;
}

int ElementBase::convert_vector_global_to_local(glm::dvec3 &local,
                                                const glm::dvec3 &global)
{
    auto ref_el = this->reference_element;
    if (ref_el == nullptr)
    {
        // We are at the global coordinate frame
        this->convert_vector_reference_to_local(local, global);
    }
    else
    {
        glm::dvec3 ref;
        ref_el->convert_vector_global_to_local(ref, global);
        this->convert_vector_reference_to_local(local, ref);
    }
    return 0;
}

int ElementBase::convert_vector_local_to_reference(glm::dvec3 &ref,
                                                   const glm::dvec3 &local)
{
    ref = this->local_to_reference * local;
    return 0;
}

int ElementBase::convert_vector_local_to_stage(glm::dvec3 &stage,
                                               const glm::dvec3 &local)
{
    if (this->is_stage())
    {
        // We are in the stage coordinate frame so the local coordinates
        // are the stage coordinates
        stage = local;
    }
    else if (this->reference_element == nullptr)
    {
        // No stage has been found so assume we are not using stages
        // and convert to global coordinates
        this->convert_vector_local_to_reference(stage, local);
        // TODO: This should probably return something other than 0...
    }
    else
    {
        glm::dvec3 ref;
        this->convert_vector_local_to_reference(ref, local);
        this->reference_element->convert_vector_local_to_stage(stage, ref);
    }
    return 0;
}

int ElementBase::convert_vector_local_to_global(glm::dvec3 &global,
                                                const glm::dvec3 &local)
{
    auto ref_el = this->reference_element;
    if (ref_el == nullptr)
    {
        this->convert_vector_local_to_reference(global, local);
    }
    else
    {
        glm::dvec3 ref;
        this->convert_vector_local_to_reference(ref, local);
        ref_el->convert_vector_local_to_global(global, ref);
    }
    return 0;
}

int ElementBase::convert_vector_global_to_reference(glm::dvec3 &ref,
                                                    const glm::dvec3 &global)
{
    if (this->reference_element == nullptr)
    {
        ref = global;
        return 0;
    }
    else
    {
        return this->reference_element->convert_vector_global_to_local(
            ref, global);
    }
}

int ElementBase::convert_vector_reference_to_global(glm::dvec3 &global,
                                                    const glm::dvec3 &ref)
{
    if (this->reference_element == nullptr)
    {
        global = ref;
        return 0;
    }
    else
    {
        return this->reference_element->convert_vector_local_to_global(
            global, ref);
    }
}

// const OpticalProperties & ElementBase::get_optical_properties() const
// {
//     return this->optics;
// }

// void ElementBase::set_optical_properties(const OpticalProperties &op)
// {
//     this->optics = op;
// }

void ElementBase::enforce_user_fields_set() const
{
    return;
}

void ElementBase::write_common_json(nlohmann::ordered_json& jnode) const
{
    jnode["active"] = this->active;
    jnode["virtual_flag"] = this->virtual_flag;
    jnode["my_id"] = this->my_id;
    jnode["stage"] = this->stage;
    jnode["my_name"] = this->my_name;

    jnode["origin"] = to_array(this->origin);
    jnode["aim"] = to_array(this->aim);
    jnode["zrot"] = this->zrot;
    
    // Not including calculated values
    //jnode["euler_angles"] = this->euler_angles.data;
}

} // namespace SolTrace::Data

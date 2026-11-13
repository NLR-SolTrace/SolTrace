#include "exception_guard.hpp"
#include "utils.hpp"

//////////////////////////////////////
// functions to add/remove elements //
//////////////////////////////////////

STAPI_V2 st_return_t st_num_elements(st_context_v2_t pcxt, uint_fast64_t *num_elements) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_num_elements", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    *num_elements = data->get_number_of_elements();
    return st_return_code::SUCCESS;
});
}

int32_t num_aperture_params(ApertureType type)
{
    int32_t rt = -1;
    
    switch (type)
    {
        case ApertureType::CIRCLE:
        case ApertureType::EQUILATERAL_TRIANGLE:
        case ApertureType::HEXAGON:
        {
            rt = 1;
            break;
        }
        case ApertureType::RECTANGLE:
        {
            rt = 2;
            break;
        }
        case ApertureType::ANNULUS:
        case ApertureType::SINGLE_AXIS_CURVATURE_SECTION:
        {
            rt = 3;
            break;
        }
        case ApertureType::IRREGULAR_TRIANGLE:
        {
            rt = 6;
            break;
        }
        case ApertureType::IRREGULAR_QUADRILATERAL:
        {
            rt = 8;
            break;
        }
        default:
            rt = -1;
            break;
        }
    return rt;
}

int32_t num_surface_params(SurfaceType type)
{
    int32_t rt = -1;

    switch (type)
    {
        case SurfaceType::FLAT:
        {
            rt = 0;
            break;
        }
        case SurfaceType::CONE:
        case SurfaceType::CYLINDER:
        case SurfaceType::SPHERE:
        {
            rt = 1;
            break;
        }
        case SurfaceType::PARABOLA:
        {
            rt = 2;
            break;
        }
        case SurfaceType::HYPER:
        case SurfaceType::GENERAL_SPENCER_MURTY:
        case SurfaceType::TORUS:
        default:
            rt = -1; // Not implemented yet
            break;
    }

    return rt;
}

st_return_t create_geometry(st_context    *cxt,
                            args_geometry *geom,
                            aperture_ptr  &ap,
                            surface_ptr   &surf)
{
    // checking geometry
    ApertureType a_type = char_to_aperture(geom->ap);
    SurfaceType s_type = char_to_surface(geom->surf);
    int32_t num_a_params = num_aperture_params(a_type);
    int32_t num_s_params = num_surface_params(s_type);
    if (num_a_params < 0 || num_s_params < 0) return st_return_code::INVALID_ARGUMENTS;

    std::vector<double> _a_params(std::begin(geom->a_params), std::end(geom->a_params));
    std::vector<double> _s_params(std::begin(geom->s_params), std::end(geom->s_params));

    try
    {
        ap = Aperture::make_aperture_from_type(a_type, _a_params);
        surf = SolTrace::Data::make_surface_from_type(s_type, _s_params);
    }
    catch (const std::invalid_argument& e)
    {
        if (cxt->p_cb) cxt->p_cb("geometry creation", e.what());
        return st_return_code::INVALID_ARGUMENTS;
    }

    return st_return_code::SUCCESS;
}

STAPI_V2 st_return_t st_add_element(st_context_v2_t pcxt,
									args_element    *args,
									int_fast64_t    opt_id,
									args_geometry   *geom,
									uint_fast64_t   *element_id) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_add_element", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    // check that inputs are good
    // checking optics
    const OpticalPropertySetReference existing_set = st_utils::get_optics_ref_by_id(data, opt_id);
    if (existing_set.id == SolTrace::Data::OPTICS_ID_TYPES::OPTICS_ID_UNASSIGNED)
    return st_return_code::DATA_VALUE_NOT_FOUND;
    
    aperture_ptr ap;
    surface_ptr surf;
    st_return_t code = create_geometry(cxt, geom, ap, surf);

    if (code > st_return_code::SUCCESS) return code;

    single_element_ptr el = make_element<SingleElement>();
    
    el->set_origin(args->x, args->y, args->z);
    el->set_aim_vector(args->ax, args->ay, args->az);
    el->set_zrot(args->zrot);
    if (args->enabled_flag) el->enable();
    else                    el->disable();
    if (args->virtual_flag) el->mark_virtual();
    else                    el->unmark_virtual();
    
    el->set_optical_property_set(existing_set);
    el->set_aperture(ap);
    el->set_surface(surf);

    data->add_element(el);
    
    *element_id = el->get_id();
    
    // to maintain grouping, changing groups goes thru SimulationData, so set group after adding the element
    if (args->group < -1) code = st_return_code::WARNING_GROUP_IGNORED;
    else                  data->set_element_group(*element_id, args->group);
    
    return code;
});
}

STAPI_V2 st_return_t st_get_element(st_context_v2_t pcxt,
									uint_fast64_t   idx,
									args_element    *args,
									int_fast64_t    *opt_id,
									args_geometry   *geom) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_element", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::DATA_VALUE_NOT_FOUND;

    // get element
    args->enabled_flag = el->is_enabled();
    args->virtual_flag = el->is_virtual();
    glm::dvec3 pos = el->get_origin_global();
    args->x = pos[0];
    args->y = pos[1];
    args->z = pos[2];
    glm::dvec3 aim = el->get_aim_vector_global();
    args->ax = aim[0];
    args->ay = aim[1];
    args->az = aim[2];
    args->zrot = el->get_zrot();
    args->group = el->get_group();
    *opt_id = el->get_optical_property_set_id();
    
    // get geometry
    aperture_ptr ap_ptr = el->get_aperture();
    ApertureType ap_type = ap_ptr->get_type();
    geom->ap = aperture_to_char(ap_type);
    surface_ptr surf_ptr = el->get_surface();
    SurfaceType surf_type = surf_ptr->get_type();
    geom->surf = surface_to_char(surf_type);
    // reset params
    for (size_t i = 0; i < 8; ++i)
    {
        geom->a_params[i] = 0;
        geom->s_params[i] = 0;
    }
    
    switch (ap_type)
    {
        case ApertureType::ANNULUS:
        {
            auto temp = std::dynamic_pointer_cast<Annulus>(ap_ptr);
            geom->a_params[0] = temp->inner_radius;
            geom->a_params[1] = temp->outer_radius;
            geom->a_params[2] = temp->arc_angle;
            break;
        }
        case ApertureType::CIRCLE:
        {
            auto temp = std::dynamic_pointer_cast<Circle>(ap_ptr);
            geom->a_params[0] = temp->diameter;
            break;
        }
        case ApertureType::HEXAGON:
        {
            auto temp = std::dynamic_pointer_cast<Hexagon>(ap_ptr);
            geom->a_params[0] = temp->circumscribe_diameter;
            break;
        }
        case ApertureType::RECTANGLE:
        {
            auto temp = std::dynamic_pointer_cast<Rectangle>(ap_ptr);
            geom->a_params[0] = temp->x_length();
            geom->a_params[1] = temp->y_length();
            break;
        }
        case ApertureType::EQUILATERAL_TRIANGLE:
        {
            auto temp = std::dynamic_pointer_cast<EquilateralTriangle>(ap_ptr);
            geom->a_params[0] = temp->circumscribe_diameter;
            break;
        }
        
        // case ApertureType::SINGLE_AXIS_CURVATURE_SECTION:

        case ApertureType::IRREGULAR_TRIANGLE:
        {
            auto temp = std::dynamic_pointer_cast<IrregularTriangle>(ap_ptr);
            geom->a_params[0] = temp->x1;
            geom->a_params[1] = temp->y1;
            geom->a_params[2] = temp->x2;
            geom->a_params[3] = temp->y2;
            geom->a_params[4] = temp->x3;
            geom->a_params[5] = temp->y3;
            break;
        }
        case ApertureType::IRREGULAR_QUADRILATERAL:
        {
            auto temp = std::dynamic_pointer_cast<IrregularQuadrilateral>(ap_ptr);
            geom->a_params[0] = temp->x1;
            geom->a_params[1] = temp->y1;
            geom->a_params[2] = temp->x2;
            geom->a_params[3] = temp->y2;
            geom->a_params[4] = temp->x3;
            geom->a_params[5] = temp->y3;
            geom->a_params[6] = temp->x4;
            geom->a_params[7] = temp->y4;
            break;
        }
        default:
            break;
    }

    switch (surf_type)
    {
        case SurfaceType::CONE:
        {
            auto temp = std::dynamic_pointer_cast<Cone>(surf_ptr);
            geom->s_params[0] = temp->half_angle;
            break;
        }
        case SurfaceType::CYLINDER:
        {
            auto temp = std::dynamic_pointer_cast<Cylinder>(surf_ptr);
            geom->s_params[0] = temp->radius;
            break;
        }
        case SurfaceType::FLAT:
        {
            break;
        }
        case SurfaceType::PARABOLA:
        {
            auto temp = std::dynamic_pointer_cast<Parabola>(surf_ptr);
            geom->s_params[0] = temp->focal_length_x;
            geom->s_params[1] = temp->focal_length_y;
            break;
        }
        case SurfaceType::SPHERE:
        {
            auto temp = std::dynamic_pointer_cast<Sphere>(surf_ptr);
            geom->s_params[0] = temp->vertex_curv;
            break;
        }

        // case SurfaceType::HYPER:
        // case SurfaceType::GENERAL_SPENCER_MURTY:
        // case SurfaceType::TORUS:

        default:
            break;
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_element_area(st_context_v2_t pcxt,
                                         uint_fast64_t idx,
                                         double *area) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_element_area", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    *area = el->get_aperture()->aperture_area();
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_delete_element(st_context_v2_t pcxt, st_uint_t idx) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_delete_element", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    uint_fast64_t removed = data->remove_element(idx);

    if (removed == 0) return st_return_code::WARNING_NOT_FOUND;
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_clear_elements(st_context_v2_t pcxt) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_clear_elements", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    data->clear_elements();
    return st_return_code::SUCCESS;
});
}

//////////////////////////////////
// functions to modify elements //
//////////////////////////////////

STAPI_V2 st_return_t st_element_enabled(st_context_v2_t pcxt,
										st_uint_t 		idx,
										bool 			enabled_flag) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_enabled", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    if (enabled_flag) el->enable();
    else              el->disable();

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_virtual(st_context_v2_t pcxt,
										st_uint_t 		idx,
										bool 			virtual_flag) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_virtual", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    if (virtual_flag) el->mark_virtual();
    else              el->unmark_virtual();

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_xyz(st_context_v2_t pcxt, 
									st_uint_t 		idx,
									double 	  		x,
									double 	  		y,
									double 	  		z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_xyz", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    el->set_origin(x, y, z);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_aim(st_context_v2_t pcxt, 
									st_uint_t 		idx,
									double 	  		ax,
									double 	  		ay,
									double 	  		az) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_aim", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    el->set_aim_vector(ax, ay, az);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_zrot(st_context_v2_t pcxt,
									 st_uint_t 		 idx,
									 double 		 zrot) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_zrot", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    el->set_zrot(zrot);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_aperture(st_context_v2_t pcxt,
                                         st_uint_t 	     idx,
                                         char      	     ap,
                                         double    	     params[8]) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_aperture", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;
    
    ApertureType ap_type = char_to_aperture(ap);
    int32_t num_params = num_aperture_params(ap_type);
    if (num_params < 0) return st_return_code::INVALID_ARGUMENTS;

    std::vector<double> _params(params, params + num_params);
    aperture_ptr a;
    try
    {
        a = Aperture::make_aperture_from_type(ap_type, _params);
    }
    catch (const std::invalid_argument& e)
    {
        if (cxt->p_cb) cxt->p_cb("geometry creation", e.what());
        return st_return_code::INVALID_ARGUMENTS;
    }

    el->set_aperture(a);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_surface(st_context_v2_t pcxt,
                                        st_uint_t 	    idx,
                                        char      	    surf,
                                        double    	    params[8]) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_surface", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;
    
    SurfaceType surf_type = char_to_surface(surf);
    int32_t num_params = num_surface_params(surf_type);
    if (num_params < 0) return st_return_code::INVALID_ARGUMENTS;

    std::vector<double> _params(params, params + num_params);
    surface_ptr s;
    try
    {
        s = SolTrace::Data::make_surface_from_type(surf_type, _params);
    }
    catch (const std::invalid_argument& e)
    {
        if (cxt->p_cb) cxt->p_cb("geometry creation", e.what());
        return st_return_code::INVALID_ARGUMENTS;
    }

    el->set_surface(s);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_geometry(st_context_v2_t pcxt,
                                         st_uint_t 	     idx,
                                         args_geometry	 *geom) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_geometry", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    aperture_ptr ap;
    surface_ptr surf;
    st_return_t code = create_geometry(cxt, geom, ap, surf);

    if (code > st_return_code::SUCCESS) return code;
    
    el->set_aperture(ap);
    el->set_surface(surf);
    
    return code;
});
}

STAPI_V2 st_return_t st_get_aperture_area(st_context_v2_t pcxt,
                                          args_geometry   *geom,
                                          double          *area) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_aperture_area", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    aperture_ptr ap;
    surface_ptr surf;
    st_return_t code = create_geometry(cxt, geom, ap, surf);

    if (code > st_return_code::SUCCESS) return code;

    *area = ap->aperture_area();
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_optic(st_context_v2_t pcxt,
									  st_uint_t 	  idx,
									  int_fast64_t 	  opt_id) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_optic", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    const OpticalPropertySetReference existing_set = st_utils::get_optics_ref_by_id(data, opt_id);
    if (existing_set.id == SolTrace::Data::OPTICS_ID_TYPES::OPTICS_ID_UNASSIGNED)
        return st_return_code::DATA_VALUE_NOT_FOUND;

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    el->set_optical_property_set(existing_set);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_element_group(st_context_v2_t pcxt,
									  st_uint_t 	  idx,
									  int32_t   	  group) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_element_group", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    if (group < -1) return st_return_code::WARNING_GROUP_IGNORED;

    element_ptr el = data->get_element(idx);
    if (!el) return st_return_code::WARNING_NOT_FOUND;

    data->set_element_group(idx, group);

    return st_return_code::SUCCESS;
});
}
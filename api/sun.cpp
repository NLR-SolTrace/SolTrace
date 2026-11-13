#include "exception_guard.hpp"
#include "utils.hpp"

///////////////////
// sun functions //
///////////////////

std::shared_ptr<Sun> get_or_create_sun(SimulationData *data)
{
    if (!data->get_number_of_ray_sources())
        data->add_ray_source(make_ray_source<Sun>());
    
    ray_source_ptr sun_ptr = data->get_ray_source(0);
    return std::dynamic_pointer_cast<Sun>(sun_ptr);
}

STAPI_V2 st_return_t st_add_sun(st_context_v2_t pcxt,
								args_sun 		*args,
								double 		 	*angle,
								double 		 	*intensity) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_add_sun", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    st_return_t code = st_return_code::SUCCESS;
    auto sun = get_or_create_sun(data);

    sun->set_position(args->x, args->y, args->z);

    if (args->npoints)
    {
        std::vector<double> v_angle(angle, 
                                    angle + args->npoints);
        std::vector<double> v_intensity(intensity, 
                                        intensity + args->npoints);
        sun->set_shape(SunShape::USER_DEFINED,
                       0, 0, 0,
                       v_angle, 
                       v_intensity);
    }
    else
    {
        SunShape sun_shape = SunShape::GAUSSIAN;
        double s_hw_csr = args->sigma_halfwidth_csr;
        switch (args->shape)
        {
            /* default to gaussian shape, 
            using default to add warning */
            case 'g':
            case 'G':
                // sun_shape = SunShape::GAUSSIAN;
                break;
            case 'p':
            case 'P':
                sun_shape = SunShape::PILLBOX;
                break;
            /* currently no set_limbdarkend_distribution
            function is implemented, so ignore and 
            emit warning
            case 'l':
            case 'L':
            {
                sun_shape = SunShape::LIMBDARKENED;
                break;
            }                                           */
            case 'b':
            case 'B':
                sun_shape = SunShape::BUIE_CSR;
                break;
            /* user defined sun shape goes thru same 
            Sun::set_shape, emit warning because not 
            enough information from this signiture, 
            maybe add defaults?                      
            case 'u':
            case 'U':                             */
        default:
            // warning code default to gaussian, default sigma = 4.65
            sun_shape = SunShape::GAUSSIAN;
            s_hw_csr = 4.65;
            code = st_return_code::WARNING_SUN_SHAPE_IGNORED;
            break;
        }

        auto sun = get_or_create_sun(data);
        sun->set_shape(sun_shape,
                       s_hw_csr,
                       s_hw_csr,
                       s_hw_csr);
    }

    return code;
});
}

STAPI_V2 st_return_t st_get_sun(st_context_v2_t pcxt,
								args_sun 		*args,
								double 		 	**angle,
								double 		 	**intensity,
								st_uint_t 		npoints) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_sun", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    if (!data->get_number_of_ray_sources())
        return st_return_code::DATA_VALUE_NOT_FOUND;
 
    auto sun = data->get_ray_source(0);
    glm::dvec3 pos = sun->get_position();
    
    args->npoints = 0;
    args->x = pos[0];
    args->y = pos[1];
    args->z = pos[2];

    SunShape shape = sun->get_shape();
    args->shape = sunshape_to_char(shape);

    switch (shape)
    {
        case SunShape::GAUSSIAN:
        {
            args->sigma_halfwidth_csr = sun->get_sigma();
            break;
        }
        case SunShape::PILLBOX:
        {
            args->sigma_halfwidth_csr = sun->get_half_width();
            break;
        }
        case SunShape::BUIE_CSR:
        {
            args->sigma_halfwidth_csr = sun->get_circumsolar_ratio();
            break;
        }
        case SunShape::USER_DEFINED:
        {
            std::vector<double> _angle;
            std::vector<double> _intensity;
            sun->get_user_data(_angle, _intensity);

            if (npoints == 0 || npoints != _angle.size())
                return st_return_code::INVALID_ARGUMENTS;
            
            args->npoints = _angle.size();

            std::copy(_angle.begin(), _angle.end(), *angle);
            std::copy(_intensity.begin(), _intensity.end(), *intensity);
            break;
        }
    }

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sun_shape(st_context_v2_t pcxt,
							      char            shape, 
							      double          sigma_halfwidth_csr) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sun_shape", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    st_return_t code = st_return_code::SUCCESS; 
    SunShape sun_shape = SunShape::GAUSSIAN;

    switch (shape)
    {
        /* default to gaussian shape, 
           using default to add warning */
        case 'g':
        case 'G':
            // sun_shape = SunShape::GAUSSIAN;
            break;
        case 'p':
        case 'P':
            sun_shape = SunShape::PILLBOX;
            break;
        /* currently no set_limbdarkend_distribution
           function is implemented, so ignore and 
           emit warning
        case 'l':
        case 'L':
        {
            sun_shape = SunShape::LIMBDARKENED;
            break;
        }                                           */
        case 'b':
        case 'B':
            sun_shape = SunShape::BUIE_CSR;
            break;
        /* user defined sun shape goes thru same 
           Sun::set_shape, emit warning because not 
           enough information from this signiture, 
           maybe add defaults?                      
           case 'u':
           case 'U':                             */
    default:
        // warning code default to gaussian, default sigma = 4.65
        sun_shape = SunShape::GAUSSIAN;
        sigma_halfwidth_csr = 4.65;
        code = st_return_code::WARNING_SUN_SHAPE_IGNORED;
        break;
    }

    auto sun = get_or_create_sun(data);
    sun->set_shape(sun_shape,
                   sigma_halfwidth_csr,
                   sigma_halfwidth_csr,
                   sigma_halfwidth_csr);
    return code;
});
}

STAPI_V2 st_return_t st_sun_xyz(st_context_v2_t pcxt,
								double          x,
								double          y,
								double          z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sun_xyz", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);
    
    auto sun = get_or_create_sun(data);
    sun->set_position(x, y, z);
    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sun_position(st_context_v2_t pcxt,
									 double          lat,
									 double          day,
									 double          hour,
									 double          *x,
									 double          *y,
									 double          *z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sun_position", [&]() -> st_return_t {
    /* TODO: make st_sun_position_v2 that take a 
	computes the sun vector xyz given arguments
	lat : [deg] latitude 
	day : [] day of the year 
	hour : [hour] solar time. 12.00 corresponds to sun at maximum elevation and does not necessarily match local time

	xyz coordinate system:
		x: +west
		y: +zenith
		z: +north
	*/

	double Declination, HourAngle, Elevation, Azimuth;

	Declination = 180 / M_PI * asin(0.39795 * cos(0.98563 * M_PI / 180 * (day - 173)));
	HourAngle = 15 * (hour - 12);
	Elevation = 180 / M_PI * asin(sin(Declination * M_PI / 180) * sin(lat * M_PI / 180) + cos(Declination * M_PI / 180) * cos(HourAngle * M_PI / 180) * cos(lat * M_PI / 180));
	Azimuth = 180 / M_PI * acos((sin(M_PI / 180 * Declination) * cos(M_PI / 180 * lat) - cos(M_PI / 180 * Declination) * sin(M_PI / 180 * lat) * cos(M_PI / 180 * HourAngle)) / cos(M_PI / 180 * Elevation) + 0.0000000001);
	if (sin(HourAngle * M_PI / 180) > 0.0)
		Azimuth = 360 - Azimuth;
	*x = -sin(Azimuth * M_PI / 180) * cos(Elevation * M_PI / 180);
	*y = sin(Elevation * M_PI / 180);
	*z = cos(Azimuth * M_PI / 180) * cos(Elevation * M_PI / 180);

	return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_sun_userdata(st_context_v2_t pcxt,
									 st_uint_t 		 npoints,
									 double 		 *angle,
									 double 		 *intensity) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_sun_userdata", [&]() -> st_return_t {
    CONTEXT(pcxt);
    DATA(cxt);

    std::vector<double> v_angle(angle, angle + npoints);
    std::vector<double> v_intensity(intensity, intensity + npoints);
    auto sun = get_or_create_sun(data);
    sun->set_shape(SunShape::USER_DEFINED,
                   0, 0, 0,
                   v_angle, 
                   v_intensity);
    return st_return_code::SUCCESS;
});
}

st_return_code int_to_calc_method(int *arg, spcm *calc_method)
{
    if (*arg >= spcm::CALCULATOR_COUNT)
        return st_return_code::INVALID_ARGUMENTS;

    *calc_method = static_cast<spcm>(*arg);
    return st_return_code::SUCCESS;
}

void set_up_calculator(spcm				       calc,
					   args_sun_location       *loc,
					   args_sun_datetime       *dt,
                       SolarPositionCalculator *solar_position)
{
    solar_position->set_location(loc->latitude, loc->longitude, loc->timeZone, loc->altitude);
    solar_position->set_datetime(dt->year, dt->month, dt->day, dt->hour, dt->minute, dt->second);
    solar_position->set_method(calc);
}

STAPI_V2 st_return_t st_get_sun_az_zen(st_context_v2_t   pcxt,
									   int				 calc,
									   args_sun_location *loc,
									   args_sun_datetime *dt,
									   double 			 *azimuth,
									   double 			 *zenith) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_sun_az_zen", [&]() -> st_return_t {
    spcm method;
    st_return_code code = int_to_calc_method(&calc, &method);
    if (code != st_return_code::SUCCESS) return code;

    CONTEXT(pcxt);
    SolarPositionCalculator solar_position;
    
    set_up_calculator(method, loc, dt, &solar_position);
    solar_position.get_azimuth_zenith(azimuth, zenith);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_sun_az_el(st_context_v2_t   pcxt,
									  int				calc,
									  args_sun_location *loc,
									  args_sun_datetime *dt,
									  double 			*azimuth,
									  double 			*elevation) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_sun_az_el", [&]() -> st_return_t {
    spcm method;
    st_return_code code = int_to_calc_method(&calc, &method);
    if (code != st_return_code::SUCCESS) return code;

    CONTEXT(pcxt);
    SolarPositionCalculator solar_position;
    
    set_up_calculator(method, loc, dt, &solar_position);
    solar_position.get_azimuth_elevation(azimuth, elevation);

    return st_return_code::SUCCESS;
});
}

STAPI_V2 st_return_t st_get_sun_vector(st_context_v2_t   pcxt,
									   int				 calc,
									   args_sun_location *loc,
									   args_sun_datetime *dt,
									   double 			 *sun_x,
									   double 			 *sun_y,
									   double 			 *sun_z) noexcept
{
return st_guard::guarded(st_utils::get_cb(pcxt), "st_get_sun_vector", [&]() -> st_return_t {
    spcm method;
    st_return_code code = int_to_calc_method(&calc, &method);
    if (code != st_return_code::SUCCESS) return code;

    CONTEXT(pcxt);
    SolarPositionCalculator solar_position;
    
    set_up_calculator(method, loc, dt, &solar_position);
    solar_position.get_sun_vector(sun_x, sun_y, sun_z);

    return st_return_code::SUCCESS;
});
}
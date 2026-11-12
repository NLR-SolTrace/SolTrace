#ifndef SOLTRACE_SIMDATA_EXPORT_H
#define SOLTRACE_SIMDATA_EXPORT_H

#include "simulation_data_api.hpp"

// Model and Element Creation
using SolTrace::Data::ElementKey;
using SolTrace::Data::ModelContext;
using SolTrace::Data::ModelToken;
using SolTrace::Data::PendingElement;
using SolTrace::Data::SimulationData;
using SolTrace::Data::SimulationParameters;

// Elements
using SolTrace::Data::CompositeElement;
using SolTrace::Data::Element;
using SolTrace::Data::SingleElement;
using SolTrace::Data::StageElement;
using SolTrace::Data::VirtualElement;
using SolTrace::Data::VirtualPlane;

// CST Template Types
using SolTrace::Data::Heliostat;
using SolTrace::Data::LinearFresnel;
using SolTrace::Data::ParabolicDish;
using SolTrace::Data::ParabolicTrough;

// Apertures
using SolTrace::Data::Annulus;
using SolTrace::Data::Aperture;
using SolTrace::Data::ApertureType;
using SolTrace::Data::Circle;
using SolTrace::Data::EquilateralTriangle;
using SolTrace::Data::Hexagon;
using SolTrace::Data::IrregularQuadrilateral;
using SolTrace::Data::IrregularTriangle;
using SolTrace::Data::Rectangle;

// Surfaces
using SolTrace::Data::Cone;
using SolTrace::Data::Cylinder;
using SolTrace::Data::Flat;
using SolTrace::Data::Parabola;
using SolTrace::Data::Sphere;
using SolTrace::Data::Surface;
using SolTrace::Data::SurfaceType;

// Optical Properties
using SolTrace::Data::AngularTablePoint;
using SolTrace::Data::DistributionType;
using SolTrace::Data::InteractionType;
using SolTrace::Data::OpticalPropertySet;
using SolTrace::Data::OpticalPropertySetReference;
using SolTrace::Data::OpticalSide;

// Ray Sources
using SolTrace::Data::GenType;
using SolTrace::Data::RaySource;
using SolTrace::Data::SolarPositionCalculationMethod;
using SolTrace::Data::SolarPositionCalculator;
using SolTrace::Data::Sun;
using SolTrace::Data::SunShape;

// Identifier and Pointer Types
using SolTrace::Data::aperture_ptr;
using SolTrace::Data::composite_element_ptr;
using SolTrace::Data::element_id;
using SolTrace::Data::element_ptr;
using SolTrace::Data::optics_id;
using SolTrace::Data::ray_source_id;
using SolTrace::Data::ray_source_ptr;
using SolTrace::Data::single_element_ptr;
using SolTrace::Data::stage_ptr;
using SolTrace::Data::surface_ptr;

// Construction Functions
using SolTrace::Data::make_aperture;
using SolTrace::Data::make_ray_source;
using SolTrace::Data::make_surface;
using SolTrace::Data::make_surface_from_type;

// File Input and Output
using SolTrace::Data::kSchemaVersion;
using SolTrace::Data::load_json_file;
using SolTrace::Data::load_stinput_file;
using SolTrace::Data::write_json_file;

// Model Functions
using SolTrace::Data::same_model;

// Coordinate Transform Functions
using SolTrace::Data::CalculateTransformMatrices;
using SolTrace::Data::TransformToLocal;
using SolTrace::Data::TransformToReference;

// Utility Functions
using SolTrace::Data::abs_max;
using SolTrace::Data::abs_min;
using SolTrace::Data::is_approx;
using SolTrace::Data::project_onto_plane;
using SolTrace::Data::project_onto_vector;
using SolTrace::Data::rotate_vector_degrees;
using SolTrace::Data::rotate_vector_radians;

// Status Constants
using SolTrace::Data::ELEMENT_ALREADY_REGISTERED;
using SolTrace::Data::ELEMENT_ERROR;
using SolTrace::Data::ELEMENT_INVALID_SETUP;
using SolTrace::Data::ELEMENT_NULL;
using SolTrace::Data::ELEMENT_WRONG_MODEL;
using SolTrace::Data::OPTICS_ID_UNASSIGNED;
using SolTrace::Data::OPTICS_ID_VIRTUAL;

// Math Constants
using SolTrace::Data::D2R;
using SolTrace::Data::PI;
using SolTrace::Data::R2D;

#endif

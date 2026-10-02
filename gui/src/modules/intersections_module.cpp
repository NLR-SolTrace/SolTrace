#include "intersections_module.h"

#include <QDebug>

namespace SolTrace::GUI::App {

IntersectionsModule::IntersectionsModule(QObject* parent)
    : QObject(parent),
      m_entity_model(new SolTrace::GUI::Data::AllElementsModel(this)),
      m_ray_geometry(new SolTrace::GUI::Analysis::RayGeometry) {
    m_ray_geometry->setParent(this);
}

void IntersectionsModule::set_results(SolTrace::GUI::Data::SimulationResultPtr ptr) {
    m_results = ptr;
    m_entity_model->reset(nullptr);
    m_ray_geometry->set_results(ptr);

    set_has_results(ptr != nullptr);
    set_sun_ray_count(ptr ? ptr->sun_ray_count : 0);
    set_sun_plane_width(ptr ? ptr->sun_width : 0.0);
    set_sun_plane_height(ptr ? ptr->sun_height : 0.0);
    set_ray_area_weight(ptr ? ptr->ray_area_weight : 0.0);

    if (!ptr) return;

    m_entity_model->reset(const_cast<SolTrace::GUI::Data::Database*>(ptr->database.get()));
}


} // namespace SolTrace::GUI::App

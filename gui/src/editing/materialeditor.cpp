#include "materialeditor.h"

#include "data/database.h"
#include "support/enummodel.h"

namespace SolTrace::GUI::Data {

using DistributionModel =
    Support::EnumModel<SD::DistributionType,
                       SD::DistributionType::USER_DEFINED,
                       SD::DistributionType::UNKNOWN>;


MaterialEditor::MaterialEditor(QObject* parent)
    : QObject { parent },
      m_back_editor(new OpticalPropertiesObject(true, this)),
      m_front_editor(new OpticalPropertiesObject(false, this)),
      m_interaction_type_model(
          new Support::EnumModel<SD::InteractionType,
                                 SD::InteractionType::UNKNOWN>(this)),
      m_distribution_type_model(
          new DistributionModel(this, [](SD::DistributionType value) {
              if (value == SD::DistributionType::NONE) {
                  return MaterialEditor::tr("No errors");
              }

              return DistributionModel::default_display_name(value);
          })) { }

MaterialEditor::~MaterialEditor() = default;

void MaterialEditor::parameters_changed(entt::entity e) {
    if (this->m_current_group != e) return;
}

void MaterialEditor::set_new_database_connections(Database* ptr) {
    add_connection(connect(ptr->material_parameters.self(),
                           &ComponentAPIBase::changed,
                           this,
                           &MaterialEditor::parameters_changed));
}

void MaterialEditor::set(Database* database, entt::entity group) {
    observe(database);
    m_current_group = group;
    m_front_editor->set(database, group);
    m_back_editor->set(database, group);
    emit updated();
}

} // namespace SolTrace::GUI::Data

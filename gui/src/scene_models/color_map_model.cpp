#include "color_map_model.h"

#include <array>
#include <utility>

namespace SolTrace::GUI::Data {

namespace {

struct BuiltinColorMap {
    char const* id;
    char const* name;
    char const* file_name;
};

constexpr std::array BUILTIN_COLOR_MAPS = {
    BuiltinColorMap { "classic", "Classic Blue–Red", "b_to_r_wide.png" },
    BuiltinColorMap { "berlin", "Berlin", "berlin.png" },
    BuiltinColorMap { "lipari", "Lipari", "lipari.png" },
    BuiltinColorMap { "managua", "Managua", "managua.png" },
    BuiltinColorMap { "navia", "Navia", "navia.png" },
    BuiltinColorMap { "vanimo", "Vanimo", "vanimo.png" },
};

BuiltinColorMap const& definition_for(QString const& requested_id) {
    for (auto const& definition : BUILTIN_COLOR_MAPS) {
        if (requested_id == QLatin1String(definition.id)) return definition;
    }

    return BUILTIN_COLOR_MAPS.front();
}

QString qml_source(BuiltinColorMap const& definition) {
    return QStringLiteral("qrc:/assets/images/") +
           QLatin1String(definition.file_name);
}

QString resource_path(BuiltinColorMap const& definition) {
    return QStringLiteral(":/assets/images/") +
           QLatin1String(definition.file_name);
}

} // namespace

ColorMapModel::ColorMapModel(QObject* parent) : StructModelAdapter(parent) {
    QVector<ColorMapRecord> records;
    records.reserve(static_cast<qsizetype>(BUILTIN_COLOR_MAPS.size()));

    for (auto const& definition : BUILTIN_COLOR_MAPS) {
        records.push_back(ColorMapRecord {
            .id     = QLatin1String(definition.id),
            .name   = QString::fromUtf8(definition.name),
            .source = qml_source(definition),
        });
    }

    store_reset(std::move(records));
}

QString ColorMapModel::source_for(QString const& id) const {
    return source_for_id(id);
}

QString ColorMapModel::name_for(QString const& id) const {
    return name_for_id(id);
}

QString ColorMapModel::normalized_id(QString const& id) {
    return QLatin1String(definition_for(id).id);
}

QString ColorMapModel::source_for_id(QString const& id) {
    return qml_source(definition_for(id));
}

QString ColorMapModel::name_for_id(QString const& id) {
    return QString::fromUtf8(definition_for(id).name);
}

QImage ColorMapModel::image_for_id(QString const& id) {
    return QImage(resource_path(definition_for(id)));
}

} // namespace SolTrace::GUI::Data

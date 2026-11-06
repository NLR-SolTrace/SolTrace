#pragma once

#include "support/structmodel.h"

#include <QImage>
#include <QObject>
#include <QString>

namespace SolTrace::GUI::Data {

/// One built-in scalar color map available to QML and image generators.
struct ColorMapRecord {
    QString id;
    QString name;
    QString source;

    RECORD_META(ColorMapRecord,
                SM_EXPOSE_RO(id),
                SM_EXPOSE_RO(name),
                SM_EXPOSE_RO(source), );
};

/// Shared catalog of built-in scalar color maps.
class ColorMapModel : public Support::StructModelAdapter<ColorMapRecord> {
    Q_OBJECT

public:
    explicit ColorMapModel(QObject* parent = nullptr);

    Q_INVOKABLE QString source_for(QString const& id) const;
    Q_INVOKABLE QString name_for(QString const& id) const;

    static QString normalized_id(QString const& id);
    static QString source_for_id(QString const& id);
    static QString name_for_id(QString const& id);
    static QImage  image_for_id(QString const& id);
};

} // namespace SolTrace::GUI::Data

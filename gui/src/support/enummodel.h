#pragma once

#include "magic_enum/magic_enum.hpp"

#include <QRangeModel>
#include <QObject>
#include <QString>
#include <QtTypes>
#include <QVector>

#include <type_traits>

namespace SolTrace::GUI::Support {


struct EnumModelRecord {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(qint32 value MEMBER value)

public:
    QString name;
    qint32  value;
};

} // namespace SolTrace::GUI::Support

template <>
struct QRangeModel::RowOptions<SolTrace::GUI::Support::EnumModelRecord> {
    static constexpr QRangeModel::RowCategory rowCategory =
        QRangeModel::RowCategory::MultiRoleItem;
};

namespace SolTrace::GUI::Support {

template <class E, E... Excluded>
class EnumModel : public QRangeModel {
    static_assert(std::is_enum_v<E>);

    static constexpr bool is_excluded(E value) {
        return ((value == Excluded) || ... || false);
    }

    static QVector<EnumModelRecord> make_all_records() {
        QVector<EnumModelRecord> ret;

        for (auto const& [value, name] : magic_enum::enum_entries<E>()) {
            if (is_excluded(value)) continue;

            auto fixed_name =
                QString::fromUtf8(name.data(), qsizetype(name.size()))
                    .toLower();

            fixed_name.replace('_', ' ');

            if (!fixed_name.isEmpty()) {
                fixed_name[0] = fixed_name[0].toUpper();
            }

            ret.push_back(EnumModelRecord {
                .name = fixed_name,
                .value =
                    static_cast<qint32>(magic_enum::enum_integer(value)) });
        }

        return ret;
    }

public:
    explicit EnumModel(QObject* parent = nullptr)
        : QRangeModel(make_all_records(), parent) { }
};


} // namespace SolTrace::GUI::Support

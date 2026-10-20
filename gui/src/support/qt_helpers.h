#pragma once

#include <QPointer>

//─── Property Macros ───────────────────────────────────────────────────────────────────

#define QOBJECT_WRITABLE_PROPERTY(TYPE, PROPERTY_NAME)                         \
public:                                                                        \
    Q_PROPERTY(TYPE* PROPERTY_NAME READ PROPERTY_NAME WRITE                    \
                   set_##PROPERTY_NAME NOTIFY PROPERTY_NAME##_changed)         \
protected:                                                                     \
    QPointer<TYPE> m_##PROPERTY_NAME;                                          \
                                                                               \
public:                                                                        \
    TYPE* PROPERTY_NAME() const {                                              \
        return m_##PROPERTY_NAME;                                              \
    }                                                                          \
public Q_SLOTS:                                                                \
    void set_##PROPERTY_NAME(TYPE* new_value) {                                \
        if (m_##PROPERTY_NAME == new_value) return;                            \
        m_##PROPERTY_NAME = new_value;                                         \
        Q_EMIT PROPERTY_NAME##_changed();                                      \
        Q_EMIT PROPERTY_NAME##_value_changed(new_value);                       \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void PROPERTY_NAME##_changed();                                            \
    void PROPERTY_NAME##_value_changed(TYPE* new_value);                       \
                                                                               \
public:


#define QOBJECT_READONLY_PROPERTY(TYPE, PROPERTY_NAME)                         \
public:                                                                        \
    Q_PROPERTY(TYPE* PROPERTY_NAME READ PROPERTY_NAME CONSTANT)                \
protected:                                                                     \
    QPointer<TYPE> m_##PROPERTY_NAME;                                          \
                                                                               \
public:                                                                        \
    TYPE* PROPERTY_NAME() const {                                              \
        return m_##PROPERTY_NAME;                                              \
    }                                                                          \
                                                                               \
public:


#define Q_WRITABLE_PROPERTY(TYPE, PROPERTY_NAME, VALUE)                        \
public:                                                                        \
    Q_PROPERTY(TYPE PROPERTY_NAME READ PROPERTY_NAME WRITE                     \
                   set_##PROPERTY_NAME NOTIFY PROPERTY_NAME##_changed)         \
protected:                                                                     \
    TYPE m_##PROPERTY_NAME = VALUE;                                            \
                                                                               \
public:                                                                        \
    TYPE PROPERTY_NAME() const {                                               \
        return m_##PROPERTY_NAME;                                              \
    }                                                                          \
public Q_SLOTS:                                                                \
    void set_##PROPERTY_NAME(TYPE const& new_value) {                          \
        if (m_##PROPERTY_NAME == new_value) return;                            \
        m_##PROPERTY_NAME = new_value;                                         \
        Q_EMIT PROPERTY_NAME##_changed();                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void PROPERTY_NAME##_changed();                                            \
                                                                               \
public:

#define Q_WRITABLE_PROPERTY_INDIRECT(TYPE, PROPERTY_NAME, GETTER, SETTER)      \
public:                                                                        \
    Q_PROPERTY(TYPE PROPERTY_NAME READ PROPERTY_NAME WRITE                     \
                   set_##PROPERTY_NAME NOTIFY PROPERTY_NAME##_changed)         \
                                                                               \
public:                                                                        \
    TYPE PROPERTY_NAME() const {                                               \
        return GETTER();                                                       \
    }                                                                          \
public Q_SLOTS:                                                                \
    void set_##PROPERTY_NAME(TYPE const& new_value) {                          \
        if (GETTER() == new_value) return;                                     \
        SETTER(new_value);                                                     \
        Q_EMIT PROPERTY_NAME##_changed();                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void PROPERTY_NAME##_changed();                                            \
                                                                               \
public:

#define Q_WRITABLE_PROPERTY_CUSTOM_STORAGE(TYPE, PROPERTY_NAME, LOCATION)      \
public:                                                                        \
    Q_PROPERTY(TYPE PROPERTY_NAME READ PROPERTY_NAME WRITE                     \
                   set_##PROPERTY_NAME NOTIFY PROPERTY_NAME##_changed)         \
                                                                               \
public:                                                                        \
    TYPE PROPERTY_NAME() const {                                               \
        return LOCATION;                                                       \
    }                                                                          \
public Q_SLOTS:                                                                \
    void set_##PROPERTY_NAME(TYPE const& new_value) {                          \
        if ((LOCATION) == new_value) return;                                   \
        LOCATION = new_value;                                                  \
        Q_EMIT PROPERTY_NAME##_changed();                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void PROPERTY_NAME##_changed();                                            \
                                                                               \
public:


#define Q_READONLY_PROPERTY(TYPE, PROPERTY_NAME)                               \
public:                                                                        \
    Q_PROPERTY(                                                                \
        TYPE PROPERTY_NAME READ PROPERTY_NAME NOTIFY PROPERTY_NAME##_changed)  \
protected:                                                                     \
    TYPE m_##PROPERTY_NAME = TYPE();                                           \
                                                                               \
public:                                                                        \
    TYPE PROPERTY_NAME() const {                                               \
        return m_##PROPERTY_NAME;                                              \
    }                                                                          \
    void set_##PROPERTY_NAME(TYPE const& new_value) {                          \
        if (m_##PROPERTY_NAME == new_value) return;                            \
        m_##PROPERTY_NAME = new_value;                                         \
        Q_EMIT PROPERTY_NAME##_changed();                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void PROPERTY_NAME##_changed();                                            \
                                                                               \
public:

// Property macro definitions end here.

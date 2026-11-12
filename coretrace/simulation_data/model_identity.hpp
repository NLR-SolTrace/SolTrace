/**
 * @file model_identity.hpp
 * @brief Token that tells whether two objects belong to the same model
 */

#ifndef SOLTRACE_MODEL_IDENTITY_H
#define SOLTRACE_MODEL_IDENTITY_H

#include <memory>

namespace SolTrace::Data {

// Empty and never serialized. A model owns one, and every object created for
// the model keeps a weak reference to it as its token.
struct ModelIdentity
{
};

using ModelToken = std::weak_ptr<const ModelIdentity>;

// True if neither token has expired and both refer to the same identity.
inline bool same_model(const ModelToken &a, const ModelToken &b)
{
    if (a.expired() || b.expired())
    {
        return false;
    }
    return !a.owner_before(b) && !b.owner_before(a);
}

} // namespace SolTrace::Data

#endif

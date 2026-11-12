/**
 * @file stage_element.hpp
 * @brief Stage-based element organization
 *
 * Defines stage-based organization of optical elements,
 * allowing for multi-stage optical system modeling.
 * Stages provide logical grouping and sequential processing
 * of optical elements in ray tracing simulations.
 */

#ifndef SOLTRACE_STAGE_ELEMENT_H
#define SOLTRACE_STAGE_ELEMENT_H

#include <memory>
#include "composite_element.hpp"
#include "element.hpp"

namespace SolTrace::Data {

class StageElement: public CompositeElement
{
public:
    StageElement(ElementKey key, int_fast64_t stage);
    StageElement(ElementKey key,
        const nlohmann::ordered_json& jnode,
        const OpticalPropertySetResolver& resolve_optics);
    ~StageElement();
    virtual bool is_stage() const override { return true; }
    virtual void write_json(nlohmann::ordered_json& jnode) const override;
    
    // only single elements have groups
    virtual int32_t get_group() const override { return -2; }
private:
};

using stage_ptr = std::shared_ptr<StageElement>;

} // namespace SolTrace::Data

#endif

#include "stage_element.hpp"

#include <sstream>

#include "element.hpp"
#include "json_helpers.hpp"


namespace SolTrace::Data {

StageElement::StageElement(ElementKey key, int_fast64_t stage)
    : CompositeElement(key)
{
    this->set_stage(stage);
    return;
}

StageElement::StageElement(ElementKey key,
    const nlohmann::ordered_json& jnode,
    const OpticalPropertySetResolver& resolve_optics) 
    : CompositeElement(key, jnode, resolve_optics)
{
    // Check that it is a stage
    if (jnode.contains("is_stage") == false || jnode.at("is_stage").get<bool>() == false)
    {
        std::stringstream ss;
        ss << "JSON node is not a valid stage: ";
        if (jnode.contains("is_stage") == false) {
            ss << "missing 'is_stage' field.";
        } else {
            ss << "'is_stage' field is present but has value: " << jnode.at("is_stage") << ".";
        }
        throw std::invalid_argument(ss.str());
    }
    // Get and set stage number
    int stage = jnode.at("stage");
    this->set_stage(stage);
    
    // CompositeElement is responsible for populating elements
    // Its constructor has already been called
}

StageElement::~StageElement()
{
    return;
}

void StageElement::write_json(nlohmann::ordered_json& jnode) const
{
    // Write stage-specific json keys
    jnode["is_stage"] = true;
    jnode["is_single"] = false;

    // Call composite write_json
    this->CompositeElement::write_json(jnode);
}


} // namespace SolTrace::Data

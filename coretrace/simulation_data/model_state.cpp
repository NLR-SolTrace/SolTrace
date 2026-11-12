#include "model_state.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

namespace SolTrace::Data {

element_id ElementIdAllocator::next()
{
    if (this->highest_ == std::numeric_limits<element_id>::max())
    {
        throw std::overflow_error("ElementIdAllocator: no larger ID exists");
    }
    const element_id id = this->highest_ + 1;
    this->used_.insert(id);
    this->highest_ = id;
    return id;
}

void ElementIdAllocator::reserve(element_id id)
{
    if (id < 1)
    {
        std::stringstream ss;
        ss << "ElementIdAllocator: element IDs must be 1 or greater, got "
           << id;
        throw std::invalid_argument(ss.str());
    }
    if (!this->used_.insert(id).second)
    {
        std::stringstream ss;
        ss << "ElementIdAllocator: element ID " << id << " is already in use";
        throw std::invalid_argument(ss.str());
    }
    if (id > this->highest_)
    {
        this->highest_ = id;
    }
}

void ElementIdAllocator::reset()
{
    this->used_.clear();
    this->highest_ = 0;
}

ModelState::ModelState() : identity_(std::make_shared<const ModelIdentity>())
{
}

void ModelState::reset()
{
    this->identity_ = std::make_shared<const ModelIdentity>();
    this->ids.reset();
}

} // namespace SolTrace::Data

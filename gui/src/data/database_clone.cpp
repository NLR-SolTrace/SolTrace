#include "database_clone.h"

#include "components.h"

namespace SolTrace::GUI::Data {

/// Remap entities on clone
struct EntityMapper {
    entt::registry&                                new_registry;
    std::unordered_map<entt::entity, entt::entity> old_to_new_map;

    entt::entity operator()(entt::entity e) {
        auto item = old_to_new_map.find(e);

        if (item == old_to_new_map.end()) {
            auto new_e        = new_registry.create();
            old_to_new_map[e] = new_e;
            return new_e;
        }

        return item->second;
    }
};

template <class T>
void copy_marker_component(entt::registry const& from,
                           EntityMapper&         mapper,
                           entt::registry&       to) {
    for (auto [e] : from.view<T>().each()) {
        to.emplace_or_replace<T>(mapper(e));
    }
}

template <class T>
void copy_plain_component(entt::registry const& from,
                          EntityMapper&         mapper,
                          entt::registry&       to) {
    for (auto [e, c] : from.view<T>().each()) {
        to.emplace_or_replace<T>(mapper(e), c);
    }
}

template <class T>
void copy_nested_component(entt::registry const& from,
                           EntityMapper&         mapper,
                           entt::registry&       to) {
    for (auto [e, c] : from.view<T>().each()) {
        T component_copy = c;
        component_copy.remap_entities(mapper);
        to.emplace_or_replace<T>(mapper(e), component_copy);
    }
}

DatabaseCloneResult clone_database_with_entity_map(Database const& from,
                                                   QString  new_database_name,
                                                   QObject* p) {
    auto  ret           = std::make_unique<Database>(new_database_name, p);
    auto& from_registry = from.as_registry();
    auto& to_registry   = ret->as_registry();

    EntityMapper mapper {
        .new_registry   = to_registry,
        .old_to_new_map = { },
    };

    copy_marker_component<InvisibleComponent>(
        from_registry, mapper, to_registry);

    copy_marker_component<DisabledComponent>(
        from_registry, mapper, to_registry);

    copy_marker_component<VirtualTagComponent>(
        from_registry, mapper, to_registry);

    copy_plain_component<IdentityComponent>(from_registry, mapper, to_registry);


    copy_marker_component<ElementComponent>(from_registry, mapper, to_registry);


    copy_nested_component<ChildOfComponent>(from_registry, mapper, to_registry);

    copy_nested_component<ChildrenComponent>(
        from_registry, mapper, to_registry);


    copy_plain_component<TransformComponent>(
        from_registry, mapper, to_registry);

    copy_plain_component<GlobalTransformComponent>(
        from_registry, mapper, to_registry);


    if (auto const* ray_source = from.ray_source_resource.get()) {
        ret->ray_source_resource.set(ray_source->clone());
    }
    ret->simulation_parameters_resource.set(
        from.simulation_parameters_resource.require());


    copy_plain_component<MaterialComponent>(from_registry, mapper, to_registry);

    copy_nested_component<MaterialGroupComponent>(
        from_registry, mapper, to_registry);

    copy_nested_component<MaterialGroupMemberComponent>(
        from_registry, mapper, to_registry);

    // GeometryComponent
    {
        for (auto [e, c] : from_registry.view<GeometryComponent>().each()) {
            auto local = c.clone();
            to_registry.emplace<GeometryComponent>(mapper(e), local);
        }
    }

    copy_nested_component<GeometryGroupComponent>(
        from_registry, mapper, to_registry);

    copy_nested_component<GeometryGroupMemberComponent>(
        from_registry, mapper, to_registry);

    copy_marker_component<TagComponent>(from_registry, mapper, to_registry);
    copy_marker_component<HiddenTagComponent>(
        from_registry, mapper, to_registry);

    // Script references are session metadata rather than simulation data, but
    // an in-memory database clone should retain them and point at the cloned
    // entities on which the components live.
    copy_plain_component<ScriptReferenceComponent>(
        from_registry, mapper, to_registry);

    // Rebuild both sides of valid tag membership from the forward relation.
    // This deliberately does not copy stale entries from dynamic tag storage.
    for (auto const& [entity, membership] :
         from_registry.view<TagMembershipComponent>().each()) {
        TagMembershipComponent membership_copy;
        for (auto tag : membership.tags) {
            if (!from_registry.valid(tag) ||
                !from_registry.all_of<TagComponent>(tag)) {
                continue;
            }
            membership_copy.tags.push_back(mapper(tag));
        }

        if (membership_copy.tags.isEmpty()) continue;

        auto const copied_entity = mapper(entity);
        to_registry.emplace<TagMembershipComponent>(copied_entity,
                                                    membership_copy);
        for (auto copied_tag : membership_copy.tags) {
            auto& storage = to_registry.storage<ATagMemberComponent>(
                entt::to_integral(copied_tag.value));
            storage.emplace(copied_entity);
        }
    }

    copy_plain_component<ImportErrorComponent>(
        from_registry, mapper, to_registry);

    copy_marker_component<SelectedComponent>(
        from_registry, mapper, to_registry);

    copy_plain_component<ColorComponent>(from_registry, mapper, to_registry);

    ret->set_modified(false);

    return DatabaseCloneResult {
        .database       = std::move(ret),
        .old_to_new_map = std::move(mapper.old_to_new_map),
    };
}

} // namespace SolTrace::GUI::Data

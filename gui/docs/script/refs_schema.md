# Shared Scene References

The `refs` global stores lightweight entity references in the current scene. References are shared by every script that opens the same scope, but they are currently retained only for the open GUI session and are not exported to simulation files.

Scopes and reference names are case-sensitive. Empty or whitespace-only names are rejected. Removing or replacing a reference never deletes its scene entity, while deleting an entity automatically removes its references.

## refs.scope(name)

Open a shared namespace and return its `ReferenceScope`. Scripts can open several scopes, and separate scripts can deliberately collaborate by using the same stable scope name.

```js
const field = refs.scope("nlr.heliostat-field")
```

## refs.scopes()

Return the sorted names of scopes that currently contain at least one reference.

## ReferenceScope.single(name)

Open a named reference that addresses at most one entity.

```js
const receiver = field.single("receiver")
receiver.set(entity)
const sameEntity = receiver.get()
receiver.clear()
```

`set(entity)` replaces the old reference without deleting the previously referenced entity. `get()` returns an invalid entity handle when unset. `clear()` removes only the reference.

## ReferenceScope.group(name)

Open a named reference that can address several entities.

```js
const mirrors = field.group("mirrors")
mirrors.add(firstMirror)
mirrors.add(secondMirror)

for (const mirror of mirrors.entities()) {
    // Update an existing mirror.
}
```

`add(entity)` is idempotent. `remove(entity)` and `clear()` remove references without deleting scene entities. `contains(entity)` tests membership and `entities()` returns all current members.

## ReferenceScope.single_names()

Return the sorted names of populated single references in this scope.

## ReferenceScope.group_names()

Return the sorted names of populated groups in this scope.

# Release tooling

`release_tools.py` contains the platform packaging behavior used by
`.github/workflows/gui-release.yml`. The workflow owns the build matrix and
third-party setup; these modules own archive construction, runtime staging,
license collection, and static artifact validation.

Run the CLI from the repository root:

```console
python -m scripts.release.release_tools --help
```

The commands use only the Python standard library. Platform commands such as
`otool`, `dumpbin`, `ditto`, `linuxdeploy`, and `gh` are invoked as subprocesses
and must be available for the relevant command.

When Apple signing secrets are configured, macOS artifacts from `main` and
version tags are signed with a Developer ID Application certificate using the
hardened runtime, submitted to Apple's notary service, stapled, and assessed
with Gatekeeper before packaging. The app bundle is signed inside-out after
runtime libraries and licenses are staged. Without those secrets, macOS builds
fall back to ad-hoc signing and continue without notarization; tag publication
is skipped in that case. Ad-hoc artifacts are intended only for local testing.

Configure these GitHub Actions secrets for macOS release signing:

- `MACOS_DEVELOPER_ID_P12`: Base64-encoded Developer ID Application certificate
   exported as a `.p12` file.
- `MACOS_DEVELOPER_ID_P12_PASSWORD`: Password used to export the certificate.
- `MACOS_DEVELOPER_IDENTITY`: Exact signing identity listed by
   `security find-identity -v -p codesigning`, including the team identifier.
- `APP_STORE_CONNECT_API_KEY`: Contents of the App Store Connect API `.p8` key.
- `APP_STORE_CONNECT_KEY_ID`: Key ID corresponding to that API key.
- `APP_STORE_CONNECT_ISSUER_ID`: App Store Connect issuer ID.

The App Store Connect API key is used only for notarization, not App Store
submission. The certificate and key are imported or written only on the
ephemeral macOS runner and are not included in the release artifact.

macOS ZIPs and Linux AppImages are accompanied by SHA-256 checksums. Linux
AppImages also have a keyless Sigstore signature bundle generated through
GitHub Actions OIDC. Main-branch CI verifies the macOS bundle signature inside
the ZIP and the Linux Sigstore signature before uploading the artifacts. For a
Linux release, verify both files from the same release:

```console
sha256sum --check "$APPIMAGE.sha256"
cosign verify-blob \
   --bundle "$APPIMAGE.sigstore.json" \
   --certificate-identity-regexp '^https://github.com/nlr-devops/SolTrace/.github/workflows/gui-release.yml@refs/(heads/main|tags/v.*)$' \
   --certificate-oidc-issuer https://token.actions.githubusercontent.com \
   "$APPIMAGE"
```

Run the portable unit tests with:

```console
python -m unittest discover -s scripts/release/tests -v
```

Keep GitHub-specific expressions and matrix decisions in the workflow. Put
branching, filesystem traversal, output parsing, and artifact validation here
so those behaviors remain readable and testable outside GitHub Actions.

## Why installers package a staged directory

The project install tree serves both application users and developers, so it
contains runtime files as well as headers, libraries, and CMake package files.
Official releases also use pinned binary dependencies and collect licenses that
may not exist in a developer's local dependency installation. Those concerns
must not become prerequisites for a normal local build or `cmake --install`.

The release pipeline therefore has two explicit phases:

1. CMake installs normally into `dist`, after which the Python tooling adds the
   pinned runtime libraries, licenses, and performs dependency validation.
2. Windows MSI packaging copies a consumer subset into `installer-root`,
   excluding the top-level `include` and `lib` development trees. A standalone
   CPack configuration packages that directory with WiX. It does not include the
   project's generated CPack configuration and does not execute install rules.

The Windows ZIP still packages the complete `dist` tree. This keeps a portable,
developer-friendly artifact while each MSI contains the consumer runtime. The
standard Embree and Embree+OptiX installers use distinct stable UpgradeCodes,
install directories, and Start menu folders. They therefore upgrade
independently and can be installed side-by-side.

Official OptiX artifacts explicitly target CUDA compute capability 7.5 as their
minimum. The build does not inspect the CI host GPU; it emits Turing native code
plus forward-compatible virtual PTX for newer NVIDIA GPUs.

The MSI command expects a validated Windows `dist` tree, CMake 3.30 or newer,
and WiX 4 with its matching UI extension. CI pins and installs those tools. The
generated installer is x64, per-machine, installs under Program Files, and
therefore requests elevation. Building or running SolTrace locally does not
invoke this command.

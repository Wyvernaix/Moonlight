# Packaging

Packaging consumes installed targets and versioned data; it never compiles
source by copying files. Platform manifests, signing configuration, license
bundles, and release notes are kept separate. Signing keys and certificates
are supplied by CI secrets or the platform keychain and never stored here.

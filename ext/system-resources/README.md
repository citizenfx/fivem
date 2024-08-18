# `system-resources`

Build scripts for bundled system resources.

## Index of resources

1. Chat - basic chat functionality
    - Source-code-based
2. Monitor - server manager, txAdmin
    - Artifact-based
    - Source code: https://github.com/tabarra/txAdmin
    - Using ready-built artifacts from the source code repository

## Updating artifact-based resources

Update artifact URL, version and SHA256 hash using this command:

```
$ node manager.js update --name=%RESOURCE_NAME% --url=%RESOURCE_ARTIFACT_URL% --version=%RESOURCE_VERSION%
```

## Adding artifact-based resource

```
$ node manager.js add --name=%RESOURCE_NAME% --url=%RESOURCE_ARTIFACT_URL% --version=%RESOURCE_VERSION%
```

> Requires adding extra build steps to `build.cmd` and `build.sh` scripts.

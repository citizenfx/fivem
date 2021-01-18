import { resourceManifestFilename, resourceManifestLegacyFilename } from "backend/constants";
import { FilesystemEntry } from "shared/api.types";
import { ResourceManifestKind } from "./resource-manifest";

export const getResourceManifestKind = (entry: FilesystemEntry): ResourceManifestKind => {
  if (entry.name === resourceManifestLegacyFilename) {
    return ResourceManifestKind.__resource;
  }

  if (entry.name === resourceManifestFilename) {
    return ResourceManifestKind.fxmanifest;
  }

  return ResourceManifestKind.none;
};

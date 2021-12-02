import { resourceManifestFilename, resourceManifestLegacyFilename } from "backend/constants";
import { ResourceManifestKind } from "./resourceManifest";

export function getResourceManifestKind(name: string): ResourceManifestKind;
export function getResourceManifestKind(entry: { name: string }): ResourceManifestKind;
export function getResourceManifestKind(entry: { name: string } | string): ResourceManifestKind {
  let name = typeof entry === 'string'
    ? entry
    : entry.name;

  if (name === resourceManifestLegacyFilename) {
    return ResourceManifestKind.__resource;
  }

  if (name === resourceManifestFilename) {
    return ResourceManifestKind.fxmanifest;
  }

  return ResourceManifestKind.none;
}

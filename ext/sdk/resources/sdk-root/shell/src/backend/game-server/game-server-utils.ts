import { ServerResourceDescriptor } from "./game-server-runtime";

export function getResourceNames(descriptors: ServerResourceDescriptor[]): string[] {
  return descriptors.map(({ name }) => name);
}

export function getResourcePaths(descriptors: ServerResourceDescriptor[]): string[] {
  return descriptors.map(({ path }) => path);
}

export function getResourceUrl({ name, path }: ServerResourceDescriptor): string {
  return `file:///${path.replace(/\\/g, '/')}#${name}`;
}

export function getResourceUrls(descriptors: ServerResourceDescriptor[]): string[] {
  return descriptors.map(getResourceUrl);
}

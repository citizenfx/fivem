import { ResourceManifest } from "backend/project/asset/asset-contributions/resource/resource-manifest";
import { AssetCreateRequest } from "shared/api.requests";

export interface ResourceTemplateDescriptor {
  id: string,
  icon: React.ReactNode,
  title: string,
  description?: React.ReactNode,

  useIsEnabled?(): boolean,
  disabledDescription?: React.ReactNode,
}

export interface ResourceTemplateScaffolderArgs {
  request: AssetCreateRequest<{ resourceTemplateId?: string }>,
  manifest: ResourceManifest,
  resourcePath: string,
}

export interface ResourceTemplateScaffolder {
  scaffold(args: ResourceTemplateScaffolderArgs): Promise<void> | void;
}

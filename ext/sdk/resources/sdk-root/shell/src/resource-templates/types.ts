import { ResourceManifest } from "assets/resource/resource-manifest";
import { APIRQ } from "shared/api.requests";

export interface ResourceTemplateDescriptor {
  id: string,
  icon: React.ReactNode,
  title: string,
  description?: React.ReactNode,

  useIsEnabled?(): boolean,
  disabledDescription?: React.ReactNode,
}

export interface ResourceTemplateScaffolderArgs {
  request: APIRQ.AssetCreate<{ resourceTemplateId?: string }>,
  manifest: ResourceManifest,
  resourcePath: string,
}

export interface ResourceTemplateScaffolder {
  scaffold(args: ResourceTemplateScaffolderArgs): Promise<void> | void;
}

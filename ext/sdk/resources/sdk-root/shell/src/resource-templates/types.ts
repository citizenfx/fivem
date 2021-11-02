import { ResourceManifest } from "fxdk/contrib/assets/resource/common/resourceManifest";
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
  manifest: ResourceManifest,
  resourceName: string,
  resourcePath: string,
  resourceTemplateId: string,
}

export interface ResourceTemplateScaffolder {
  scaffold(args: ResourceTemplateScaffolderArgs): Promise<void> | void;
}

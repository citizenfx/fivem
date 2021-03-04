import { ResourceTemplateDescriptor } from "./types";

export const resourceTemplateDescriptors: ResourceTemplateDescriptor[] = [
  require('./empty/descriptor').default,
  require('./lua/descriptor').default,
  require('./js/descriptor').default,
  require('./csharp/descriptor').default,
];

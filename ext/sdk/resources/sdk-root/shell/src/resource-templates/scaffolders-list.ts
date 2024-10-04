import { ResourceTemplateScaffolder } from "./types";

export const resourceTemplateScaffolders: Record<string, new() => ResourceTemplateScaffolder> = {
  empty: require('./empty/scaffolder').default,
  lua: require('./lua/scaffolder').default,
  js: require('./js/scaffolder').default,
  ts: require('./ts/scaffolder').default,
  csharp: require('./csharp/scaffolder').default,
};

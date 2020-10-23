import * as fs from 'fs';
import * as path from 'path';
import * as paths from './paths';

export function setupWellKnownPaths() {
  const wellKnownPaths = {
    'ts_types_path': path.join(paths.citizen, 'scripting/v8/index.d.ts').replace('\\', '/'),
  };

  fs.writeFileSync(paths.wellKnownPathsPath, JSON.stringify(wellKnownPaths));
}

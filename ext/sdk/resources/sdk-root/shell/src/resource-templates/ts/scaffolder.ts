import { inject, injectable } from "inversify";
import { FsService } from "backend/fs/fs-service";
import { concurrently } from "utils/concurrently";
import * as cp from "child_process";
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";

@injectable()
export default class TsScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  async scaffold({ manifest, resourceName, resourcePath }: ResourceTemplateScaffolderArgs) {
    manifest.clientScripts.push('dist/client.js');
    manifest.serverScripts.push('dist/server.js');

    manifest.fxdkWatchCommands.push([
      "yarn",
      [ "watch" ]
    ]);
    manifest.fxdkBuildCommands.push([
      "yarn",
      [ "build" ]
    ]);

    const promises: Promise<any>[] = [];
    const relativePath = (to: string) => this.fsService.joinPath(resourcePath, to);

    // Client Script
    promises.push(this.fsService.mkdirp(relativePath('client'))
      .then(() => concurrently(
          this.fsService.writeFile(relativePath('client/client.ts'), `console.log("[${resourceName}] Client Resource Started");`),
          this.fsService.writeFile(relativePath('client/tsconfig.json'), getClientTSConfig()),
        )));

    // Server Script
    promises.push(this.fsService.mkdirp(relativePath('server'))
      .then(() => concurrently(
          this.fsService.writeFile(relativePath('server/server.ts'), `console.log("[${resourceName}] Server Resource Started");`),
          this.fsService.writeFile(relativePath('server/tsconfig.json'), getServerTSConfig()),
        )));

    // Build Tools
    promises.push(this.fsService.writeFile(relativePath('package.json'), getPackageJson(resourceName)));
    promises.push(this.fsService.writeFile(relativePath('build.js'), getBuildJS()));
    promises.push(this.fsService.writeFile(relativePath('.fxdkignore'), getIgnore()));

    await Promise.all(promises);

    await this.installModules(resourcePath);
  }

  private async installModules(path: string) {
    return new Promise<void>((resolve, reject) => {
      cp.exec("yarn", { cwd: path, windowsHide: true }, (err) => {
        if (err) {
          return reject(err);
        }

        return resolve();
      });
    });
  }
}

function getIgnore(): string {
  return `client/*
server/*
package.json
build.js
`;
}

function getClientTSConfig(): string {
  return `{
  "compilerOptions": {
    "baseUrl": ".",
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "es2021",
    "lib": [
      "es2021"
    ],
    "types": [
      "@citizenfx/client"
    ],
    "moduleResolution": "node",
    "resolveJsonModule": true,
    "esModuleInterop": true,
    "noEmit": true,
  },
  "include": [
    "./",
    "../common"
  ]
}`;
}

function getServerTSConfig(): string {
  return `{
  "compilerOptions": {
    "baseUrl": ".",
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "es2021",
    "lib": [
      "es2021"
    ],
    "types": [
      "@citizenfx/server",
      "@types/node"
    ],
    "moduleResolution": "node",
    "resolveJsonModule": true,
    "esModuleInterop": true,
    "noEmit": true
  },
  "include": [
    "./",
    "../common"
  ]
}`;
}

function getPackageJson(resourceName: string): string {
  return `{
  "name": "${resourceName}",
  "version": "1.0.0",
  "main": "index.js",
  "author": "you",
  "license": "MIT",
  "scripts": {
    "build": "node build.js --mode=production",
    "watch": "node build.js"
  },
  "devDependencies": {
    "@citizenfx/client": "latest",
    "@citizenfx/server": "latest",
    "@types/node": "^16.3.1",
    "esbuild": "^0.14.10"
  }
}`;
}

function getBuildJS(): string {
  return `const esbuild = require('esbuild');

const production = process.argv.findIndex(argItem => argItem === '--mode=production') >= 0;

const onRebuild = (context) => {
  return async (err, res) => {
    if (err) {
      return console.error(\`[\${context}]: Rebuild failed\`, err);
    }

    console.log(\`[\${context}]: Rebuild succeeded, warnings:\`, res.warnings);
  }
}

const server = {
  platform: 'node',
  target: ['node16'],
  format: 'cjs',
};

const client = {
  platform: 'browser',
  target: ['chrome93'],
  format: 'iife',
};

for (const context of [ 'client', 'server' ]) {
  esbuild.build({
    bundle: true,
    entryPoints: [\`\${context}/\${context}.ts\`],
    outfile: \`dist/\${context}.js\`,
    watch: production ? false : {
      onRebuild: onRebuild(context),
    },
    ...(context === 'client' ? client : server),
  }).then(() => console.log(\`[\${context}]: Built successfully!\`)).catch(() => process.exit(1));
}`;
}

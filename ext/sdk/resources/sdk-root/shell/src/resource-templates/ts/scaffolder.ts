import { inject, injectable } from "inversify";
import { FsService } from "backend/fs/fs-service";
import * as cp from 'child_process';
import { concurrently } from "utils/concurrently";
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";

@injectable()
export default class TsScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  async scaffold({ request, manifest, resourcePath }: ResourceTemplateScaffolderArgs) {
    const resourceName = request.assetName;
    
    manifest.clientScripts.push('dist/client/*.client.js');
    manifest.serverScripts.push('dist/server/*.server.js');

    manifest.fxdkWatchCommands.push(['yarn', [
      'webpack', '--mode', 'development', '--watch'
    ]]);
    manifest.fxdkWatchCommands.push(['yarn', [
      'webpack', '--mode', 'production'
    ]]);
    
    const promises = [];
    const relativePath = (to: string) => this.fsService.joinPath(resourcePath, to);



    promises.push(this.fsService.writeFile(relativePath('webpack.config.js'), getWebpackContent()));
    promises.push(this.fsService.writeFile(relativePath('package.json'), getPackageContent(resourceName)));
    

    promises.push(
      this.fsService.mkdirp(relativePath('client'))
        .then(() => concurrently(
          this.fsService.writeFile(relativePath('client/client.ts'), getCode('client')),
          this.fsService.writeFile(relativePath('client/tsconfig.json'), getTsConfigClient())
        )),
    );


    promises.push(
      this.fsService.mkdirp(relativePath('server'))
        .then(() => concurrently(
          this.fsService.writeFile(relativePath('server/server.ts'), getCode('server')),
          this.fsService.writeFile(relativePath('server/tsconfig.json'), getTsConfigServer())
        )),
    );

    await Promise.all(promises);

    await this.installModules(resourcePath);
  };

  private async installModules(path: string) {
    return new Promise<void>((resolve, reject) => {
      cp.exec('yarn', { cwd: path, windowsHide: true }, (err) => {
        if (err) {
          return reject(err);
        }

        return resolve();
      })
    })
  }
}


function getTsConfigClient() {
  return `
{
  "compilerOptions": {
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "ES2018",
    "allowJs": true,
    "lib": ["ES2018"],
    "types": ["@citizenfx/client", "@types/node"],
    "moduleResolution": "node",
    "resolveJsonModule": true,
    "esModuleInterop": true
  },
  "include": ["./**/*"],
  "exclude": ["**/node_modules"]
}
  `
}


function getTsConfigServer() {
  return `
  {
  "compilerOptions": {
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "ES2018",
    "allowJs": false,
    "lib": ["ES2018"],
    "types": ["@citizenfx/server", "@types/node"],
    "moduleResolution": "node",
    "resolveJsonModule": true,
    "esModuleInterop": true
  },
  "include": ["./**/*"],
  "exclude": ["**/node_modules"]
}
  `
}

function getWebpackContent() {
  return `
const webpack = require('webpack');
const path = require('path');
const RemovePlugin = require("remove-files-webpack-plugin");

const buildPath = path.resolve(process.cwd(), 'dist');

const client = {
  entry: './client/client.ts',

  module: {
    rules: [
      {
        test: /\.ts$/,
        use: "ts-loader",
        exclude: /node_modules/,
      },
    ],
  },
  plugins: [
    new RemovePlugin({
      before: {
        include: [path.resolve(buildPath, "client")]
      },
      watch: {
        include: [path.resolve(buildPath, "client")]
      }
    })
  ],
  optimization: {
    minimize: true
  },
  resolve: {
    extensions: [".ts", ".js"]
  },

  output: {
    path: path.resolve(buildPath, "client"),
    filename: "client.js"
  }
}

const server = {
  entry: './server/server.ts',

  module: {
    rules: [
      {
        test: /\.ts$/,
        use: "ts-loader",
        exclude: /node_modules/,
      },
    ],
  },
  plugins: [
    new RemovePlugin({
      before: {
        include: [path.resolve(buildPath, "server")]
      },
      watch: {
        include: [path.resolve(buildPath, "server")]
      }
    })
  ],

  resolve: {
    extensions: [".ts", ".js"]
  },

  output: {
    path: path.resolve(buildPath, "server"),
    filename: "[contenthash].server.js"
  },
  target: "node"
}

module.exports = [client, server];
  `
}

function getPackageContent(resourceName: string) {
  return `
{
  "name": "${resourceName}",
  "version": "1.0.0",
  "main": "index.js",
  "author": "Your name",
  "license": "MIT",
  "scripts": {
    "build": "webpack --mode production",
    "watch": "webpack --mode development --watch"
  },
  "devDependencies": {
    "@citizenfx/client": "^2.0.3602-1",
    "@citizenfx/server": "^2.0.3602-1",
    "@types/node": "^14.14.31",
    "ts-loader": "^8.0.17",
    "webpack": "^5.24.3",
    "webpack-cli": "^4.5.0",
    "remove-files-webpack-plugin": "^1.4.4",
    "typescript": "^4.2.2"
  }
}
  `
}


function getCode (type: string) {
  if (type === 'client') {
    return `
RegisterCommand('helloserver', () => {
  emitNet('helloServer', 'Hi');
}, false);
    `
  } else {
    return `
onNet('helloServer', (message: string) => {
  console.log(message);
});
    `
  }
}
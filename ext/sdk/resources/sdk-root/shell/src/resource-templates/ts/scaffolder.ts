import { inject, injectable } from "inversify";
import { FsService } from "backend/fs/fs-service";
import { concurrently } from "utils/concurrently";
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";

@injectable()
export default class TsScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  async scaffold({ request, manifest, resourcePath }: ResourceTemplateScaffolderArgs) {
    const resourceName = request.assetName;
    
    manifest.clientScripts.push('dist/*.client.js');
    manifest.serverScripts.push('dist/*.server.js');
    
    const promises = [];
    const relativePath = (to: string) => this.fsService.joinPath(resourcePath, to);


    // webpack and package.json
    promises.push(this.fsService.writeFile(relativePath('webpack.config.js'), getWebpackContent()));
    promises.push(this.fsService.writeFile(relativePath('package.json'), getPackageContent(resourceName))); 
    
    // client
    promises.push(
      this.fsService.mkdirp(relativePath('client'))
        .then(() => concurrently(
          this.fsService.writeFile(relativePath('client/client.ts'), ''),
          this.fsService.writeFile(relativePath('client/tsconfig.json'), getTsConfigClient())
        )),
    );

    // server
    promises.push(
      this.fsService.mkdirp(relativePath('server'))
        .then(() => concurrently(
          this.fsService.writeFile(relativePath('server/server.ts'), ''),
          this.fsService.writeFile(relativePath('server/tsconfig.json'), getTsConfigServer())
        )),
    );


    await Promise.all(promises);
  };
}

function getTsConfigClient() {
  return `
{
  "compilerOptions": {
    "baseUrl": ".",
    "paths": {
      "*": ["types/*"]
    },
    "outDir": "./",
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "es6",
    "allowJs": true,
    "lib": ["es2017"],
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
    "baseUrl": ".",
    "paths": {
      "*": ["types/*"]
    },
    "outDir": "./",
    "noImplicitAny": true,
    "module": "commonjs",
    "target": "es6",
    "allowJs": false,
    "lib": ["es2015", "DOM"],
    "types": ["@citizenfx/server", "@types/node"],
    "moduleResolution": "node",
    "emitDecoratorMetadata": true,
    "experimentalDecorators": true,
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
        test: /\.tsx?$/,
        use: "ts-loader",
        exclude: /node_modules/,
      },
    ],
  },
  plugins: [
    new webpack.DefinePlugin({ "global.GENTLY": false }),
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
    extensions: [".tsx", ".ts", ".js"]
  },

  output: {
    path: path.resolve(buildPath, "client"),
    filename: "[contenthash].client.js"
  }
}

const server = {
  entry: './server/server.ts',

  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: "ts-loader",
        exclude: /node_modules/,
      },
    ],
  },
  plugins: [
    new webpack.DefinePlugin({ "global.GENTLY": false }),
    new RemovePlugin({
      before: {
        include: [path.resolve(buildPath, "server")]
      },
      watch: {
        include: [path.resolve(buildPath, "server")]
      }
    })
  ],
  optimization: {
    minimize: true
  },
  resolve: {
    extensions: [".tsx", ".ts", ".js"]
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
    "build": "webpack --mode production --color --progress",
    "watch": "webpack --mode development --progress --watch"
  },
  "devDependencies": {
    "@citizenfx/client": "^2.0.3602-1",
    "@citizenfx/server": "^2.0.3602-1",
    "@types/node": "^14.14.31",
    "ts-loader": "^8.0.17",
    "webpack": "^5.24.3",
    "webpack-cli": "^4.5.0"
  },
  "dependencies": {
    "remove-files-webpack-plugin": "^1.4.4",
    "typescript": "^4.2.2"
  }
}
  `
}
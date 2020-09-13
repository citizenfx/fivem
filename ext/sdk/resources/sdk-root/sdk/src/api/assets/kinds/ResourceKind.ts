import * as fs from 'fs';
import * as path from 'path';
import * as mkdirp from 'mkdirp';
import { AssetCreateRequest, assetKinds, AssetMeta } from "../../api.types";
import { BaseAssetKind } from "../types";

export class ResourceKind extends BaseAssetKind {
  async create(request: AssetCreateRequest): Promise<boolean> {
    if (!this.project.projectInstance) {
      return false;
    }

    this.client.log('Creating resource asset', request);

    const resourcePath = path.join(request.assetPath, request.assetName);
    const assetMeta: AssetMeta = {
      kind: assetKinds.resource,
      flags: {
        readOnly: false,
      },
    };

    await mkdirp(resourcePath);

    const fxmanifestPath = path.join(resourcePath, 'fxmanifest.lua');
    const fxmanifestContent = `
fx_version 'cerulean'
games { 'gta5' }
`.trimStart();

    const { manifest } = this.project.projectInstance.project;

    manifest.resources[request.assetName] = {
      name: request.assetName,
      enabled: true,
      restartOnChange: false,
    };

    await Promise.all([
      this.project.projectInstance.setAssetMeta(resourcePath, assetMeta, { forceReal: true }),
      fs.promises.writeFile(fxmanifestPath, fxmanifestContent),
      this.project.projectInstance.setManifest(manifest),
    ]);

    this.client.log('Finish creating asset', request);

    if (request.callback) {
      request.callback();
    }

    return true;
  }
}

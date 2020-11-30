import os from 'os';
import fs from 'fs';
import path from 'path';
import mkdirp from 'mkdirp';
import cp from 'child_process';
import { rimraf } from 'server/rimraf';
import { featuresStatuses } from "shared/api.statuses";
import { ApiClient, Feature, FeaturesMap } from "shared/api.types";
import { createDeferred, Deferred } from "shared/utils";
import { StatusesApi } from "./StatusesApi";

export class FeaturesApi {
  private defers: Partial<Record<Feature, Deferred<boolean>>> = {};

  constructor(
    private readonly client: ApiClient,
    private readonly statuses: StatusesApi,
  ) {
    this.probeFeatures();
  }

  async get(feature: Feature): Promise<boolean> {
    const featureState = this.state[feature];

    if (typeof featureState === 'boolean') {
      return featureState;
    }

    let featureDefer = this.defers[feature];

    if (!featureDefer) {
      featureDefer = this.defers[feature] = createDeferred();
    }

    return featureDefer.promise;
  }

  private get state(): FeaturesMap {
    return this.statuses.get<FeaturesMap>(featuresStatuses.state) || {};
  }

  private resolveFeature(feature: Feature, featureState: boolean) {
    const state = this.state;

    state[feature] = featureState;

    this.statuses.set(featuresStatuses.state, state);

    const featureDefer = this.defers[feature];
    if (featureDefer) {
      featureDefer.resolve(featureState);
    }

    const msg = `Feature "${Feature[feature]}"? ${featureState}.`;
    console.log(msg);
    this.client.log(msg);
  }

  private async probeFeatures() {
    // Checking windows dev mode enabled by trying to create directory symlink
    {
      let windowsDevModeEnabled: boolean;

      const tmpdir = os.tmpdir();

      const source = path.join(tmpdir, '__fxdk_devmode_feature_probe_source');
      const target = path.join(tmpdir, '__fxdk_devmode_feature_probe_target');


      await Promise.all([
        await rimraf(source),
        await rimraf(target),
      ]);

      await mkdirp(source);

      try {
        await fs.promises.symlink(source, target, 'dir');

        windowsDevModeEnabled = true;
      } catch (e) {
        windowsDevModeEnabled = false;
      }

      this.resolveFeature(Feature.windowsDevModeEnabled, windowsDevModeEnabled);

      await Promise.all([
        await rimraf(source),
        await rimraf(target),
      ]);
    }

    // Checking system git client installed
    {
      await new Promise((resolve) => {
        let systemGitClientAvailable: boolean;

        try {
          const response = cp.execSync('git --version').toString();

          systemGitClientAvailable = response.startsWith('git version');
        } catch (e) {
          systemGitClientAvailable = false;
        }

        this.resolveFeature(Feature.systemGitClientAvailable, systemGitClientAvailable);

        resolve();
      });
    }
  }
}

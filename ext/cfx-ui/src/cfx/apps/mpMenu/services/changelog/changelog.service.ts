import { injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';
import React from 'react';

import { defineService, ServicesContainer } from 'cfx/base/servicesContainer';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { fetcher } from 'cfx/utils/fetcher';
import { html2react } from 'cfx/utils/html2react';

export const IChangelogService = defineService<IChangelogService>('ChangelogService');
export type IChangelogService = ChangelogService;

export function registerChangelogService(container: ServicesContainer) {
  container.registerImpl(IChangelogService, ChangelogService);

  registerAppContribution(container, ChangelogService);
}

const endpoint = 'https://changelogs-live.fivem.net/api/changelog';
const lastSeenVersionsLSKey = 'changelogVersions';

@injectable()
class ChangelogService implements AppContribution {
  private _versionsContent: Record<string, null | React.ReactNode> = {};

  private _versionsContentLoadRequested: Record<string, true> = {};

  private _versions: string[] = [];
  get versions(): string[] {
    return this._versions;
  }
  private set versions(versions: string[]) {
    this._versions = versions;
  }

  private _versionsError: string | null = null;
  public get versionsError(): string | null {
    return this._versionsError;
  }
  private set versionsError(versionsError: string | null) {
    this._versionsError = versionsError;
  }

  public selectedVersion = '';

  public get selectedVersionContent(): React.ReactNode | null {
    if (!this.selectedVersion) {
      return null;
    }

    if (!this._versionsContentLoadRequested[this.selectedVersion]) {
      this._versionsContentLoadRequested[this.selectedVersion] = true;
      this.fetchVersionContent(this.selectedVersion);
    }

    return this._versionsContent[this.selectedVersion];
  }

  public get unreadVersionsCount(): number {
    if (this._lastSeenVersions.size === 0) {
      return 1;
    }

    return this._versions.reduce((acc, version) => acc + (this._lastSeenVersions.has(version)
      ? 0
      : 1), 0);
  }

  private _lastSeenVersions: Set<string> = new Set();

  private _markedNewAsSeen = false;

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error
      _versionsContent: observable.shallow,
      _versionsContentLoadRequested: observable.shallow,
      _lastSeenVersions: observable.struct,
    });
  }

  afterRender() {
    this.fetchVersions();
    this.fetchLastSeenVersions();
  }

  public readonly selectVersion = (version: string) => {
    if (this._versions.includes(version)) {
      this.selectedVersion = version;
    }
  };

  public maybeMarkNewAsSeen() {
    if (this._markedNewAsSeen) {
      return;
    }

    this._markedNewAsSeen = true;

    if (this._versions.length > 0) {
      this.updateLastSeenVersions();
    }
  }

  private fetchLastSeenVersions() {
    try {
      this._lastSeenVersions = new Set(JSON.parse(window.localStorage.getItem(lastSeenVersionsLSKey) || '[]'));
    } catch (e) {
      // noop
    }
  }

  private updateLastSeenVersions() {
    this._lastSeenVersions = new Set(this.versions);
    window.localStorage.setItem(lastSeenVersionsLSKey, JSON.stringify(Array.from(this._lastSeenVersions)));
  }

  private async fetchVersions() {
    try {
      const versions = await fetcher.json(`${endpoint}/versions`);

      if (!Array.isArray(versions)) {
        console.warn('Unexpected versions content', versions);

        return;
      }

      this.versions = versions.map((version) => String(version));
      this.selectVersion(this.versions[0]);

      if (this._markedNewAsSeen) {
        this.updateLastSeenVersions();
      }
    } catch (e) {
      console.error(e);

      this.versionsError = 'Failed to fetch changelog data';
    }
  }

  private async fetchVersionContent(version: string) {
    if (this._versionsContent[version] === null) {
      return;
    }

    try {
      const content = await fetcher.text(`${endpoint}/versions/${version}`);

      this.setVersionContent(version, html2react(content));
    } catch (e) {
      this.setVersionContent(version, `Failed to parse or load change logs for version ${version}`);
    }
  }

  private setVersionContent(version: string, content: null | React.ReactNode) {
    this._versionsContent[version] = content;
  }
}

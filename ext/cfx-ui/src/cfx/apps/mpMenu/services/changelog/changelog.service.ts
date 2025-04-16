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
const MAX_VERSIONS_TO_PRELOAD = 5;
const PRELOAD_BATCH_SIZE = 2;

@injectable()
class ChangelogService implements AppContribution {
  private _versionsContent: Record<string, undefined | null | React.ReactNode> = {};

  private _versionsContentLoadRequested: Record<string, true> = {};
  private _versionsContentLoading: Record<string, boolean> = {};

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

  public getVersionContent(version: string): React.ReactNode | undefined | null {
    if (!version || !this._versions.includes(version)) {
      return null;
    }

    if (this._versionsContentLoadRequested[version] && this._versionsContent[version] === undefined) {
      return undefined;
    }

    if (!this._versionsContentLoadRequested[version]) {
      this._versionsContentLoadRequested[version] = true;
      this.fetchVersionContent(version);
      return undefined;
    }

    return this._versionsContent[version];
  }

  public get selectedVersionContent(): React.ReactNode | undefined | null {
    if (!this.selectedVersion) {
      return null;
    }
    
    return this.getVersionContent(this.selectedVersion);
  }

  public get hasAnyVersions(): boolean {
    return this._versions.length > 0;
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
      _versionsContentLoading: observable.shallow,
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
      
      if (this._versionsContent[version] === undefined) {
        this.fetchVersionContent(version, true);
      }
      
      this.preloadAdjacentVersions(version);
    }
  };

  private preloadAdjacentVersions(currentVersion: string) {
    if (!currentVersion || !this._versions.includes(currentVersion)) {
      return;
    }
    
    const currentIndex = this._versions.indexOf(currentVersion);
    const preloadIndexes: number[] = [];
    
    for (let i = 1; i <= 2; i++) {
      if (currentIndex - i >= 0) {
        preloadIndexes.push(currentIndex - i);
      }
      if (currentIndex + i < this._versions.length) {
        preloadIndexes.push(currentIndex + i);
      }
    }
    
    preloadIndexes.forEach(index => {
      const version = this._versions[index];
      if (!this._versionsContentLoadRequested[version]) {
        this._versionsContentLoadRequested[version] = true;
        this.fetchVersionContent(version);
      }
    });
  }

  public preloadVersionContents(count: number = MAX_VERSIONS_TO_PRELOAD) {
    const versionsToLoad = this._versions.slice(0, count);
    
    const initialVersions = versionsToLoad.slice(0, PRELOAD_BATCH_SIZE);
    initialVersions.forEach(version => {
      if (!this._versionsContentLoadRequested[version]) {
        this._versionsContentLoadRequested[version] = true;
        this.fetchVersionContent(version);
      }
    });
    
    const remainingVersions = versionsToLoad.slice(PRELOAD_BATCH_SIZE);
    
    remainingVersions.forEach((version, index) => {
      const batchIndex = Math.floor(index / PRELOAD_BATCH_SIZE);
      const delay = (batchIndex + 1) * 450;
      
      setTimeout(() => {
        if (!this._versionsContentLoadRequested[version]) {
          this._versionsContentLoadRequested[version] = true;
          this.fetchVersionContent(version);
        }
      }, delay);
    });
  }

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
      
      this.preloadVersionContents();

      if (this._markedNewAsSeen) {
        this.updateLastSeenVersions();
      }
    } catch (e) {
      console.error(e);

      this.versionsError = 'Failed to fetch changelog data';
    }
  }

  private async fetchVersionContent(version: string, priority = false) {
    if (this._versionsContent[version] !== undefined || this._versionsContentLoading[version]) {
      return;
    }
  
  this._versionsContentLoading[version] = true;
  
  try {
    const content = await fetcher.text(`${endpoint}/versions/${version}`);
    
    const dateMatch = content.match(/<h2>Build \d+ \(created on (.*?)\)<\/h2>/i);
    const buildDate = dateMatch ? dateMatch[1] : null;
    
    if (buildDate) {
      this._buildDates = this._buildDates || {};
      this._buildDates[version] = buildDate;
    }
    
    let processedContent = content.replace(/<h2>Build \d+ \(created on .*?\)<\/h2>/i, '').trim();
    processedContent = processedContent.replace(/<h3>Detailed change list<\/h3>/i, '').trim();
    const contentTextOnly = processedContent.replace(/<[^>]*>/g, '').trim();
    
    if (contentTextOnly.length < 10) {
      const customMessage = '<div style="font-style: italic; padding: 4px 0;"><p style="font-size: 0.9em; margin: 0; color: #666;">Psst... this build has some changes, but they\'re our little secret! 🤫</p></div>';
      this.setVersionContent(version, html2react(customMessage));
    } else {
      this.setVersionContent(version, html2react(processedContent));
    }
  } catch (e) {
    console.error('Error fetching version content:', e);
    const errorMessage = '<div style="color: #d64646; padding: 10px;"><p>Failed to load changelog for this build.</p></div>';
    this.setVersionContent(version, html2react(errorMessage));
  } finally {
    this._versionsContentLoading[version] = false;
  }
}
  
  private _buildDates: Record<string, string> = {};
  
  public getBuildDate(version: string): string | null {
    return this._buildDates?.[version] || null;
  }

  private setVersionContent(version: string, content: null | React.ReactNode) {
    this._versionsContent[version] = content;
  }
}

import React from "react";
import { makeAutoObservable, reaction, runInAction } from "mobx";
import { serverApi } from "shared/api.events";
import { GameState } from "store/GameState";
import { ShellPersonality, ShellState } from "store/ShellState";
import { GameStates } from "backend/game/game-constants";
import { apiHost } from "utils/apiHost";
import { LocalStorageValue } from "store/generic/LocalStorageValue";
import { IFxDKGlue } from 'vs/fxdk/browser/glue';
import { ShellEvents } from "shell-api/events";
import { FXCodeImportConfig } from "backend/fxcode/fxcode-types";
import { fxcodeApi } from "backend/fxcode/fxcode-requests";
import { ImportConfigDraft } from "./FXCodeImporter/ImportConfigDraft";
import { NotificationState } from "store/NotificationState";
import { TaskState } from "store/TaskState";
import { ShellCommands } from "shell-api/commands";
import { OpenFlag } from "store/generic/OpenFlag";
import { Api } from "fxdk/browser/Api";
import { Project } from "fxdk/project/browser/state/project";
import { ProjectApi } from "fxdk/project/common/project.api";

const fxcodeRef = React.createRef<HTMLIFrameElement>();

export namespace FXCodeCommands {
  export const OPEN_IMPORTER_DIALOG = 'fxdk.fxcode.importer.openDialog';
  export const IMPORT_ALL = 'fxdk.fxcode.importer.importAll';
  export const IGNORE_IMPORT = 'fxdk.fxcode.importer.ignore';
}

export interface TheiaProject {
  name: string,
  path: string,
  folders: string[],
}

export const FXCodeState = new class FXCodeState {
  private readonly askAboutImport = new LocalStorageValue({
    key: 'fxcode:askAboutImport',
    defaultValue: true,
  });

  public readonly importerDialogState = new OpenFlag(false);

  readonly originUrl = `http://${apiHost.host}:${apiHost.port}`;

  public importConfigDraft: ImportConfigDraft | null = null;

  get ref(): React.RefObject<HTMLIFrameElement> {
    return fxcodeRef;
  }

  get container(): HTMLIFrameElement | null {
    return fxcodeRef.current;
  }

  get iframeSrc(): string {
    return `${this.originUrl}/fxcode-root?path=${Project.path.replace(/\\/g, '/')}`;
  }

  private get glue(): IFxDKGlue | void {
    const glue = (<any>this.container?.contentWindow)?.fxdkGlue;
    if (glue) {
      return glue;
    } else {
      return;
    }
  }

  constructor() {
    makeAutoObservable(this);

    ShellCommands.register(FXCodeCommands.OPEN_IMPORTER_DIALOG, async () => {
      if (!this.importConfigDraft) {
        await this.loadImportConfig();
      }

      this.importerDialogState.open();
    });

    ShellCommands.register(FXCodeCommands.IGNORE_IMPORT, () => {
      this.importConfigDraft = null;
      this.askAboutImport.set(false);
    });

    ShellCommands.register(FXCodeCommands.IMPORT_ALL, this.processImportConfig);

    Api.on(serverApi.bufferedOutput, this.glued((glue, data) => glue.dataService.setBufferedServerOutput(data)));
    Api.on(serverApi.structuredOutputMessage, this.glued((glue, data) => glue.dataService.receiveStructuredServerMessage(data)));
    Api.on(serverApi.clearOutput, this.glued((glue) => glue.dataService.clearAllServerOutputs()));
    Api.on(serverApi.resourceDatas, this.glued((glue, data) => glue.dataService.setServerResourcesData(data)));

    Api.on(ProjectApi.FsEndpoints.entryRenamed, this.glued((glue, { fromEntryPath, toEntryPath }) => glue.emitFileMoved(fromEntryPath, toEntryPath)));
    Api.on(ProjectApi.FsEndpoints.entryDeleted, this.glued((glue, { entryPath }) => glue.emitFileDeleted(entryPath)));

    ShellEvents.on('fxdk:data', this.glued((glue, data) => glue.dataService.acceptData(data)));
    ShellEvents.on('game:consoleMessage', this.glued((glue, data) => glue.dataService.receiveStructuredGameMessage(data)));
    ShellEvents.on('fxdk:clientResourcesData', this.glued((glue, data) => glue.dataService.setClientResourcesData(data)));

    ShellEvents.on('fxcode:ready', () => this.setIsReady(true));
    ShellEvents.on('fxcode:notReady', () => this.setIsReady(false));

    reaction(
      () => GameState.launched,
      (launched) => {
        if (!launched && this.glue) {
          this.glue.dataService.clearGameOutput();
        }
      },
    );

    reaction(
      () => ShellState.personality,
      (shellPersonality) => {
        if (this.isReady) {
          this.setIsActive(shellPersonality === ShellPersonality.MAIN);
        }
      },
    );

    reaction(
      () => GameState.state,
      (gameState: GameStates) => {
        this.setGameState(gameState);

        if (gameState === GameStates.LOADING) {
          this.openGameView();
        }
      },
    );
  }

  public isReady = false;
  setIsReady = (ready: boolean) => {
    this.isReady = ready;

    if (ready) {
      this.maybeImportFromVSC();

      this.setIsActive(ShellState.personality === ShellPersonality.MAIN);
      this.setGameState(GameState.state);
    }
  };

  readonly openFile = this.glued((glue, file: string, pinned = false) => glue.openProjectFile(file, pinned));

  readonly findInFiles = this.glued((glue, entryPath: string) => glue.findInFiles(entryPath));

  readonly openGameView = this.glued((glue) => glue.openGameView());

  readonly setIsActive = this.glued((glue, isActive: boolean) => glue.dataService.setFXCodeIsActive(isActive));

  readonly setGameState = this.glued((glue, gameState: GameStates) => glue.dataService.setGameState(gameState));

  readonly installExtensions = this.glued((glue, ids: string[]) => glue.installExtensions(ids));

  public readonly closeImporter = () => {
    this.importConfigDraft = null;
    this.askAboutImport.set(false);
  };

  public readonly processImportConfig = async () => {
    if (!this.importConfigDraft) {
      return;
    }

    this.askAboutImport.set(false);

    const config = this.importConfigDraft.getConfig();
    const extensionIds = this.importConfigDraft.getExtensionIds();

    this.importConfigDraft = null;

    if (extensionIds.length) {
      TaskState.wrap('Importing VSCode extensions...', async (task) => {
        try {
          await this.installExtensions(extensionIds)
        } catch (e) {
          NotificationState.error(`Failed to import some or all VSCode extensions: ${e}`);
          return
        }

        NotificationState.info('VSCode extensions imported successfully!');
      });
    }

    try {
      await Api.sendPromise(fxcodeApi.applyImportConfig, config);

      NotificationState.info('Imported settings from VSCode!');
    } catch (e) {
      NotificationState.error(`Failed to import settings from VSCode: ${e}`);
    }
  };

  private async maybeImportFromVSC() {
    if (!this.askAboutImport.get()) {
      return;
    }

    await this.loadImportConfig();

    NotificationState.info(`Do you want to import settings and extensions from existing VSCode installation?`, {
      actions: [
        {
          id: 'open',
          label: 'Open import dialog',
          commandId: FXCodeCommands.OPEN_IMPORTER_DIALOG,
        },
        {
          id: 'importAll',
          label: 'Import everything',
          commandId: FXCodeCommands.IMPORT_ALL,
        },
        {
          id: 'ignore',
          label: `Ignore and don't ask again`,
          commandId: FXCodeCommands.IGNORE_IMPORT,
        },
      ],
    });
  }

  private async loadImportConfig() {
    try {
      const importConfig = await Api.sendPromise<FXCodeImportConfig | undefined>(fxcodeApi.getImportConfig);
      if (importConfig) {
        runInAction(() => {
          this.importConfigDraft = new ImportConfigDraft(importConfig);
        });
      }
    } catch (e) {
      console.warn('Failed to load FXCode import config', e);
    }
  }

  private glued<Args extends any[], Ret>(fn: (glue: IFxDKGlue, ...args: Args) => Ret): ((...args: Args) => void | Ret) {
    return (...args: Args) => {
      if (this.glue) {
        return fn(this.glue, ...args);
      }
    };
  }
};

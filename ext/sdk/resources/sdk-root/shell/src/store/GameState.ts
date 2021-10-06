import { GameStates } from "backend/game/game-contants";
import { Api } from "fxdk/browser/Api";
import { makeAutoObservable, runInAction } from "mobx";
import { gameApi } from "shared/api.events";
import { NetLibraryConnectionState, SDKGameProcessState } from "shared/native.enums";
import { ShellEvents } from "shell-api/events";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { GameLoadingState } from "./GameLoadingState";
import { NotificationState } from "./NotificationState";

export const GameState = new class GameState {
  public state = GameStates.NOT_RUNNING;
  public launched = false;
  public processState = SDKGameProcessState.GP_STOPPED;
  public connectionState = NetLibraryConnectionState.CS_IDLE;

  public archetypesCollectionPending = false;
  public archetypesCollectionReady = false;

  constructor() {
    makeAutoObservable(this);

    Api.onWithAction(gameApi.ack, (data) => {
      this.state = data.gameState;
      this.launched = data.gameLaunched;
      this.processState = data.gameProcessState;
      this.connectionState = data.connectionState;
      this.archetypesCollectionReady = data.archetypesCollectionReady;
    });

    Api.onWithAction(gameApi.gameState, (state) => {
      this.state = state;

      if (this.state === GameStates.LOADING) {
        GameLoadingState.resetLoadingProgress();
      }
    });
    Api.onWithAction(gameApi.gameLaunched, (launched) => {
      this.launched = launched;
    });
    Api.onWithAction(gameApi.gameProcessStateChanged, ({ current }) => {
      this.processState = current;
    });
    Api.onWithAction(gameApi.connectionStateChanged, ({ current }) => {
      this.connectionState = current;
    });

    ShellEvents.on('fxdk:loadingScreenWarning', () => {
      const msg = [
        'Loading screen has not been shut down for 15 seconds',
        `This is the reason why you see the "Awaiting scripts" message in bottom right corner of the game-view`,
        '',
        'Check if the "Spawn manager" system resource is enabled in Project settings,',
        `or if using custom spawn manager, check if you're shutting down loading screen properly`,
      ].join('\n');

      NotificationState.warning(msg);
    });

    ShellEvents.on('fxdk:gameFatalError', (error: string) => {
      NotificationState.error(error);
      this.restart();
    });
  }

  readonly restart = () => Api.send(gameApi.restart);

  private archetypesCollectionReadyEvent = new SingleEventEmitter<void>();
  onArchetypeCollectionReady(cb: () => void) {
    this.archetypesCollectionReadyEvent.addListener(cb);
  }

  async refreshArchetypesCollection() {
    this.archetypesCollectionPending = true;
    this.archetypesCollectionReady = false;

    try {
      await Api.sendPromise(gameApi.refreshArchetypesCollection);

      this.archetypesCollectionReadyEvent.emit();
    } catch (e) {
      NotificationState.error(`Failed to obtain object list: ${e}`);
    } finally {
      runInAction(() => {
        this.archetypesCollectionPending = false;
        this.archetypesCollectionReady = true;
      });
    }
  }
}();

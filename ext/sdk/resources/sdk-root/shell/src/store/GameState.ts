import { GameStates } from "backend/game/game-contants";
import { makeAutoObservable, runInAction } from "mobx";
import { gameApi } from "shared/api.events";
import { NetLibraryConnectionState, SDKGameProcessState } from "shared/native.enums";
import { onApiMessage, sendApiMessage, sendApiMessageCallback } from "utils/api";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { onWindowEvent } from "utils/windowMessages";
import { GameLoadingState } from "./GameLoadingState";
import { NotificationState } from "./NotificationState";

export const GameState = new class GameState {
  constructor() {
    makeAutoObservable(this);

    onApiMessage(gameApi.ack, (data) => runInAction(() => {
      this.state = data.gameState;
      this.launched = data.gameLaunched;
      this.processState = data.gameProcessState;
      this.connectionState = data.connectionState;
      this.archetypesCollectionReady = data.archetypesCollectionReady;
    }));

    onApiMessage(gameApi.gameState, this.setState);
    onApiMessage(gameApi.gameLaunched, this.setLaunched);
    onApiMessage(gameApi.gameProcessStateChanged, this.setProcessState);
    onApiMessage(gameApi.connectionStateChanged, this.setConnectionState);

    onWindowEvent('fxdk:loadingScreenWarning', () => {
      const msg = [
        'Loading screen has not been shut down for 15 seconds',
        `This is the reason why you see the "Awaiting scripts" message in bottom right corner of the game-view`,
        '',
        'Check if the "Spawn manager" system resource is enabled in Project settings,',
        `or if using custom spawn manager, check if you're shutting down loading screen properly`,
      ].join('\n');

      NotificationState.warning(msg);
    });

    onWindowEvent('fxdk:gameFatalError', (error: string) => {
      NotificationState.error(error);
      this.restart();
    });
  }

  public ack() {
    sendApiMessage(gameApi.ack);
  }

  public state = GameStates.NOT_RUNNING;
  private setState = (state) => {
    this.state = state;

    if (this.state === GameStates.LOADING) {
      GameLoadingState.resetLoadingProgress();
    }
  };

  public launched = false;
  private setLaunched = (launched) => {
    this.launched = launched;
  };

  public processState = SDKGameProcessState.GP_STOPPED;
  private setProcessState = ({ current }) => {
    this.processState = current;
  };

  public connectionState = NetLibraryConnectionState.CS_IDLE;
  private setConnectionState = ({ current }) => {
    this.connectionState = current;
  }

  readonly restart = () => {
    sendApiMessage(gameApi.restart);
  };

  public archetypesCollectionPending = false;
  public archetypesCollectionReady = false;

  private archetypesCollectionReadyEvent = new SingleEventEmitter<void>();
  onArchetypeCollectionReady(cb: () => void) {
    this.archetypesCollectionReadyEvent.addListener(cb);
  }

  refreshArchetypesCollection() {
    this.archetypesCollectionPending = true;
    this.archetypesCollectionReady = false;

    sendApiMessageCallback<boolean>(gameApi.refreshArchetypesCollection, null, (error, data) => runInAction(() => {
      this.archetypesCollectionPending = false;
      this.archetypesCollectionReady = true;

      if (!data && error) {
        NotificationState.error(`Failed to obtain object list: ${error}`);
      } else {
        this.archetypesCollectionReadyEvent.emit();
      }
    }));
  }
}();

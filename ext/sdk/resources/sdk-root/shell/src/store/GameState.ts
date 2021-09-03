import { GameStates } from "backend/game/game-contants";
import { makeAutoObservable, runInAction } from "mobx";
import { gameApi } from "shared/api.events";
import { NetLibraryConnectionState, SDKGameProcessState } from "shared/native.enums";
import { sendApiMessage, sendApiMessageCallback } from "utils/api";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { onWindowEvent } from "utils/windowMessages";
import { GameLoadingState } from "./GameLoadingState";
import { NotificationState } from "./NotificationState";
import { onApiMessageAction } from "./utils/setterHelpers";

export const GameState = new class GameState {
  public state = GameStates.NOT_RUNNING;
  public launched = false;
  public processState = SDKGameProcessState.GP_STOPPED;
  public connectionState = NetLibraryConnectionState.CS_IDLE;

  public archetypesCollectionPending = false;
  public archetypesCollectionReady = false;

  constructor() {
    makeAutoObservable(this);

    onApiMessageAction(gameApi.ack, (data) => {
      this.state = data.gameState;
      this.launched = data.gameLaunched;
      this.processState = data.gameProcessState;
      this.connectionState = data.connectionState;
      this.archetypesCollectionReady = data.archetypesCollectionReady;
    });

    onApiMessageAction(gameApi.gameState, (state) => {
      this.state = state;

      if (this.state === GameStates.LOADING) {
        GameLoadingState.resetLoadingProgress();
      }
    });
    onApiMessageAction(gameApi.gameLaunched, (launched) => {
      this.launched = launched;
    });
    onApiMessageAction(gameApi.gameProcessStateChanged, ({ current }) => {
      this.processState = current;
    });
    onApiMessageAction(gameApi.connectionStateChanged, ({ current }) => {
      this.connectionState = current;
    });

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

  readonly restart = () => {
    sendApiMessage(gameApi.restart);
  };

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

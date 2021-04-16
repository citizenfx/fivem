import React from "react";
import { makeAutoObservable, reaction } from "mobx";
import { onApiMessage } from "utils/api";
import { serverApi } from "shared/api.events";
import { onWindowEvent, onWindowMessage } from "utils/windowMessages";
import { GameState } from "./GameState";

const theiaRef = React.createRef<HTMLIFrameElement>();

export interface TheiaProject {
  name: string,
  path: string,
  folders: string[],
}

export const TheiaState = new class TheiaState {
  get ref(): React.RefObject<HTMLIFrameElement> {
    return theiaRef;
  }

  get container(): HTMLIFrameElement | null {
    return theiaRef.current;
  }

  constructor() {
    makeAutoObservable(this);

    onApiMessage(serverApi.bufferedOutput, (data: string) => {
      this.sendMessage({ type: 'fxdk:serverOutput', data });
    });

    onApiMessage(serverApi.structuredOutputMessage, (data) => {
      this.sendMessage({ type: 'fxdk:serverOutputStructured', data });
    });

    onApiMessage(serverApi.clearOutput, () => {
      this.sendMessage({ type: 'fxdk:clearServerOutput' });
    });

    onWindowEvent('theia:ready', () => this.setIsReady(true));
    onWindowEvent('theia:notReady', () => this.setIsReady(false));

    onWindowEvent('game:consoleMessage', (data: { channel: string, message: string }) => {
      if (!data.message.trim()) {
        return;
      }

      this.sendMessage({
        type: 'fxdk:gameStructuredMessage',
        data,
      });
    });

    reaction(
      () => GameState.launched,
      (launched) => {
        if (!launched) {
          this.sendMessage({ type: 'fxdk:clearGameOutput' });
        }
      },
    );

    // Also proxying all window messages to theia
    onWindowMessage((data) => this.sendMessage(data));
  }

  public isReady = false;
  setIsReady = (ready: boolean) => this.isReady = ready;

  openProject(theiaProject: TheiaProject) {
    this.sendMessage({
      type: 'fxdk:setProject',
      data: theiaProject,
    });
  }

  openFile(file: string) {
    this.sendMessage({
      type: 'fxdk:openFile',
      data: file,
    });
  }

  openGameView() {
    this.sendMessage({
      type: 'fxdk:openGameView',
    });
  }

  forceReload() {
    this.sendMessage({
      type: 'fxdk:forceReload',
    });
  }

  sendMessage(msg: any) {
    const { container } = this;

    if (container) {
      container.contentWindow?.postMessage(msg, '*');
    }
  }
};

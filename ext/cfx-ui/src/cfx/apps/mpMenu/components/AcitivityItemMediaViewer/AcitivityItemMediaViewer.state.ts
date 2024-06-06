import { makeAutoObservable, observable } from 'mobx';

import { IActivityItemMedia } from 'cfx/common/services/activity/types';
import { IActivityItemContext } from 'cfx/ui/ActivityItem/ActivityItem.context';

export interface IRect {
  x: number;
  y: number;
  w: number;
  h: number;
}

export const AcitivityItemMediaViewerState = new (class AcitivityItemMediaViewer {
  private _active: boolean = false;
  public get active(): boolean {
    return this._active;
  }
  private set active(active: boolean) {
    this._active = active;
  }

  private _media: IActivityItemMedia | null = null;
  public get media(): IActivityItemMedia | null {
    return this._media;
  }
  private set media(media: IActivityItemMedia | null) {
    this._media = media;
  }

  private _fromRect: IRect | null = null;
  public get fromRect(): IRect | null {
    return this._fromRect;
  }
  private set fromRect(fromRect: IRect | null) {
    this._fromRect = fromRect;
  }

  private rAF: RequestAnimationFrameReturn | null = null;

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _media: observable.ref,
    });
  }

  readonly showFull: IActivityItemContext['showFull'] = (media, ref) => {
    this.cancelRAF();

    if (!ref.current) {
      return;
    }

    this.active = true;
    this.media = media;

    const rect = ref.current.getBoundingClientRect();

    this.fromRect = {
      x: rect.x,
      y: rect.y,
      w: rect.width,
      h: rect.height,
    };
  };

  readonly close = () => {
    this.cancelRAF();

    this.active = false;
    this.media = null;
    this.fromRect = null;
  };

  private cancelRAF() {
    if (this.rAF !== null) {
      cancelAnimationFrame(this.rAF);
    }
  }
})();

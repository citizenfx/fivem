import { injectable } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IUiService } from 'cfx/common/services/ui/ui.service';

export function registerMpMenuUiService(container: ServicesContainer) {
  container.registerImpl(IUiService, MpMenuUiService);
}

@injectable()
class MpMenuUiService implements IUiService {
  private _viewportWidth = 0;
  public get viewportWidth(): number {
    return this._viewportWidth;
  }

  private set viewportWidth(viewportWidth: number) {
    this._viewportWidth = viewportWidth;
  }

  private _viewportHeight = 0;
  public get viewportHeight(): number {
    return this._viewportHeight;
  }

  private set viewportHeight(viewportHeight: number) {
    this._viewportHeight = viewportHeight;
  }

  private _quant = 0;
  public get quant(): number {
    return this._quant;
  }

  private set quant(quant: number) {
    this._quant = quant;
  }

  constructor() {
    makeAutoObservable(this);

    this.init();
  }

  private init() {
    // Special node to materilize CSS variables into actual pixel values
    const meterNode = document.createElement('div');
    meterNode.style.width = 'var(--width)';
    meterNode.style.height = 'var(--height)';
    meterNode.style.position = 'fixed';
    meterNode.style.top = '0';
    meterNode.style.left = '0';
    meterNode.style.pointerEvents = 'none';
    meterNode.style.opacity = '0';
    meterNode.style.transform = 'translate(-100%, -100%)';
    document.body.appendChild(meterNode);

    function calculateDimensions(bodyRect: DOMRect) {
      const style = getComputedStyle(meterNode);

      const viewportWidth = parseFloat(style.getPropertyValue('width')) || bodyRect.width;
      const viewportHeight = parseFloat(style.getPropertyValue('height')) || bodyRect.height;
      const quant = viewportHeight * 0.0055;

      return {
        viewportWidth,
        viewportHeight,
        quant,
      };
    }

    const resizeHandler = () => {
      const rect = window.document.body.getBoundingClientRect();

      ({
        viewportWidth: this.viewportWidth,
        viewportHeight: this.viewportHeight,
        quant: this.quant,
      } = calculateDimensions(rect));
    };

    requestAnimationFrame(resizeHandler);

    window.addEventListener('resize', resizeHandler);
  }
}

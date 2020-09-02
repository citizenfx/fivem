class GameView extends HTMLElement {
  modes = {
    observing: 0,
    controling: 1,
  };

  mode = this.modes.observing;
  mouseMoveMultiplier = 1.0;

  _plugin;
  _canvas;

  _keysState = [];

  _observingModeActiveKeys = new Set();
  _observingModeActiveMouseButtons = new Set();
  _observingModeMouseLock = false;

  _controlingModeMouseLock = false;

  get isObservingMode() {
    return this.mode === this.modes.observing;
  }

  get isControlingMode() {
    return this.mode === this.modes.controling;
  }

  get _acceptingInput() {
    if (this.isObservingMode) {
      return this._observingModeMouseLock;
    }

    return this._controlingModeMouseLock;
  }

  constructor() {
    super();

    const shadow = this.attachShadow({ mode: 'open' });

    const style = document.createElement('style');
    style.textContent = `
			:host {
				display: block;

				position: relative;

				width: 60vw;
				height: 60vh;
			}

			object,
			canvas {
				position: absolute;
				top: 0;
				left: 0;
				width: 100%;
				height: 100%;
			}

			object {
				z-index: 1;
			}

			canvas {
				z-index: 2;
				opacity: 1;
			}
		`;

    this._canvas = document.createElement('canvas');

    this._plugin = document.createElement('object');
    this._plugin.type = 'application/x-cfx-sdk-view';

    this._setupResizeObserver();
    this._setupEvents();

    shadow.appendChild(style);
    shadow.appendChild(this._plugin);
    shadow.appendChild(this._canvas);
  }

  disconnectedCallback() {
    this.removeEventListener('mousedown', this._handleMousedown);
    this.removeEventListener('mouseup', this._handleMouseup);

    this.removeEventListener('mousemove', this._handleMousemove);
    this.removeEventListener('mousewheel', this._handleMousewheel);

    document.removeEventListener('keydown', this._handleKeydown);
    document.removeEventListener('keyup', this._handleKeyup);

    document.removeEventListener('pointerlockchange', this._handlePointerLock);
  }

  _handlePointerLock = () => {
    if (!document.pointerLockElement) {
      this._controlingModeMouseLock = false;
    }
  };

  _handleDocumentMouseMove = (e) => {
    // Handling cases when pointer was unlocked externally
    if (this._controlingModeMouseLock && e.target !== this) {
      document.exitPointerLock();
    }
  };

  _mapMouseButton(button) {
    if (button === 2) {
      return 1;
    }

    if (button == 1) {
      return 2;
    }

    return button;
  }

  _mapKey(which, location) {
    // Alt
    if (which === 18) {
      return location === 1
        ? 0xA4
        : 0xA5;
    }

    // Ctrl
    if (which === 17) {
      return location === 1
        ? 0xA2
        : 0xA3;
    }

    // Shift
    if (which === 16) {
      return location === 1
        ? 0xA0
        : 0xA1;
    }

    return which;
  }

  _setupResizeObserver() {
    let width = 0
    let height = 0;

    let debounce;

    const resizeObserver = new ResizeObserver(entries => {
      for (const entry of entries) {
        width = entry.contentRect.width;
        height = entry.contentRect.height;

        if (debounce) {
          clearTimeout(debounce);
        }

        debounce = setTimeout(() => resizeGame(width, height), 250);
      }
    });

    resizeObserver.observe(this._plugin);
  }

  _handleMousedown = (e) => {
    e.preventDefault();

    if (this.isObservingMode) {
      if (e.button === 0) {
        this._observingModeMouseLock = true;
      }
    } else {
      if (e.button === 0) {
        if (!this._controlingModeMouseLock) {
          this._controlingModeMouseLock = true;
          this.requestPointerLock();
          return;
        }
      }

      setMouseButtonState(this._mapMouseButton(e.button), true);
    }
  };
  _handleMouseup = (e) => {
    e.preventDefault();

    if (this.isObservingMode) {
      if (e.button === 0) {
        this._observingModeMouseLock = false;

        for (const activeKey of this._observingModeActiveKeys) {
          setKeyState(activeKey, false);
        }

        for (const activeMouseButton of this._observingModeActiveMouseButtons) {
          setMouseButtonState(activeMouseButton, false);
        }
      }
    } else {
      setMouseButtonState(this._mapMouseButton(e.button), false);
    }
  };
  _handleMousewheel = (e) => {
    e.preventDefault();

    if (this._acceptingInput) {
      sendMouseWheel(e.deltaY);
    }
  };
  _handleMousemove = (e) => {
    e.preventDefault();

    if (this._acceptingInput) {
      sendMousePos(e.movementX, e.movementY);
    }
  };

  _handleKeydown = (e) => {
    e.preventDefault();

    // Handling mouse unlock
    if (e.key === 'Escape' && e.shiftKey) {
      if (this._controlingModeMouseLock) {
        document.exitPointerLock();
        return;
      }
    }

    const vk = this._mapKey(e.which, e.location);

    // Don't spam
    if (this._keysState[vk]) {
      return;
    }

    if (this._acceptingInput) {
      this._keysState[vk] = true;
      setKeyState(vk, true);

      if (this.isObservingMode) {
        this._observingModeActiveKeys.add(vk);
      }
    }
  };
  _handleKeyup = (e) => {
    e.preventDefault();

    const vk = this._mapKey(e.which, e.location);

    if (this._acceptingInput) {
      this._keysState[vk] = false;
      setKeyState(vk, false);

      if (this.isObservingMode) {
        this._observingModeActiveKeys.delete(vk);
      }
    }
  };

  _setupEvents() {
    this.addEventListener('mousedown', this._handleMousedown);
    this.addEventListener('mouseup', this._handleMouseup);

    this.addEventListener('mousemove', this._handleMousemove);
    this.addEventListener('mousewheel', this._handleMousewheel);

    document.addEventListener('keydown', this._handleKeydown);
    document.addEventListener('keyup', this._handleKeyup);

    document.addEventListener('pointerlockchange', this._handlePointerLock);

    document.addEventListener('mousemove', this._handleDocumentMouseMove);
  }
}

window.customElements.define('game-view', GameView);

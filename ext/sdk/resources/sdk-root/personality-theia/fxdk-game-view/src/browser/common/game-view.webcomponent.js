const vertexShaderSrc = `
  attribute vec2 a_position;
  attribute vec2 a_texcoord;
  uniform mat3 u_matrix;
  varying vec2 textureCoordinate;
  void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    textureCoordinate = a_texcoord;
  }
`;

const fragmentShaderSrc = `
varying highp vec2 textureCoordinate;
uniform sampler2D external_texture;
void main()
{
  gl_FragColor = texture2D(external_texture, textureCoordinate);
}
`;

function attachShader(gl, program, type, src) {
  const shader = gl.createShader(type);

  gl.shaderSource(shader, src);
  gl.attachShader(program, shader);

  return shader;
}

function compileAndLinkShaders(gl, program, vs, fs) {
  gl.compileShader(vs);
  gl.compileShader(fs);

  gl.linkProgram(program);

  if (gl.getProgramParameter(program, gl.LINK_STATUS)) {
    return;
  }

  console.error('Link failed:', gl.getProgramInfoLog(program));
  console.error('vs log:', gl.getShaderInfoLog(vs));
  console.error('fs log:', gl.getShaderInfoLog(fs));

  throw new Error('Failed to compile shaders');
}

function createTexture(gl) {
  const tex = gl.createTexture();

  const texPixels = new Uint8Array([0, 0, 255, 255]);

  gl.bindTexture(gl.TEXTURE_2D, tex);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, texPixels);

  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);

  // Magic hook sequence
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.MIRRORED_REPEAT);
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

  // Reset
  gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

  return tex;
}

function createBuffers(gl) {
  const vertexBuff = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
    -1, -1,
    1, -1,
    -1, 1,
    1, 1,
  ]), gl.STATIC_DRAW);

  const texBuff = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
    0, 1,
    1, 1,
    0, 0,
    1, 0,
  ]), gl.STATIC_DRAW);

  return { vertexBuff, texBuff };
}

function createProgram(gl) {
  const program = gl.createProgram();

  const vertexShader = attachShader(gl, program, gl.VERTEX_SHADER, vertexShaderSrc);
  const fragmentShader = attachShader(gl, program, gl.FRAGMENT_SHADER, fragmentShaderSrc);

  compileAndLinkShaders(gl, program, vertexShader, fragmentShader);

  gl.useProgram(program);

  const vloc = gl.getAttribLocation(program, "a_position");
  const tloc = gl.getAttribLocation(program, "a_texcoord");

  return { program, vloc, tloc };
}

function createGameView() {
  const canvas = document.createElement('canvas');
  const gl = canvas.getContext('webgl', {
    antialias: false,
    depth: false,
    alpha: false,
    stencil: false,
    desynchronized: true,
    powerPreference: 'high-performance',
  });

  const gameView = {
    canvas,
    gl,
    animationFrame: void 0,
    resize: (width, height) => {
      window.resizeGame(width, height);
      gl.viewport(0, 0, width, height);
      gl.canvas.width = width;
      gl.canvas.height = height;
    },
    destroy: () => {
      cancelAnimationFrame(this.animationFrame);
    },
  };

  const tex = createTexture(gl);
  const { program, vloc, tloc } = createProgram(gl);
  const { vertexBuff, texBuff } = createBuffers(gl);

  gl.useProgram(program);

  gl.bindTexture(gl.TEXTURE_2D, tex);

  gl.uniform1i(gl.getUniformLocation(program, "external_texture"), 0);

  gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
  gl.vertexAttribPointer(vloc, 2, gl.FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(vloc);

  gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
  gl.vertexAttribPointer(tloc, 2, gl.FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(tloc);

  const render = () => {
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    gl.finish();

    gameView.animationFrame = requestAnimationFrame(render);
  };

  render();

  return gameView;
}

function mapMouseButton(button) {
  if (button === 2) {
    return 1;
  }

  if (button == 1) {
    return 2;
  }

  return button;
}

function mapKey(which, location) {
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

function isLMB(e) {
  return e.button === 0;
}


class GameView extends HTMLElement {
  get mode() {
    return this._mode;
  }

  set mode(newMode) {
    this._mode = newMode;

    this.dispatchEvent(new CustomEvent('modechange', {
      bubbles: true,
      cancelable: false,
      composed: true,
      detail: {
        mode: this.mode,
      },
    }));
  }

  get isObservingMode() {
    return this.mode === GameView.ModeObserving;
  }

  get isControlingMode() {
    return this.mode === GameView.ModeControling;
  }

  constructor() {
    super();

    this.mouseMoveMultiplier = 1.0;

    this._mode = GameView.ModeControling;

    this._keysState = [];
    this._buttonsState = [];

    this._acceptInput = false;
    this._acceptMouseButtons = false;

    this._pointerLocked = false;
    this._fullscreenActive = false;

    this._observingModeActiveKeys = new Set();
    this._observingModeActiveMouseButtons = new Set();


    const shadow = this.attachShadow({ mode: 'open' });

    const style = document.createElement('style');
    style.textContent = `
			:host {
				display: block;

				position: relative;

				width: 60vw;
				height: 60vh;
      }

      hint {
        display: block;

        position: absolute;
        top: 10vw;
        left: 50%;
        transform: translateX(-50%);

        padding: 5px 15px;

        backdrop-filter: blur(20px);
        background-color: hsla(226, 23%, 11%, .45);

        border: solid 1px hsla(226, 23%, 11%, .1);
        border-radius: 4px;

        color: white;
        font-size: 14px;
        font-family: 'Segoe UI';
        font-weight: 300;
        letter-spacing: 1px;

        pointer-events: none;

        opacity: 0;

        z-index: 2;
      }

      @keyframes hint-animation {
        0% {
          opacity: 0;
        }
        25% {
          opacity: 1;
        }
        75% {
          opacity: 1;
        }
        100% {
          opacity: 0;
        }
      }

      hint.active {
        animation-name: hint-animation;
        animation-duration: 2s;
      }

			canvas {
				position: absolute;
				top: 0;
				left: 0;
				width: 100%;
        height: 100%;

        z-index: 1;
			}
		`;

    this.gameView = createGameView();

    this._canvas = this.gameView.canvas;

    this._hint = document.createElement('hint');
    this._hint.innerHTML = `<strong>Shift+Esc</strong> to release mouse`;

    this._createHandlers();

    shadow.appendChild(style);
    shadow.appendChild(this._canvas);
    shadow.appendChild(this._hint);
  }

  /**
   * @lifecycle
   */
  connectedCallback() {
    this._setupResizeObserver();
    this._addEventListeners();
  }

  /**
   * @lifecycle
   */
  disconnectedCallback() {
    this._resizeObserver.disconnect();
    this._removeEventListeners();
  }

  /**
   * @api
   */
  enterFullscreenControlingMode() {
    this.mode = GameView.ModeControling;

    this.requestPointerLock();
    this.requestFullscreen();
  }

  /**
   * @api
   */
  lockPointer() {
    if (!this.isControlingMode) {
      console.warn('game-view is not in controling mode thus it is impossible to lock pointer');
      return false;
    }

    this.requestPointerLock();
    return true;
  }

  /**
   * @api
   */
  enterFullscreen() {
    if (!this.isControlingMode) {
      console.warn('game-view is not in controling mode thus it is impossible to enter fullscreen');
      return false;
    }

    this.requestFullscreen();
    return true;
  }

  _addEventListeners() {
    this.addEventListener('mousedown', this._handleMousedown);
    this.addEventListener('mouseup', this._handleMouseup);

    this.addEventListener('mousemove', this._handleMousemove);
    this.addEventListener('mousewheel', this._handleMousewheel);

    document.addEventListener('keydown', this._handleKeydown);
    document.addEventListener('keyup', this._handleKeyup);

    document.addEventListener('pointerlockchange', this._handlePointerLockChange);
    document.addEventListener('fullscreenchange', this._handleFullscreenChange);

    document.addEventListener('mousemove', this._handleDocumentMouseMove);
  }

  _removeEventListeners() {
    this.removeEventListener('mousedown', this._handleMousedown);
    this.removeEventListener('mouseup', this._handleMouseup);

    this.removeEventListener('mousemove', this._handleMousemove);
    this.removeEventListener('mousewheel', this._handleMousewheel);

    document.removeEventListener('keydown', this._handleKeydown);
    document.removeEventListener('keyup', this._handleKeyup);

    document.removeEventListener('pointerlockchange', this._handlePointerLockChange);
    document.removeEventListener('fullscreenchange', this._handleFullscreenChange);

    document.removeEventListener('mousemove', this._handleDocumentMouseMove);
  }

  _setupResizeObserver() {
    let width = 0
    let height = 0;

    let debounce;

    this._resizeObserver = new ResizeObserver(entries => {
      for (const entry of entries) {
        width = entry.contentRect.width;
        height = entry.contentRect.height;

        if (debounce) {
          clearTimeout(debounce);
        }

        debounce = setTimeout(() => {
          this.gameView.resize(width, height);
        }, 100);
      }
    });

    this._resizeObserver.observe(this._canvas);
  }

  _resetStates() {
    this._keysState.map((active, key) => {
      if (active) {
        this._keysState[key] = false;
        setKeyState(key, false);
      }
    });

    this._buttonsState.map((active, button) => {
      if (active) {
        this._buttonsState[button] = false;
        setMouseButtonState(button, false);
      }
    });
  }

  _createHandlers() {
    this._handlePointerLockChange = () => {
      const pointerLocked = document.pointerLockElement === this;
      const wasPointerLocked = this._pointerLocked;

      this._pointerLocked = pointerLocked;
      this._acceptInput = pointerLocked;

      if (pointerLocked !== wasPointerLocked) {
        this.dispatchEvent(new CustomEvent('pointerlockchange', {
          bubbles: true,
          cancelable: false,
          composed: true,
          detail: {
            locked: pointerLocked,
          },
        }));

        if (pointerLocked) {
          this._hint.classList.add('active');
        } else {
          this._hint.classList.remove('active');
        }
      }

      if (!pointerLocked && wasPointerLocked) {
        this._resetStates();
      }
    };
    this._handleFullscreenChange = () => {
      const fullscreenActive = document.fullscreenElement === this;

      this._fullscreenActive = fullscreenActive;
    };

    this._handleDocumentMouseMove = (e) => {
      // Handling cases when pointer was unlocked externally
      // Like if you alt-tab from FxDK or something like that
      if (this._pointerLocked && e.target !== this) {
        document.exitPointerLock();
      }
    };

    this._handleMousedown = (e) => {
      const leftMouseButton = isLMB(e);

      // Preventing default behaviour for other mouse buttons
      if (!leftMouseButton) {
        e.preventDefault();
      }

      if (this.isObservingMode && leftMouseButton) {
        this._acceptInput = true;
      }

      if (this.isControlingMode) {
        // Lock mouse pointer to GameView if it's LMB
        if (!this._pointerLocked && leftMouseButton) {
          return this.requestPointerLock();
        }

        // Pass mouse button state to game
        this._buttonsState[e.button] = true;
        setMouseButtonState(mapMouseButton(e.button), true);
      }
    };
    this._handleMouseup = (e) => {
      e.preventDefault();

      const leftMouseButton = isLMB(e);

      if (this.isObservingMode && leftMouseButton) {
        this._acceptInput = false;

        this._resetStates();
      }

      if (this.isControlingMode) {
        // Pass mouse button state to game
        this._buttonsState[e.button] = false;
        setMouseButtonState(mapMouseButton(e.button), false);
      }
    };
    this._handleMousewheel = (e) => {
      e.preventDefault();

      if (this._acceptInput) {
        sendMouseWheel(e.deltaY);
      }
    };
    this._handleMousemove = (e) => {
      e.preventDefault();

      if (this._acceptInput) {
        sendMousePos(e.movementX, e.movementY);
      }
    };

    this._handleKeydown = (e) => {
      if (!this._acceptInput) {
        return;
      }

      e.preventDefault();

      // Handling pointer unlock
      if (e.key === 'Escape' && e.shiftKey) {
        if (this._fullscreenActive) {
          document.exitFullscreen();
        }

        if (this._pointerLocked) {
          document.exitPointerLock();
        }

        return;
      }

      const vk = mapKey(e.which, e.location);

      // Don't spam
      if (this._keysState[vk]) {
        return;
      }

      this._keysState[vk] = true;
      setKeyState(vk, true);
    };
    this._handleKeyup = (e) => {
      if (!this._acceptInput) {
        return;
      }

      e.preventDefault();

      const vk = mapKey(e.which, e.location);

      this._keysState[vk] = false;
      setKeyState(vk, false);
    };
  }
}

/**
 * DEFAULT MODE
 *
 * GameView fully captures all input, locks pointer to itself
 */
GameView.ModeControling = 0;

/**
 * GameView only captures all keyboard input and mouse movements while user holding LeftMouseButton over it
 *
 * No mouse buttons state will be passed to game
 */
GameView.ModeObserving = 1;

window.customElements.define('game-view', GameView);

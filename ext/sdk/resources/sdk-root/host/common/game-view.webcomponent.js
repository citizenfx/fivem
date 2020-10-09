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

function makeShader(gl, type, src) {
  const shader = gl.createShader(type);

  gl.shaderSource(shader, src);
  gl.compileShader(shader);

  const infoLog = gl.getShaderInfoLog(shader);
  if (infoLog) {
    console.error(infoLog);
  }

  return shader;
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
  const vertexShader = makeShader(gl, gl.VERTEX_SHADER, vertexShaderSrc);
  const fragmentShader = makeShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSrc);

  const program = gl.createProgram();

  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);
  gl.linkProgram(program);
  gl.useProgram(program);

  const vloc = gl.getAttribLocation(program, "a_position");
  const tloc = gl.getAttribLocation(program, "a_texcoord");

  return { program, vloc, tloc };
}

function createGameView() {
  const canvas = document.createElement('canvas');
  const gl = canvas.getContext('webgl', { antialiasing: false });

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

class GameView extends HTMLElement {
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

    this.modes = {
      observing: 0,
      controling: 1,
    };

    this.mode = this.modes.observing;
    this.mouseMoveMultiplier = 1.0;

    this._keysState = [];

    this._observingModeActiveKeys = new Set();
    this._observingModeActiveMouseButtons = new Set();
    this._observingModeMouseLock = false;

    this._controlingModeMouseLock = false;
    this._controlingModeFullscreen = false;


    const shadow = this.attachShadow({ mode: 'open' });

    const style = document.createElement('style');
    style.textContent = `
			:host {
				display: block;

				position: relative;

				width: 60vw;
				height: 60vh;
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

    this._createHandlers();
    this._setupResizeObserver();
    this._setupEvents();

    shadow.appendChild(style);
    shadow.appendChild(this._canvas);
  }

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

  disconnectedCallback() {
    cancelAnimationFrame(this.gameView.animationFrame);

    this.removeEventListener('mousedown', this._handleMousedown);
    this.removeEventListener('mouseup', this._handleMouseup);

    this.removeEventListener('mousemove', this._handleMousemove);
    this.removeEventListener('mousewheel', this._handleMousewheel);

    document.removeEventListener('keydown', this._handleKeydown);
    document.removeEventListener('keyup', this._handleKeyup);

    document.removeEventListener('pointerlockchange', this._handlePointerLock);

    document.removeEventListener('mousemove', this._handleDocumentMouseMove);
  }

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

        debounce = setTimeout(() => {
          this.gameView.resize(width, height);
        }, 100);
      }
    });

    resizeObserver.observe(this._canvas);
  }

  enterFullscreenControlingMode() {
    this.mode = this.modes.controling;
    this._controlingModeMouseLock = true;
    this._controlingModeFullscreen = true;
    this.requestPointerLock();
    this.requestFullscreen();
  }

  _changeMode(mode) {
    this.mode = mode;

    this.dispatchEvent(new Event('modechange'));
  }

  _createHandlers() {
    this._handlePointerLock = () => {
      if (!document.pointerLockElement) {
        this._controlingModeMouseLock = false;
      }
    };

    this._handleDocumentMouseMove = (e) => {
      // Handling cases when pointer was unlocked externally
      if (this._controlingModeMouseLock && e.target !== this) {
        document.exitPointerLock();
      }
    };

    this._handleMousedown = (e) => {
      if (e.button !== 0) {
        e.preventDefault();
      }

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
    this._handleMouseup = (e) => {
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
    this._handleMousewheel = (e) => {
      e.preventDefault();

      if (this._acceptingInput) {
        sendMouseWheel(e.deltaY);
      }
    };
    this._handleMousemove = (e) => {
      e.preventDefault();

      if (this._acceptingInput) {
        sendMousePos(e.movementX, e.movementY);
      }
    };

    this._handleKeydown = (e) => {
      if (!this._observingModeMouseLock && !this._controlingModeMouseLock) {
        return;
      }

      e.preventDefault();

      // Handling mouse unlock
      if (e.key === 'Escape' && e.shiftKey) {
        if (this._controlingModeFullscreen) {
          this.mode = this.modes.observing;
          this._controlingModeFullscreen = false;
          document.exitFullscreen();
        }

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
    this._handleKeyup = (e) => {
      if (!this._observingModeMouseLock && !this._controlingModeMouseLock) {
        return;
      }

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
  }
}

window.customElements.define('game-view', GameView);

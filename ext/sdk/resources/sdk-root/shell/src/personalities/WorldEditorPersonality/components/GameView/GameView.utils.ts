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

export class GameViewRenderer {
  private gl: WebGLRenderingContext;
  private animationFrame: number;

  constructor(canvas: HTMLCanvasElement) {
    const gl = canvas.getContext('webgl', {
      antialias: false,
      depth: false,
      alpha: false,
      stencil: false,
      desynchronized: true,
      powerPreference: 'high-performance',
    });

    if (!gl) {
      throw new Error('Failed to acquire webgl context for GameViewRenderer');
    }

    this.gl = gl;

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

    this.render();
  }

  resize(width: number, height: number) {
    window.resizeGame(width, height);
    this.gl.viewport(0, 0, width, height);
    this.gl.canvas.width = width;
    this.gl.canvas.height = height;
  }

  destroy() {
    if (this.animationFrame) {
      cancelAnimationFrame(this.animationFrame);
    }
  }

  private render = () => {
    this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, 4);
    this.gl.finish();

    this.animationFrame = requestAnimationFrame(this.render);
  };
}

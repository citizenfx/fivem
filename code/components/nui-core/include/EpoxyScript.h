#pragma once

#include <string>

static std::string g_epoxyScript = R"(
// Replace type="application/x-cfx-game-view" with improved canvas painting
class CfxGameViewRenderer {
  #gl;
  #texture;
  #animationFrame;

  constructor(canvas) {
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

    this.#gl = gl;

    this.#texture = this.#createTexture(gl);
    const { program, vloc, tloc } = this.#createProgram(gl);
    const { vertexBuff, texBuff } = this.#createBuffers(gl);

    gl.useProgram(program);

    gl.bindTexture(gl.TEXTURE_2D, this.#texture);

    gl.uniform1i(gl.getUniformLocation(program, "external_texture"), 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
    gl.vertexAttribPointer(vloc, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vloc);

    gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
    gl.vertexAttribPointer(tloc, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(tloc);

    this.#render();
  }

  #compileAndLinkShaders(gl, program, vs, fs) {
    gl.compileShader(vs);
    gl.compileShader(fs);

    gl.linkProgram(program);

    if (gl.getProgramParameter(program, gl.LINK_STATUS))
    {
      return;
    }

    console.error('Link failed:', gl.getProgramInfoLog(program));
    console.error('vs log:', gl.getShaderInfoLog(vs));
    console.error('fs log:', gl.getShaderInfoLog(fs));

    throw new Error('Failed to compile shaders');
  }

  #attachShader(gl, program, type, src) {
    const shader = gl.createShader(type);

    gl.shaderSource(shader, src);
    gl.attachShader(program, shader);

    return shader;
  }

  #createProgram(gl) {
    const program = gl.createProgram();

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

    const vertexShader = this.#attachShader(gl, program, gl.VERTEX_SHADER, vertexShaderSrc);
    const fragmentShader = this.#attachShader(gl, program, gl.FRAGMENT_SHADER, fragmentShaderSrc);

    this.#compileAndLinkShaders(gl, program, vertexShader, fragmentShader);

    gl.useProgram(program);

    const vloc = gl.getAttribLocation(program, "a_position");
    const tloc = gl.getAttribLocation(program, "a_texcoord");

    return { program, vloc, tloc };
  }

  #createTexture(gl) {
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

  #createBuffers(gl) {
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

  resize(width, height) {
    this.#gl.viewport(0, 0, width, height);
    this.#gl.canvas.width = width;
    this.#gl.canvas.height = height;
  }

  destroy() {
    if (this.#animationFrame) {
      cancelAnimationFrame(this.#animationFrame);
    }
    this.#texture = null;
  }

  #render = () => {
    const gl = this.#gl;
    if (gl)
    {
      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    }
    this.#animationFrame = requestAnimationFrame(this.#render);
  };
}

let __cfx_game_view = {

ReplaceGameView: function(obj, cb)
{
  // don't replace an object if its marked as being replaced already.
  if (obj.hasAttribute("cfx-game-view-compatibility"))
  {
     return obj;
  }

  const canvas = document.createElement('canvas');
  if (obj.id)
  {
    canvas.id = obj.id;
  }
  if (obj.className)
  {
    canvas.className = obj.className;
  }

  // copy width and height attributes seperately
  if (obj.hasAttribute('width') && parseInt(obj.getAttribute('width')))
  {
    canvas.style.width = obj.getAttribute('width');
  }

  if (obj.hasAttribute('height') && parseInt(obj.getAttribute('height')))
  {
    canvas.style.height = obj.getAttribute('height');
  }

  // Clearly indicate that its been replaced
  canvas.setAttribute("cfx-game-view-compatibility", true);

  Array.from(obj.attributes).forEach(attr => {
    let name = attr.name.toLowerCase()
    if (name !== "type" && name !== "width" && name != "height")
    {
      canvas.setAttribute(attr.name, attr.value);
    }
  });

  if (!canvas.width || !canvas.height || !canvas.style.width || !canvas.style.height)
  {
    const cs = window.getComputedStyle(obj);
    const w = cs.width;
    const h = cs.height;
    if (w && h) 
    {
      canvas.width = w;
      canvas.height = h;
      canvas.style.width = cs.width;
      canvas.style.height = cs.height;
    }
  }

  // replace in DOM
  obj.parentNode && obj.parentNode.replaceChild(canvas, obj);

  // Initalize GameViewRender
  if (cb)
  {
    cb(canvas, obj)
  }
  return canvas;
},

FindLegacyGameView: function(cb) {
  const objects = Array.from(document.querySelectorAll('[type="application/x-cfx-game-view"]'));
  objects.map(obj => this.ReplaceGameView(obj, cb));
},

CreateCanvasRenderer: function(canvas)
{
    const renderer = new CfxGameViewRenderer(canvas);
    const resizeObserver = new ResizeObserver(() => {
      renderer.resize(canvas.clientWidth, canvas.clientHeight);
    });

    resizeObserver.observe(canvas);
    canvas.addEventListener('remove', () => {
      renderer.destroy();
      resizeObserver.disconnect();
    });
}
};

// Account for DX -> GL coordinate conversion.
const targetComparsion = new Float32Array([
    0, 0,
    1, 0,
	0, 1,
    1, 1,
]);

const newArrayData = new Float32Array([
    0, 1,
    1, 1,
    0, 0,
    1, 0,
]);

const originalBufferData = WebGLRenderingContext.prototype.bufferData;
WebGLRenderingContext.prototype.bufferData = function(target, data, usage) {
    if (!(data instanceof Float32Array) || target != 0x8892 /*ARRAY_BUFFER*/ || usage != 0x88E4 /*STATIC_DRAW*/)
    {
		return originalBufferData.call(this, target, data, usage);
	}

    const areBuffersEqual = (data) => {
        if (data.length != targetComparsion.length)
        {
           return false;
        }

		for (let i = 0; i < data.length; i++)
        {            
            if (data[i] != targetComparsion[i]) 
            {
              return false;
            }
		}

		return true;
    }

	if (areBuffersEqual(data))
    {
		return originalBufferData.call(this, target, newArrayData, usage);
    }
	
	return originalBufferData.call(this, target, data, usage);
}

const originalReadPixels = WebGLRenderingContext.prototype.readPixels;
WebGLRenderingContext.prototype.readPixels = function(x, y, width, height, format, type, pixels) {
    const result = originalReadPixels.apply(this, arguments);

    // screenshot-basic/three.js game-view compatability.
    if (x != 0 || y != 0 || width != window.innerWidth || height != window.innerHeight || format != 6408/*GL_RGBA*/
        || type != 5121/*GL_UNSIGNED_BYTE*/ || pixels.length != (width * height * 4 /*RGBA*/))
    {
       return result;
    }

    const framebuffer = this.getParameter(this.FRAMEBUFFER_BINDING);
    if (framebuffer) {
        const rowSize = width * 4;
        const tempRow = new Uint8Array(rowSize);
        for (let row = 0; row < Math.floor(height / 2); row++) {
            const topOffset = row * rowSize;
            const bottomOffset = (height - 1 - row) * rowSize;

            tempRow.set(pixels.subarray(topOffset, topOffset + rowSize));
            pixels.copyWithin(topOffset, bottomOffset, bottomOffset + rowSize);
            pixels.set(tempRow, bottomOffset);
        }
    }

    return result;
};
)";

/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>


///
/// "epoxy" is a set of compatability scripts intended to maintain backwards compatability for behaviour/functions/features 
/// that have changed as a result of an newer CEF Version. These scripts are split up into two categorys
/// 1) Changes that only effect "main" iframes (e.g. NUI iframes)
/// 2) Changes that should effect all iframes (e.g. NUI iframes that have iframes within them)
/// 

///
/// "epoxy" provides backwards compatability for behaviour/functions/features that have changed as a result of a newer CEF/Chromium version
/// Currently consisting of:
/// GetParentResourceName/window.GetParentResourceName
/// application/x-cfx-game-view
/// 
static std::string g_epoxyScript = R"(
GetParentResourceName = function()
{
  return "%s";
}
window.GetParentResourceName = GetParentResourceName;

// Epoxy handling
registerEpoxyHandler(function(type, data)
{
	switch (type) {
		case "allowRefocus":
			shouldNotifyBlur = true;
			break;
		case "denyRefocus":
            shouldNotifyBlur = false;
            break;
        case "setHandoverData":
            window.nuiHandoverData = data.handoverData;
            break;
	}
});

shouldNotifyBlur = false;

window.addEventListener("blur", (event) => {
  if (shouldNotifyBlur) {
    window.parent.postMessage({ type: "frameBlurred", frameName: GetParentResourceName() }, "*");
  }
});

window.parent.postMessage({ type: "frameLoaded", frameName: GetParentResourceName() }, "*");
)";

static std::string g_gameViewScript = R"(
if (typeof CfxGameViewRenderer == 'undefined')
{
    // Expose a helper class for rendering game-view in NUI
    // based off of FxDK's GameViewRenderer.
class CfxGameViewRenderer {
    #gl;
    #texture;
    #animationFrame;
    #vao;

    constructor(canvas) {
        const gl = canvas.getContext('webgl2', {
            antialias: false,
            depth: false,
            alpha: false,
            stencil: false,
            desynchronized: true,
            powerPreference: 'high-performance',
        });

        if (!gl) {
            throw new Error('Failed to acquire webgl2 context for GameViewRenderer');
        }

        this.#gl = gl;

        this.#texture = this.#createTexture(gl);
        const { program } = this.#createProgram(gl);

        gl.useProgram(program);
        gl.uniform1i(gl.getUniformLocation(program, "external_texture"), 0);

        this.#vao = this.#createBuffers(gl, program);
        gl.bindVertexArray(this.#vao);
        gl.bindTexture(gl.TEXTURE_2D, this.#texture);

        this.#render();
    }

    #compileAndLinkShaders(gl, program, vs, fs) {
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

    #attachShader(gl, program, type, src) {
        const shader = gl.createShader(type);
        gl.shaderSource(shader, src);
        gl.attachShader(program, shader);
        return shader;
    }

    #createProgram(gl) {
        const program = gl.createProgram();

        const vertexShaderSrc = `#version 300 es
            in vec2 a_position;
            in vec2 a_texcoord;
            out vec2 textureCoordinate;
            void main() {
                gl_Position = vec4(a_position, 0.0, 1.0);
                textureCoordinate = a_texcoord;
            }
        `;

        const fragmentShaderSrc = `#version 300 es
            precision highp float;
            in vec2 textureCoordinate;
            uniform sampler2D external_texture;
            out vec4 fragColor;
            void main() {
                fragColor = texture(external_texture, textureCoordinate);
            }
        `;

        const vertexShader = this.#attachShader(gl, program, gl.VERTEX_SHADER, vertexShaderSrc);
        const fragmentShader = this.#attachShader(gl, program, gl.FRAGMENT_SHADER, fragmentShaderSrc);

        this.#compileAndLinkShaders(gl, program, vertexShader, fragmentShader);

        return { program };
    }

    #createTexture(gl) {
        const tex = gl.createTexture();
        const texPixels = new Uint8Array([0, 0, 255, 255]);

        gl.bindTexture(gl.TEXTURE_2D, tex);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, texPixels);

        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);

        // Bind game render to gl
        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CFX_BIND_GAME_VIEW);

        // Reset
        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

        return tex;
    }

    #createBuffers(gl, program) {
        const vao = gl.createVertexArray();
        gl.bindVertexArray(vao);

        const vertexBuff = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuff);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
            -1, -1,
             1, -1,
            -1,  1,
             1,  1,
        ]), gl.STATIC_DRAW);
        const vloc = gl.getAttribLocation(program, "a_position");
        gl.enableVertexAttribArray(vloc);
        gl.vertexAttribPointer(vloc, 2, gl.FLOAT, false, 0, 0);

        const texBuff = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, texBuff);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
            0, 1,
            1, 1,
            0, 0,
            1, 0,
        ]), gl.STATIC_DRAW);
        const tloc = gl.getAttribLocation(program, "a_texcoord");
        gl.enableVertexAttribArray(tloc);
        gl.vertexAttribPointer(tloc, 2, gl.FLOAT, false, 0, 0);

        gl.bindVertexArray(null);
        return vao;
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
        this.#vao = null;
    }

    #render = () => {
        const gl = this.#gl;
        if (gl) {
            gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        }
        this.#animationFrame = requestAnimationFrame(this.#render);
    };
}
    window.CfxGameViewRenderer = CfxGameViewRenderer;
}

(function(){
var __cfx_game_view = {
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

  // replace in DOM, For compatability reasons it may be better to make the canvas a child of the object in the future.
  obj.parentNode && obj.parentNode.replaceChild(canvas, obj);
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

// Provide backwards compatability for 'application/x-cfx-game-view' mime type.
// Originally implemented through the now removed PepperPlugins.
document.addEventListener('DOMContentLoaded', () => {
	const __cfx_game_view_observer = new MutationObserver(() => {
		const node = document.querySelector(
			'[type="application/x-cfx-game-view"]'
		);

		if (node) {
		  __cfx_game_view.ReplaceGameView(node, __cfx_game_view.CreateCanvasRenderer);
		}
	});

	__cfx_game_view_observer.observe(document.documentElement, {
		childList: true,
		subtree: true
	});

	// Replace all legacy canvas's at startup.
	__cfx_game_view.FindLegacyGameView(__cfx_game_view.CreateCanvasRenderer);
});
})();
)";

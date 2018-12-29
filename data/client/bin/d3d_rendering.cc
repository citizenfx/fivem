// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array.h"
#include "ppapi/lib/gl/gles2/gl2ext_ppapi.h"
#include "ppapi/utility/completion_callback_factory.h"

#ifdef WIN32
#undef PostMessage
// Allow 'this' in initializer list
#pragma warning(disable : 4355)
#endif

namespace {

GLuint CompileShader(GLenum type, const char* data) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &data, NULL);
  glCompileShader(shader);

  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status != GL_TRUE) {
    // Shader failed to compile, let's see what the error is.
    char buffer[1024] = {0};
    GLsizei length;
    glGetShaderInfoLog(shader, sizeof(buffer), &length, &buffer[0]);
    OutputDebugStringA("Shader failed to compile:");
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
    return 0;
  }

  return shader;
}

GLuint LinkProgram(GLuint frag_shader, GLuint vert_shader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, frag_shader);
  glAttachShader(program, vert_shader);
  glLinkProgram(program);

  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE) {
    // Program failed to link, let's see what the error is.
    char buffer[1024];
    GLsizei length;
    glGetProgramInfoLog(program, sizeof(buffer), &length, &buffer[0]);
    OutputDebugStringA("Program failed to link:");
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
    return 0;
  }

  return program;
}

const char kFragShaderSource[] =
    "varying highp vec2 textureCoordinate;\n"
	"uniform sampler2D external_texture;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(external_texture, textureCoordinate); \n"
	"}\n";

const char kVertexShaderSource[] =
    "attribute vec2 a_position;\n"
    "attribute vec2 a_texcoord;\n"
    "varying vec2 textureCoordinate;\n"
    "void main() {\n"
    "  gl_Position = vec4(a_position, 0.0, 1.0);\n"
    "  textureCoordinate = a_texcoord;\n"
    "}\n";
}  // namespace


class Graphics3DInstance : public pp::Instance {
 public:
  explicit Graphics3DInstance(PP_Instance instance)
      : pp::Instance(instance),
        callback_factory_(this),
        width_(0),
        height_(0),
        frag_shader_(0),
        vertex_shader_(0),
        program_(0),
        texture_loc_(0),
        position_loc_(0),
        color_loc_(0),
        mvp_loc_(0),
        x_angle_(0),
        y_angle_(0),
        animating_(true) {}

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    return true;
  }

  virtual void DidChangeView(const pp::View& view) {
    // Pepper specifies dimensions in DIPs (device-independent pixels). To
    // generate a context that is at device-pixel resolution on HiDPI devices,
    // scale the dimensions by view.GetDeviceScale().
    int32_t new_width = view.GetRect().width() * view.GetDeviceScale();
    int32_t new_height = view.GetRect().height() * view.GetDeviceScale();

    if (context_.is_null()) {
      if (!InitGL(new_width, new_height)) {
        // failed.
        return;
      }

      InitShaders();
      InitBuffers();
      InitTexture();
      MainLoop(0);
    } else {
      // Resize the buffers to the new size of the module.
      int32_t result = context_.ResizeBuffers(new_width, new_height);
      if (result < 0) {
        fprintf(stderr,
                "Unable to resize buffers to %d x %d!\n",
                new_width,
                new_height);
        return;
      }
    }

    width_ = new_width;
    height_ = new_height;
    glViewport(0, 0, width_, height_);
  }

  virtual void HandleMessage(const pp::Var& message) {
    // A bool message sets whether the cube is animating or not.
    if (message.is_bool()) {
      animating_ = message.AsBool();
      return;
    }

    // An array message sets the current x and y rotation.
    if (!message.is_array()) {
      fprintf(stderr, "Expected array message.\n");
      return;
    }

    pp::VarArray array(message);
    if (array.GetLength() != 2) {
      fprintf(stderr, "Expected array of length 2.\n");
      return;
    }

    pp::Var x_angle_var = array.Get(0);
    if (x_angle_var.is_int()) {
      x_angle_ = x_angle_var.AsInt();
    } else if (x_angle_var.is_double()) {
      x_angle_ = x_angle_var.AsDouble();
    } else {
      fprintf(stderr, "Expected value to be an int or double.\n");
    }

    pp::Var y_angle_var = array.Get(1);
    if (y_angle_var.is_int()) {
      y_angle_ = y_angle_var.AsInt();
    } else if (y_angle_var.is_double()) {
      y_angle_ = y_angle_var.AsDouble();
    } else {
      fprintf(stderr, "Expected value to be an int or double.\n");
    }
  }

 private:
  bool InitGL(int32_t new_width, int32_t new_height) {
    if (!glInitializePPAPI(pp::Module::Get()->get_browser_interface())) {
      fprintf(stderr, "Unable to initialize GL PPAPI!\n");
      return false;
    }

    const int32_t attrib_list[] = {
      PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
      PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
      PP_GRAPHICS3DATTRIB_WIDTH, new_width,
      PP_GRAPHICS3DATTRIB_HEIGHT, new_height,
      PP_GRAPHICS3DATTRIB_NONE
    };

    context_ = pp::Graphics3D(this, attrib_list);
    if (!BindGraphics(context_)) {
      fprintf(stderr, "Unable to bind 3d context!\n");
      context_ = pp::Graphics3D();
      glSetCurrentContextPPAPI(0);
      return false;
    }

    glSetCurrentContextPPAPI(context_.pp_resource());
    return true;
  }

  void InitShaders() {
    frag_shader_ = CompileShader(GL_FRAGMENT_SHADER, kFragShaderSource);
    if (!frag_shader_)
      return;

    vertex_shader_ = CompileShader(GL_VERTEX_SHADER, kVertexShaderSource);
    if (!vertex_shader_)
      return;

    program_ = LinkProgram(frag_shader_, vertex_shader_);
    if (!program_)
      return;

    texture_loc_ = glGetUniformLocation(program_, "u_texture");
    position_loc_ = glGetAttribLocation(program_, "a_position");
    texcoord_loc_ = glGetAttribLocation(program_, "a_texcoord");
    color_loc_ = glGetAttribLocation(program_, "a_color");
    mvp_loc_ = glGetUniformLocation(program_, "u_mvp");
  }

  void InitBuffers() {
  }

  void InitTexture() {
    glGenTextures(1, &texture_);

    glBindTexture(GL_TEXTURE_2D, texture_);
    
    uint8_t blue[] = { 0, 0, 255, 255 };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               blue); // blue

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	// magic hook sequence
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  void Render() {
	glSetCurrentContextPPAPI(context_.pp_resource());
  
    glClearColor(0.5, 0.5, 0.5, 1);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //set what program to use
    static const GLfloat squareVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };

    static const GLfloat textureVertices[] = {
        /*1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f,  1.0f,
        0.0f,  0.0f,*/
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    glUseProgram(program_);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture_);

    glUniform1i(glGetUniformLocation(program_, "external_texture"), 0);

    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, squareVertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void MainLoop(int32_t) {
    Render();
    context_.SwapBuffers(
        callback_factory_.NewCallback(&Graphics3DInstance::MainLoop));
  }

  pp::CompletionCallbackFactory<Graphics3DInstance> callback_factory_;
  pp::Graphics3D context_;
  int32_t width_;
  int32_t height_;
  GLuint frag_shader_;
  GLuint vertex_shader_;
  GLuint program_;
  GLuint vertex_buffer_;
  GLuint index_buffer_;
  GLuint texture_;

  GLuint texture_loc_;
  GLuint position_loc_;
  GLuint texcoord_loc_;
  GLuint color_loc_;
  GLuint mvp_loc_;

  float x_angle_;
  float y_angle_;
  bool animating_;
};

class Graphics3DModule : public pp::Module {
 public:
  Graphics3DModule() : pp::Module() {}
  virtual ~Graphics3DModule() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new Graphics3DInstance(instance);
  }
};

namespace pp {
Module* CreateModule() { return new Graphics3DModule(); }
}  // namespace pp

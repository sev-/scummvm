/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/system.h"
#include "osys_main.h"
#include "common/advxmlparser.h"
#include "backends/platform/iphone/debug.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "backends/graphics/graphics.h"
#include "common/array.h"

namespace {
    
    const OSystem::GraphicsMode glGraphicsModes[] = {
        { "opengl_linear",  _s("OpenGL"),                kGraphicsModeLinear  },
        { "opengl_nearest", _s("OpenGL (No filtering)"), kGraphicsModeNone },
        { nullptr, nullptr, 0 }
    };
    
} // End of anonymous namespace

Common::Array<OSystem::GraphicsMode> *s_supportedGraphicsModes = 0;

static void initGraphicsModes() {
    s_supportedGraphicsModes = new Common::Array<OSystem::GraphicsMode>;
    
    OSystem::GraphicsMode gm;
    Common::ArchiveMemberList files;
    SearchMan.listMatchingMembers(files, "*.shader");
    int index = 1;
    
    // No gl calls can be made since the OpenGL context has not been created yet.
    gm.name = "opengl_nearest";
    gm.id = 0;
    gm.description = "OpenGL";
    s_supportedGraphicsModes->push_back(gm);
    
    for (Common::ArchiveMemberList::iterator i = files.begin(); i != files.end(); ++i) {
        gm.description = gm.name = strdup((*i)->getName().c_str());
        gm.id = index;
        s_supportedGraphicsModes->push_back(gm);
        index++;
        
        warning("Added shader %s", (*i)->getName().c_str());
    }
    
    gm.name = 0;
    gm.description = 0;
    gm.id = 0;
    s_supportedGraphicsModes->push_back(gm);
}

const OSystem::GraphicsMode *OSystem_IPHONE::getSupportedGraphicsModes() const {
    if (!s_supportedGraphicsModes)
        initGraphicsModes();
    
    return &::s_supportedGraphicsModes->front();
}

bool OSystem_IPHONE::parseShader(const Common::String &filename, ShaderInfo &info) {
    // FIXME
    Common::AdvXMLParser shaderParser;
    if (!shaderParser.loadFile(filename)) {
        warning("Could not open file:%s", filename.c_str());
        return false;
    }

    warning("Parsing shader: %s", filename.c_str());
    Common::XMLTree *root, *shader = NULL, *t;

    root = shaderParser.parse();

    for (uint i = 0; i < root->children.size(); ++i) {
        if (root->children[i]->type == Common::XMLTree::kKey &&
            root->children[i]->text == "shader") {
            shader = root->children[i];
        }
    }

    if (!shader) {
        warning("No shader element");
        delete root;
        return false;
    }

    info.vertex = 0;

    for (uint i = 0; i < shader->children.size(); ++i) {
        if (shader->children[i]->type != Common::XMLTree::kKey)
            continue;
        t = shader->children[i];

        if (t->text == "vertex") {
            if (t->children[0]->type != Common::XMLTree::kText) {
                warning("Unexpected key");
                delete root;
                return false;
            }
            const Common::String &src = t->children[0]->text;
            info.vertex = compileShader(src, GL_VERTEX_SHADER);
        } else if (t->text == "fragment") {
            if (t->children[0]->type != Common::XMLTree::kText) {
                warning("Unexpected key");
                delete root;
                return false;
            }
            ShaderPass p;
            bool x = false, y = false;
            p.xScaleMethod = ShaderPass::kNotSet;
            p.yScaleMethod = ShaderPass::kNotSet;
            p.filter = GL_NEAREST;
            if (t->attrs.contains("filter")) {
                const Common::String &value = t->attrs["filter"];
                if (value == "nearest") {
                    p.filter = GL_NEAREST;
                } else if (value == "linear") {
                    p.filter = GL_LINEAR;
                } else {
                    warning("filter must be linear or nearest");
                    delete root;
                    return false;
                }
            }
            if (t->attrs.contains("size")) {
                x = y = true;
                p.xScaleMethod = ShaderPass::kFixed;
                p.yScaleMethod = ShaderPass::kFixed;
                const Common::String &value = t->attrs["size"];
                sscanf(value.c_str(), "%f", &p.xScale);
                p.yScale = p.xScale;
            }
            if (t->attrs.contains("size_x")) {
                if (x) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                x = true;
                p.xScaleMethod = ShaderPass::kFixed;
                const Common::String &value = t->attrs["size_x"];
                sscanf(value.c_str(), "%f", &p.xScale);
            }
            if (t->attrs.contains("size_y")) {
                if (y) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                y = true;
                p.yScaleMethod = ShaderPass::kFixed;
                const Common::String &value = t->attrs["size_y"];
                sscanf(value.c_str(), "%f", &p.yScale);
            }
            if (t->attrs.contains("scale")) {
                if (x || y) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                x = y = true;
                p.xScaleMethod = ShaderPass::kInput;
                p.yScaleMethod = ShaderPass::kInput;
                const Common::String &value = t->attrs["scale"];
                sscanf(value.c_str(), "%f", &p.xScale);
                p.yScale = p.xScale;
            }
            if (t->attrs.contains("scale_x")) {
                if (x) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                x = true;
                p.xScaleMethod = ShaderPass::kInput;
                const Common::String &value = t->attrs["scale_x"];
                sscanf(value.c_str(), "%f", &p.xScale);
            }
            if (t->attrs.contains("scale_y")) {
                if (y) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                y = true;
                p.yScaleMethod = ShaderPass::kInput;
                const Common::String &value = t->attrs["scale_y"];
                sscanf(value.c_str(), "%f", &p.yScale);
            }
            if (t->attrs.contains("outscale")) {
                if (x || y) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                x = y = true;
                p.xScaleMethod = ShaderPass::kOutput;
                p.yScaleMethod = ShaderPass::kOutput;
                const Common::String &value = t->attrs["outscale"];
                sscanf(value.c_str(), "%f", &p.xScale);
                p.yScale = p.xScale;
            }
            if (t->attrs.contains("outscale_x")) {
                if (x) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                x = true;
                p.xScaleMethod = ShaderPass::kOutput;
                const Common::String &value = t->attrs["outscale_x"];
                sscanf(value.c_str(), "%f", &p.xScale);
            }
            if (t->attrs.contains("outscale_y")) {
                if (y) {
                    warning("Conflicting attributes");
                    delete root;
                    return false;
                }
                y = true;
                p.yScaleMethod = ShaderPass::kOutput;
                const Common::String &value = t->attrs["outscale_y"];
                sscanf(value.c_str(), "%f", &p.yScale);
            }
            const Common::String &src = t->children[0]->text;
            p.fragment = compileShader(src, GL_FRAGMENT_SHADER);
            info.passes.push_back(p);
        }

    }

    // The vertex shader may not have been compiled when the pass was parsed.
    // Link the programs now.
    for (uint j = 0; j < info.passes.size(); ++j) {
        ShaderPass &p = info.passes[j];
        p.program = linkShaders(info.vertex, p.fragment);
        GLCALL(p.positionAttributeLoc = glGetAttribLocation(p.program, "vPosition"));
        GLCALL(p.texCoordAttributeLoc = glGetAttribLocation(p.program, "a_TexCoordinate"));
        GLCALL(p.textureLoc = glGetUniformLocation(p.program, "rubyTexture"));
        GLCALL(p.inputSizeLoc = glGetUniformLocation(p.program, "rubyInputSize"));
        GLCALL(p.outputSizeLoc = glGetUniformLocation(p.program, "rubyOutputSize"));
        GLCALL(p.textureSizeLoc = glGetUniformLocation(p.program, "rubyTextureSize"));
        GLCALL(p.frameCountLoc = glGetUniformLocation(p.program, "rubyFrameCount"));
        GLCALL(p.alphaFactorLoc = glGetUniformLocation(p.program, "alphaFactor"));
        GLCALL(p.textureFractLoc = glGetUniformLocation(p.program, "rubyTextureFract"));

        GLCALL(glEnableVertexAttribArray(p.positionAttributeLoc));
        GLCALL(glEnableVertexAttribArray(p.texCoordAttributeLoc));

        // Non-standard but sometimes used
        GLCALL(p.origTextureLoc = glGetUniformLocation(p.program, "rubyOrigTexture"));
        GLCALL(p.origTextureSizeLoc = glGetUniformLocation(p.program, "rubyOrigTextureSize"));
        GLCALL(p.origInputSizeLoc = glGetUniformLocation(p.program, "rubyOrigInputSize"));
    }

    delete root;

    return true;
}

const char *s_defaultVertex = 
"attribute vec4 vPosition;\n"
"attribute vec2 a_TexCoordinate;\n"
"varying vec2 v_TexCoordinate;\n"
"void main() {\n"
"  v_TexCoordinate = a_TexCoordinate;\n"
"  gl_Position = vPosition;\n"
"}\n";

const char *s_defaultFragment =
"uniform sampler2D rubyTexture;\n"
"uniform float alphaFactor;\n"
"varying vec2 v_TexCoordinate;\n"
" "
"void main() {\n"
""
"  gl_FragColor = texture2D(rubyTexture, v_TexCoordinate);\n"
"  gl_FragColor.a *= alphaFactor;\n"
"}\n";

void OSystem_IPHONE::initShaders() {
    if (_shadersInited && 0) {
        _currentShader = &_shaders[_videoContext->graphicsMode];

        _frameCount = 0;
        return;
    }
    _shadersInited = true;
    const char * versionStr = (const char *)glGetString(GL_VERSION);
    Common::String version;
    int majorVersion, minorVersion;
    Common::StringTokenizer st(versionStr);
    // Version number is 3rd token in OpenGL ES
    // and 1st token in OpenGL.
#ifdef USE_GLES
    st.nextToken();
    st.nextToken();
#endif
    version = st.nextToken();
    sscanf(version.c_str(), "%d.%d", &majorVersion, &minorVersion);
    _enableShaders = majorVersion >= 2;

    if (!_enableShaders && 0)
        return;

    ShaderInfo dInfo;
    ShaderPass p;

    // Initialize built-in shader
    dInfo.vertex = compileShader(s_defaultVertex, GL_VERTEX_SHADER);
    p.fragment = compileShader(s_defaultFragment, GL_FRAGMENT_SHADER);
    p.program = linkShaders(dInfo.vertex, p.fragment);
    dInfo.name = "default";
    p.filter = GL_NEAREST;
    p.positionAttributeLoc = glGetAttribLocation(p.program, "vPosition");
    p.texCoordAttributeLoc = glGetAttribLocation(p.program, "a_TexCoordinate");
    p.textureLoc = glGetUniformLocation(p.program, "rubyTexture");
    p.inputSizeLoc = glGetUniformLocation(p.program, "rubyInputSize");
    p.outputSizeLoc = glGetUniformLocation(p.program, "rubyOutputSize");
    p.textureSizeLoc = glGetUniformLocation(p.program, "rubyTextureSize");
    p.alphaFactorLoc = glGetUniformLocation(p.program, "alphaFactor");
    p.textureFractLoc = glGetUniformLocation(p.program, "rubyTextureFract");
    p.origTextureSizeLoc = glGetUniformLocation(p.program, "rubyOrigTextureSize");
    p.origTextureLoc = glGetUniformLocation(p.program, "rubyOrigTexture");
    p.origInputSizeLoc = glGetUniformLocation(p.program, "rubyOrigInputSize");

    GLCALL(glEnableVertexAttribArray(p.positionAttributeLoc));
    GLCALL(glEnableVertexAttribArray(p.texCoordAttributeLoc));

    p.xScaleMethod = ShaderPass::kNotSet;
    p.yScaleMethod = ShaderPass::kNotSet;
    dInfo.passes.push_back(p);

    //_shaders.push_back(dInfo);

    for (uint i = 1; i < s_supportedGraphicsModes->size() - 1; ++i) {
        ShaderInfo info;
        OSystem::GraphicsMode &gm = (*s_supportedGraphicsModes)[i];
        info.name = Common::String(gm.name);
        if (parseShader(info.name, info)) {
            warning("Successfully compiled %s", info.name.c_str());
            _shaders.push_back(info);
        } else {
            warning("%s is not a shader file", info.name.c_str());
        }
    }

    _defaultShader = &_shaders[0];
    _currentShader = &_shaders[_videoContext->graphicsMode];

    _frameCount = 0;
    //_currentShader = &_shaders[0];
}

GLuint OSystem_IPHONE::compileShader(const Common::String &src, GLenum type) {
    int size = src.size();
    const char * source = src.c_str();
    GLuint shader = glCreateShader(type);
    GLCALL(glShaderSource(shader, 1, &source, &size));
    GLCALL(glCompileShader(shader));

    int status;
    GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
    if (!status) {
        char *buffer;
        int length;
        GLCALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));
        buffer = new char[length];
        GLCALL(glGetShaderInfoLog(shader, length, NULL, buffer));
        if (type == GL_VERTEX_SHADER)
            error("Vertex shader compilation failed:\n%s", buffer);
        else
            error("Fragment shader compilation failed:\n%s", buffer);
    }
    return shader;
}

GLuint OSystem_IPHONE::linkShaders(GLuint vertex, GLuint fragment) {
    GLuint program = glCreateProgram();
    if (vertex)
        GLCALL(glAttachShader(program, vertex));
    if (fragment)
        GLCALL(glAttachShader(program, fragment));

    GLCALL(glLinkProgram(program));

    int status;
    GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &status));

    if (!status) {
        char *buffer;
        int length;
        GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
        buffer = new char[length];
        GLCALL(glGetProgramInfoLog(program, length, NULL, buffer));
        error("Shader program link failed:\n%s", buffer);
    }

    return program;
}

// This function performs multipass rendering using shaders.
// TODO: check if extensions are available, test OpenGL ES, clean/split function,
//       generate buffers on gfxmode init, use x and y (instead of implicit 0,0)
void OSystem_IPHONE::drawTexture(Texture *texture, GLshort x, GLshort y, GLshort w, GLshort h, const ShaderInfo *info) {
    float outputw, outputh, inputw, inputh, texw, texh,
    origInputw, origInputh, origTexw, origTexh;
    origInputw = inputw = texture->getWidth();
    origInputh = inputh = texture->getHeight();
    origTexw = texw = texture->getTextureWidth();
    origTexh = texh = texture->getTextureHeight();
    bool implicitPass = false;
    GLuint currentTexture = texture->getName();
    GLuint origTexture = currentTexture;
    GLuint fbo, outputtex;
    
    texture->updateTexture();
    
    for (uint i = 0; i < info->passes.size(); ++i) {
        bool lastPass = (i == info->passes.size() - 1);
        
        const ShaderPass &p = info->passes[i];
        
        switch (p.xScaleMethod) {
            case ShaderPass::kFixed:
                outputw = p.xScale;
                break;
            case ShaderPass::kInput:
                outputw = inputw * p.xScale;
                break;
            case ShaderPass::kOutput:
                outputw = w * p.xScale;
                break;
            case ShaderPass::kNotSet:
                outputw = inputw;
        }
        
        if (lastPass) {
            if (p.xScaleMethod == ShaderPass::kNotSet) {
                outputw = w;
            } else {
                implicitPass = true;
            }
        }
        
        switch (p.yScaleMethod) {
            case ShaderPass::kFixed:
                outputh = p.yScale;
                break;
            case ShaderPass::kInput:
                outputh = inputh * p.yScale;
                break;
            case ShaderPass::kOutput:
                outputh = h * p.yScale;
                break;
            case ShaderPass::kNotSet:
                outputh = inputh;
        }
        // ChecShaderPass::k if last pass
        if (lastPass) {
            if (p.yScaleMethod == ShaderPass::kNotSet) {
                outputh = h;
            } else {
                implicitPass = true;
            }
        }
        
        if (!lastPass || implicitPass) {
            GLCALL(glGenTextures(1, &outputtex));
            GLCALL(glBindTexture(GL_TEXTURE_2D, outputtex));
            GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p.filter));
            GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p.filter));
            GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
            GLCALL(glTexImage2D(
                                GL_TEXTURE_2D, 0, GL_RGB,
                                outputw, outputh,
                                0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
            
            GLCALL(glGenFramebuffersEXT(1, &fbo));
            GLCALL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo));
            GLCALL(glFramebufferTexture2DEXT(
                                             GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                             GL_TEXTURE_2D, outputtex, 0));
            GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
            if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
                error("Framebuffer creation failed with %x", status);
            }
            GLCALL(glPushMatrix());
            GLCALL(glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT));
            GLCALL(glLoadIdentity());
            GLCALL(glClear(GL_COLOR_BUFFER_BIT));
            GLCALL(glViewport(0,0,outputw, outputh));
        }
        GLCALL(glDisable(GL_BLEND));
        
        // Set up current Texture
        GLCALL(glActiveTexture(GL_TEXTURE1));
        GLCALL(glBindTexture(GL_TEXTURE_2D, currentTexture));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p.filter));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p.filter));
        
        // Set up orig Texture
        GLCALL(glActiveTexture(GL_TEXTURE0));
        GLCALL(glBindTexture(GL_TEXTURE_2D, origTexture));
        
        GLCALL(glUseProgram(p.program));
        
        GLCALL(glUniform1i(p.textureLoc, 0));
        //GLCALL(glUniform1i(p.frameCountLoc, _frameCount));
        
        GLCALL(glUniform2f(p.textureSizeLoc, (GLfloat)texw, (GLfloat)texh));
        
        GLCALL(glUniform2f(p.textureFractLoc, (GLfloat)1/texw, (GLfloat)1/texh));
        
        GLCALL(glUniform2f(p.inputSizeLoc, inputw, inputh));
        
        GLCALL(glUniform2f(p.outputSizeLoc, outputw, outputh));
        
        // Use non-standard uniforms
        //GLCALL(glUniform2f(p.origInputSizeLoc, origInputw, origInputh));
        //GLCALL(glUniform2f(p.origTextureSizeLoc, origTexw, origTexh));
        //GLCALL(glUniform1i(p.origTextureLoc, 0));
#if 0
        const GLfloat vertices[] = {
            0, 0,
            outputw, 0,
            0, outputh,
            outputw, outputh
        };
        const GLfloat texCoords[] = {
            0, 0,
            inputw/texw, 0,
            0, inputh/texh,
            inputw/texw, inputh/texh,
        };
#else
        
        int scalingFactor = 1;
        GLfloat dirtyRectLeft, dirtyRectTop, dirtyRectWidth, dirtyRectHeight;
        
        dirtyRectLeft = 0;
        dirtyRectTop = 0;
        dirtyRectWidth = 1;
        dirtyRectHeight = 1;
        
        const GLfloat tex_width = inputw * scalingFactor / (GLfloat) texw;
        const GLfloat tex_height = inputh * scalingFactor / (GLfloat) texh;
        
        GLfloat texRectX = dirtyRectLeft * tex_width;
        GLfloat texRectY = dirtyRectTop * tex_height;
        GLfloat texRectW = dirtyRectWidth * tex_width;
        GLfloat texRectH = dirtyRectHeight * tex_height;
        
        const GLfloat texCoords[] = { texRectX, texRectY, texRectX + texRectW,
            texRectY, texRectX, texRectY + texRectH, texRectX + texRectW,
            texRectY + texRectH, };
        
        GLfloat vX = dirtyRectLeft * 2.0 - 1.0;
        GLfloat vY = dirtyRectTop * (-2.0) + 1.0;
        GLfloat vW = dirtyRectWidth * 2.0;
        GLfloat vH = dirtyRectHeight * 2.0;
        
        const GLfloat vertices[] = { vX, vY, vX + vW, vY, vX, vY - vH, vX + vW, vY
            - vH };
#endif
        
        GLCALL(glVertexAttribPointer(p.positionAttributeLoc, 2, GL_FLOAT, false, 0, vertices));
        GLCALL(glVertexAttribPointer(p.texCoordAttributeLoc, 2, GL_FLOAT, false, 0, texCoords));
        
        GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        
        GLCALL(glUseProgram(0));
        
        inputw = outputw;
        inputh = outputh;
        texw = outputw;
        texh = outputh;
        if (i)
            GLCALL(glDeleteTextures(1, &currentTexture));
        if (!lastPass || implicitPass) {
            GLCALL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
            GLCALL(glDeleteFramebuffersEXT(1, &fbo));
            GLCALL(glPopMatrix());
            GLCALL(glPopAttrib());
        }
        currentTexture = outputtex;
    }
    if (implicitPass) {
        const GLshort vertices[] = {
            0, 0,
            w, 0,
            0, h,
            w, h
        };
        const GLfloat texCoords[] = {
            0, 0,
            inputw/texw, 0,
            0, inputh/texh,
            inputw/texw, inputh/texh,
        };
        GLCALL(glBindTexture(GL_TEXTURE_2D, currentTexture));
        GLCALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
        GLCALL(glVertexPointer(2, GL_SHORT, 0, vertices));
        GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        glDeleteTextures(1, &currentTexture);
    }
    GLCALL(glEnable(GL_BLEND));
    
    _frameCount++;
}

void OSystem_IPHONE::drawTexture(Texture *texture, GLshort x, GLshort y, GLshort w, GLshort h) {
    // First update any potentional changes.
    texture->updateTexture();
    
    // Set the texture.
    GLCALL(glBindTexture(GL_TEXTURE_2D, texture->getName()));
    
    // Calculate the texture rect that will be drawn.
    const GLfloat texWidth = texture->getDrawWidth();
    const GLfloat texHeight = texture->getDrawHeight();
    const GLfloat texcoords[4*2] = {
        0,        0,
        texWidth, 0,
        0,        texHeight,
        texWidth, texHeight
    };
    GLCALL(glTexCoordPointer(2, GL_FLOAT, 0, texcoords));
    
    // Calculate the screen rect where the texture will be drawn.
    const GLfloat vertices[4*2] = {
        x,     y,
        x + w, y,
        x,     y + h,
        x + w, y + h
    };
    GLCALL(glVertexPointer(2, GL_FLOAT, 0, vertices));
    
    // Draw the texture to the screen buffer.
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

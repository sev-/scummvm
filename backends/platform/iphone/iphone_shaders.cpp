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

void OSystem_IPHONE::initGraphicsModes() {
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
    }
    
    gm.name = 0;
    gm.description = 0;
    gm.id = 0;
    s_supportedGraphicsModes->push_back(gm);
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

    if (!_enableShaders)
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

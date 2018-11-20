#include <QDebug>
#include <QFileInfo>

#include "glutils.hpp"

QOpenGLFunctions_4_0_Core* GLUtils::oglFuncs = nullptr;

QString SHADER_PATHS[] = { "/glsl/", "./glsl/", "../glsl/", "../../glsl/", "../../../glsl/" };

QOpenGLFunctions_4_0_Core* GLUtils::glFunc() {
    return oglFuncs;
}

QString GLUtils::getGLVersion() {
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    QString str;
    str += "GL ";
    str += QString::number(major);
    str += ".";
    str += QString::number(minor);

    return str;
}

QString GLUtils::glError() {
    QString msg("");
    GLenum err (glGetError());
    while(err!=GL_NO_ERROR) {
            QString error;
            switch(err) {
                    case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
                    case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
                    case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
                    case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
            }
            msg += "GL_" + error + " ";
            err=glGetError();
    }
    return msg;
}

void GLUtils::obtainOGlFunc(QOpenGLContext* context) {
    if(!oglFuncs) {
        oglFuncs = context->versionFunctions<QOpenGLFunctions_4_0_Core>();
        if(!oglFuncs) {
            qInfo() << "ERROR could not obtain OpenGLFunctions 4.0!";
        } else {
            qInfo() << "OpenGLFunction 4.0 ok";
        }
    }
}

QOpenGLShaderProgram* GLUtils::createShaderProg(QString _vertPath, QString _fragPath, QString _tessCtrlPath, QString _tessEvalPath) {

    // find correct glsl path prefix
    int i;
    for (i = 0; i < 4 && !QFileInfo(SHADER_PATHS[i] + _vertPath).exists(); i++);
    // abort if no shaders were found
    if (i == 4)
    {
        qCritical() << "Couldn't find shaders in";
        for (i = 0; i < 4; i++)
            qCritical() << "  " << (SHADER_PATHS[i] + _vertPath);
        qCritical() << "\n";
        return new QOpenGLShaderProgram();
    }

    QString vertPath = SHADER_PATHS[i] + _vertPath;
    QString fragPath = SHADER_PATHS[i] + _fragPath;
    QString tessCtrlPath = SHADER_PATHS[i] + _tessCtrlPath;
    QString tessEvalPath = SHADER_PATHS[i] + _tessEvalPath;


    qInfo() << GLUtils::getGLVersion() << "Creating shader program with:";

    QOpenGLShaderProgram *shaderProg = new QOpenGLShaderProgram();
    QOpenGLShader *vertShader, *fragShader, *tessCtrlShader, *tessEvalShader;
    // read vertex shader file
    if (QFileInfo(vertPath).exists())
    {
        qInfo() << " VS: " << vertPath;
        vertShader = new QOpenGLShader(QOpenGLShader::Vertex);
        if (vertShader->compileSourceFile(vertPath)) {
            shaderProg->addShader(vertShader);
            //qInfo() << "Vertex shader compiled succesfully.";
        }
        else
            qCritical() << "Could not compile vertex shader:" << vertShader->log();
    }
    else { qCritical() << "Could not open file" << QFileInfo(vertPath).absolutePath().append("/").append(vertPath); }

    // read fragment shader file
    if (QFileInfo(fragPath).exists())
    {
        qInfo() << " FS: " << fragPath;
        fragShader = new QOpenGLShader(QOpenGLShader::Fragment);
        if (fragShader->compileSourceFile(fragPath)) {
            shaderProg->addShader(fragShader);
            //qInfo() << "Fragment shader compiled succesfully.";
        }
        else qCritical() << "Could not compile fragment shader:" << fragShader->log();
    }
    else { qCritical() << "Could not open file" << QFileInfo(fragPath).absolutePath().append("/").append(fragPath); }

    // read tesselation control shader file
    if(_tessCtrlPath != "") {
        qInfo() << " TCS: " << tessCtrlPath;
        if (QFileInfo(tessCtrlPath).exists())
        {
            tessCtrlShader = new QOpenGLShader(QOpenGLShader::TessellationControl);
            if (tessCtrlShader->compileSourceFile(tessCtrlPath)) {
                shaderProg->addShader(tessCtrlShader);
                //qInfo() << "Tesselation Control shader compiled succesfully.";
            }
            else qCritical() << "Could not compile tesselation control shader:" << tessCtrlShader->log();
        }
        else { qCritical() << "Could not open file" << QFileInfo(tessCtrlPath).absolutePath().append("/").append(tessCtrlPath); }
    }

    // read tesselation evaluation shader file
    if(_tessEvalPath != "") {
        qInfo() << " TES: " << tessEvalPath;
        if (QFileInfo(tessEvalPath).exists())
        {
            tessEvalShader = new QOpenGLShader(QOpenGLShader::TessellationEvaluation);
            if (tessEvalShader->compileSourceFile(tessEvalPath)) {
                shaderProg->addShader(tessEvalShader);
                //qInfo() << "Tesselation Evaluation shader compiled succesfully.";
            }
            else qCritical() << "Could not compile tesselation evaluation shader:" << tessEvalShader->log();
        }
        else { qCritical() << "Could not open file" << QFileInfo(tessEvalPath).absolutePath().append("/").append(tessEvalPath); }
    }


    // link the shader program
    if (!shaderProg->link())
        qCritical() << "! Could not link program:" << shaderProg->log() << "\n";
    else
        qInfo() << "Shader program linked: " << shaderProg->log() << "\n";

    return shaderProg;
}

GLUtils::GLUtils()
{
}

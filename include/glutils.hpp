#pragma once

#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>

class GLUtils
{
public:
    static QOpenGLFunctions_4_0_Core* glFunc();

    static QString getGLVersion();
    static QString glError();
    static QOpenGLShaderProgram* createShaderProg(QString vertPath, QString fragPath, QString tessCtrlPath = "", QString tessEvalPath = "");
    static void obtainOGlFunc(QOpenGLContext* context);

private:
    GLUtils();
    static QOpenGLFunctions_4_0_Core* oglFuncs;

};
#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();
  
  glClearColor (0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  createBuffers();
}

void MyGLWidget::paintGL ()
{
  glClear (GL_COLOR_BUFFER_BIT);  // Esborrem el frame-buffer

  // Activem l'Array a pintar 
  glBindVertexArray(VAO);
 
  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, 3);
  
  // Desactivem el VAO
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h)
{
  glViewport (0, 0, w, h);
}

void MyGLWidget::createBuffers ()
{
  glm::vec3 Vertices[3];  // Tres vèrtexs amb X, Y i Z
  Vertices[0] = glm::vec3(-1.0, -1.0, 0.0);
  Vertices[1] = glm::vec3(1.0, -1.0, 0.0);
  Vertices[2] = glm::vec3(0.0, 1.0, 0.0);

  glm::vec3 color[3];
  color[0] = glm::vec3(1.0, 0.0, 0.0);
  color[1] = glm::vec3(0.0, 1.0, 0.0);
  color[2] = glm::vec3(0.0, 0.0, 1.0);
  
  // Creació del Vertex Array Object (VAO) que usarem per pintar
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Creació del buffer amb les dades dels vèrtexs
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex	
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBOcolor);
  glBindBuffer(GL_ARRAY_BUFFER, VBOcolor);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
  // Activem l'atribut que farem servir per vèrtex
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);


  // Desactivem el VAO
  glBindVertexArray(0);
}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/fragshad.frag");
  vs.compileSourceFile("shaders/vertshad.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  colorLoc = glGetAttribLocation (program->programId(), "color");

  varLoc = glGetUniformLocation(program->programId(), "val");
  maxLoc = glGetUniformLocation(program->programId(), "TG");
  scl= 1.f;
  matrix = glm::mat4(1.0f);
  glUniform1f(varLoc, scl);
  glUniformMatrix4fv(maxLoc, 1, GL_FALSE, &matrix[0][0]);

}

void MyGLWidget::keyPressEvent(QKeyEvent *e){
    makeCurrent();
    switch(e->key()){

        case Qt::Key_Up:
            v.y+=0.1f;
            break;
        case Qt::Key_Down:
            v.y -=0.1f;
        break;
        case Qt::Key_Left:
            v.x-=0.1f;
        break;
        case Qt::Key_Right:
            v.x +=0.1f;
        break;

        case Qt::Key_S:
            scl-=0.1f;
        break;
        case Qt::Key_D:
        scl+=0.1f;

        default: e->ignore();
    }

    modelTransform();
    update();
}

void MyGLWidget::modelTransform(){
    glm::mat4 TG(1.0);
    TG = glm::translate(TG, MyGLWidget::v);
    glUniformMatrix4fv(maxLoc, 1, GL_FALSE, &TG[0][0]);
    glUniform1f(varLoc, scl);
}

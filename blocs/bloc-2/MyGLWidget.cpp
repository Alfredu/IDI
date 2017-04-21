#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
  scale = 1.0f;

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
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  createBuffers();
  FOV = (float)M_PI/2.0f;
  ra = 1.0f;
  znear = 0.4f;
  zfar = 3.0f;
  projectTransform();
  OBS = glm::vec3(0, 0.01, 2); VRP = glm::vec3(0,0,0); UP = glm::vec3(0,1,0);
  viewTransform();
}

void MyGLWidget::paintGL () 
{
  // Esborrem el frame-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Carreguem la transformació de model
  modelTransform ();

  // Activem el VAO per a pintar la caseta 
  glBindVertexArray (VAO_Casa);

  // pintem
  glDrawArrays(GL_TRIANGLES, 0, m.faces().size() * 3);

  identityTransform();
  glBindVertexArray(VAO_Terra);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindVertexArray (0);
}

void MyGLWidget::modelTransform () 
{
  // Matriu de transformació de model
  glm::mat4 transform (1.0f);
  transform = glm::scale(transform, glm::vec3(scale));
  transform = glm::rotate(transform, rot, glm::vec3(0,1,0));
  transform = glm::translate(transform, -baseHomer());
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

glm::vec3 MyGLWidget::baseHomer(){
  glm::vec3 base;
  base.x = (minCapsa.x+maxCapsa.x)/2;
  base.y = minCapsa.y;
  base.z = (minCapsa.z + maxCapsa.z)/2;

  return base;
}

void MyGLWidget::identityTransform(){
  glm::mat4 transform(1.0f);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}


void MyGLWidget::resizeGL (int w, int h) 
{
  float raNou = float(w) / float(h);
  if(raNou <1 ){
    FOV = 2 * (atan(tan((float)M_PI/4.0f)/ra));
  }
  ra = raNou;

  projectTransform();
  glViewport(0, 0, w, h);
}

void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_S: { // escalar a més gran
      scale += 0.05;
      break;
    }
    case Qt::Key_D: { // escalar a més petit
      scale -= 0.05;
      break;
    }
    case Qt::Key_R:{
      rot += M_PI/4;
      break;
  }
    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::createBuffers () 
{
  glm::vec3 VerticesTerra[4];

  VerticesTerra[0] = glm::vec3(-1.0,0.0,1.0);
  VerticesTerra[1] = glm::vec3(-1.0,0.0,-1.0);
  VerticesTerra[2] = glm::vec3(1.0,0.0,1.0);
  VerticesTerra[3] = glm::vec3(1.0,0.0,-1.0);
  
  glm::vec3 colorTerra[4];
  for(int i=0;i<4;i++){
    colorTerra[i] = glm::vec3(1.0,0.0,1.0);
  }
  m.load("../../models/Patricio.obj");
  calculaCapsa(this->m);
  // Dades de la caseta
  // Dos VBOs, un amb posició i l'altre amb color

  // Creació del Vertex Array Object per pintar
  glGenVertexArrays(1, &VAO_Casa);
  glBindVertexArray(VAO_Casa);

  glGenBuffers(1, &VBO_CasaPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_CasaPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m.faces().size() *3 *3, m.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBO_CasaCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_CasaCol);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m.faces().size() * 3* 3, m.VBO_matdiff(), GL_STATIC_DRAW);

  // Activem l'atribut colorLoc
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);

  glBindVertexArray (0);
  
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  glGenBuffers(1, &VBO_TerraPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesTerra), VerticesTerra, GL_STATIC_DRAW);

  //VertexLocTerra
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBO_TerraCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraCol);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colorTerra), colorTerra, GL_STATIC_DRAW);

  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);
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
  // Obtenim identificador per a l'atribut “color” del vertex shader
  colorLoc = glGetAttribLocation (program->programId(), "color");
  // Uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");
  //Uniform projection matrix
  projLoc = glGetUniformLocation(program->programId(), "proj");
  //Uniform view matrix
  viewLoc = glGetUniformLocation(program->programId(), "view");
}

void MyGLWidget::viewTransform(){
    glm::mat4 View = glm::lookAt(OBS, VRP, UP);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::projectTransform(){
    glm::mat4 Proj = glm::perspective(FOV, ra, znear, zfar);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::calculaCapsa(Model &model){
  glm::vec3 min, max;
  assert(model.vertices().size() > 3);
  min.x = model.vertices()[0];
  min.y = model.vertices()[1];
  min.z = model.vertices()[2];
  max = min;

  for(int i=0;i<model.vertices().size();i++){

    if(i%3==0){
      if(model.vertices()[i] < min.x){
        min.x = model.vertices()[i];
      }
      if(model.vertices()[i] > max.x ){
        max.x = model.vertices()[i];
      }
    }
    if(i%3==1){
      if(model.vertices()[i] < min.y){
        min.y = model.vertices()[i];
      }
      if(model.vertices()[i] > max.y){
        max.y = model.vertices()[i];
      }

    }
    if(i%3==2){
      if(model.vertices()[i] < min.z){
        min.z = model.vertices()[i];
      }
      if(model.vertices()[i] > max.z){
        max.z = model.vertices()[i];
      }
    }

    minCapsa = min; maxCapsa = max;
  }
}

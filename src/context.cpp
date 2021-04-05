#include "context.h"
#include "image.h"

ContextUPtr Context::Create() {
  auto context = ContextUPtr(new Context());
  if (!context->Init())
    return nullptr;
  return std::move(context);
}

bool Context::Init() {

    float vertices[] = {
     0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    uint32_t indices[] = { // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3, // second triangle
    };
	
    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER,
    GL_STATIC_DRAW, vertices, sizeof(float) * 32);

  //m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 3);
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 6);
    
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER,
    GL_STATIC_DRAW, indices,sizeof(float) * 6);
 
    ShaderPtr vertShader = Shader::CreateFromFile("./shader/texture.vs", GL_VERTEX_SHADER);
    ShaderPtr fragShader = Shader::CreateFromFile("./shader/texture.fs", GL_FRAGMENT_SHADER);
    if (!vertShader || !fragShader)
      return false;
    SPDLOG_INFO("vertex shader id: {}", vertShader->Get());
    SPDLOG_INFO("fragment shader id: {}", fragShader->Get());

     m_program = Program::Create({fragShader, vertShader});
     if (!m_program)
      return false;
     SPDLOG_INFO("program id: {}", m_program->Get());


  glClearColor(0.9f, 0.7f, 0.6f, 0.5f);

  auto image = Image::Load("./image/container.jpg");
  if (!image) 
      return false;
  SPDLOG_INFO("image: {}x{}, {} channels",
      image->GetWidth(), image->GetHeight(), image->GetChannelCount());
  
    //auto image = Image::Create(512, 512);
    //image->SetCheckImage(16, 16);          체크 상자

    m_texture = Texture::CreateFromImage(image.get());
  
    auto image2 = Image::Load("./image/awesomeface.png");
    m_texture2 = Texture::CreateFromImage(image2.get());

  /*
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        image->GetWidth(), image->GetHeight(), 0,
        GL_RGB, GL_UNSIGNED_BYTE, image->GetData());
    */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    m_program->Use();
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex"), 0);
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex2"), 1);

    //0.5배 축소후 z축으로 90도 회전하는 행렬
    auto transform = glm::rotate(
       glm::scale(glm::mat4(1.0f), glm::vec3(0.7f)),
        glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f)
        );
  
    auto transformLoc = glGetUniformLocation(m_program->Get(), "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

  return true;
}

void Context::Render() {
  glClear(GL_COLOR_BUFFER_BIT);

  m_program->Use();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  //m_program->Use();
  //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
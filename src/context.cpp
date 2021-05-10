#include "context.h"
#include "image.h"
#include <imgui.h>

ContextUPtr Context::Create() {
    auto context = ContextUPtr(new Context());
      if (!context->Init())
      return nullptr;
    return std::move(context);
}

// W,S,D 키 
void Context::ProcessInput(GLFWwindow* window) {
    if (!m_cameraControl)
      return;

    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::Reshape(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Context::MouseMove(double x, double y) {
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;    
}

void Context::MouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}

bool Context::Init() {
    m_box = Mesh::CreateBox();

    // m_model = Model::Load("./model/backpack.obj");
    // if (!m_model)
    //     return false;

    m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
    if (!m_simpleProgram)
        return false;

    m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
    if (!m_program)
        return false;

  glClearColor(0.5f, 0.5f, 0.6f, 0.5f);

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

    // m_material.diffuse = Texture::CreateFromImage( Image::Load("./image/container2.png").get());
    // m_material.specular = Texture::CreateFromImage( Image::Load("./image/container2_specular.png").get());
    // m_material.diffuse = Texture::CreateFromImage( Image::CreateSingleColorImage(4, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).get());
    // m_material.specular = Texture::CreateFromImage( Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

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

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    // m_program->Use();   	
    // m_program->SetUniform("tex", 0);
    // m_program->SetUniform("tex2", 1);
    //glUniform1i(glGetUniformLocation(m_program->Get(), "tex"), 0);
    //glUniform1i(glGetUniformLocation(m_program->Get(), "tex2"), 1);
    
    TexturePtr darkGrayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)).get());

    TexturePtr grayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/marble.jpg").get());
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 128.0f;

    m_box1Material = Material::Create();
    m_box1Material->diffuse = Texture::CreateFromImage(Image::Load("./image/container.jpg").get());
    m_box1Material->specular = darkGrayTexture;
    m_box1Material->shininess = 16.0f;

    m_box2Material = Material::Create();
    m_box2Material->diffuse = Texture::CreateFromImage(Image::Load("./image/container2.png").get());
    m_box2Material->specular = Texture::CreateFromImage(Image::Load("./image/container2_specular.png").get());
    m_box2Material->shininess = 64.0f;

    // // x축으로 -55도 회전
    // auto model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // // 카메라는 원점으로부터 z축 방향으로 -3만큼 떨어짐
    // auto view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    // // 종횡비 4:3, 세로화각 45도의 원근 투영
    // auto projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 10.0f);
    // auto transform = projection * view * model;
    // m_program -> SetUniform("transform", transform);
    // auto transformLoc = glGetUniformLocation(m_program->Get(), "transform");
    // glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

     return true;
}

void Context::Render() {
    if (ImGui::Begin("ui window")) {             // Begin 과 End는 상을 이룸
      if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))) {
          glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
      }
      ImGui::Separator();
      ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
      ImGui::DragFloat("camera yaw", &m_cameraYaw, 0.5f);
      ImGui::DragFloat("camera pitch", &m_cameraPitch, 0.5f, -89.0f, 89.0f);
      ImGui::Separator();
      if (ImGui::Button("reset camera")) {
          m_cameraYaw = 0.0f;
          m_cameraPitch = 0.0f;
          m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
      }
      if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen)) {
          ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
          ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
          ImGui::DragFloat2("l.cutoff", glm::value_ptr(m_light.cutoff), 0.5f, 0.0f, 180.0f);
          ImGui::DragFloat("l.distance", &m_light.distance, 0.5f, 0.0f, 3000.0f);    
          ImGui::ColorEdit3("l.ambient", glm::value_ptr(m_light.ambient));
          ImGui::ColorEdit3("l.diffuse", glm::value_ptr(m_light.diffuse));
          ImGui::ColorEdit3("l.specular", glm::value_ptr(m_light.specular));
          ImGui::Checkbox("flash Light Mode", &m_flashLightMode);
      }
 
      // if (ImGui::CollapsingHeader("material", ImGuiTreeNodeFlags_DefaultOpen)) {
      //   ImGui::DragFloat("m.shininess", &m_material.shininess, 1.0f, 1.0f, 256.0f);
      // }
      ImGui::Checkbox("animation", &m_animation);
  }
  ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
  
    m_cameraFront =
      glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
      
    // m_light.position = m_cameraPos;
    // m_light.direction = m_cameraFront;
    
    auto projection = glm::perspective(glm::radians(45.0f),
        (float)m_width / (float)m_height, 0.1f, 30.0f);

    // 좌우로 돌아가는 카메라
    // float angle = (float)glfwGetTime() * glm::pi<float>() * 0.5f;   
    // auto x = sinf(angle) * 10.0f;
    // auto z = cosf(angle) * 10.0f;
    // auto cameraPos = glm::vec3(x, 0.0f, z);
    // auto cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    // auto cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    auto view = glm::lookAt(
      m_cameraPos,
      m_cameraPos + m_cameraFront,
      m_cameraUp);

    // 위와 같은 function
    // auto cameraZ = glm::normalize(cameraPos - cameraTarget);
    // auto cameraX = glm::normalize(glm::cross(cameraUp, cameraZ));
    // auto cameraY = glm::cross(cameraZ, cameraX);

    // auto cameraMat = glm::mat4(
    //   glm::vec4(cameraX, 0.0f),
    //   glm::vec4(cameraY, 0.0f),
    //   glm::vec4(cameraZ, 0.0f),
    //   glm::vec4(cameraPos, 1.0f));

    // auto view = glm::inverse(cameraMat);
    // // auto view = glm::translate(glm::mat4(1.0f),
    //     glm::vec3(0.0f, 0.0f, -3.0f));
   
    //빛의 위치
    // m_program->SetUniform("light.position", m_light.position);
    // m_program->SetUniform("light.ambient", m_light.diffuse);
    // m_program->SetUniform("material.ambient", m_light.diffuse);
    // m_program->SetUniform("transform", projection * view * lightModelTransform);
    // m_program->SetUniform("modelTransform", lightModelTransform);
    // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    glm::vec3 lightPos = m_light.position;
    glm::vec3 lightDir = m_light.direction;
    if (m_flashLightMode) {
        lightPos = m_cameraPos;
        lightDir = m_cameraFront;
    }
    else {
        // after computing projection and view matrix
        auto lightModelTransform =
            glm::translate(glm::mat4(1.0), m_light.position) *
            glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
        m_program->Use();
        m_simpleProgram->Use();
        m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
        m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
        m_box->Draw(m_simpleProgram.get());
    }
   
    m_program->Use();
    m_program->SetUniform("viewPos", m_cameraPos);
    m_program->SetUniform("light.position", lightPos);
    m_program->SetUniform("light.direction", lightDir);
    m_program->SetUniform("light.cutoff", glm::vec2(
      cosf(glm::radians(m_light.cutoff[0])),
      cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_program->SetUniform("light.ambient", m_light.ambient);
    m_program->SetUniform("light.diffuse", m_light.diffuse);
    m_program->SetUniform("light.specular", m_light.specular);
    // m_program->SetUniform("material.diffuse", 0);
    // m_program->SetUniform("material.specular", 1);
    // m_program->SetUniform("material.shininess", m_box1Material->shininess);

    auto modelTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    auto transform = projection * view * modelTransform;
    m_program->SetUniform("transform", transform);
    m_program->SetUniform("modelTransform", modelTransform);
    m_planeMaterial->SetToProgram(m_program.get());
    m_box->Draw(m_program.get());

    modelTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.75f, -4.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    transform = projection * view * modelTransform;
    m_program->SetUniform("transform", transform);
    m_program->SetUniform("modelTransform", modelTransform);
    m_box1Material->SetToProgram(m_program.get());
    m_box->Draw(m_program.get());

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    modelTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.75f, 2.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    transform = projection * view * modelTransform;
    m_program->SetUniform("transform", transform);
    m_program->SetUniform("modelTransform", modelTransform);
    m_box2Material->SetToProgram(m_program.get());
    m_box->Draw(m_program.get());

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(1.0f, 1.0f, 0.5f, 1.0f));
    m_simpleProgram->SetUniform("transform", transform *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.05f, 1.05f, 1.05f)));
    m_box->Draw(m_simpleProgram.get());

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

//     glActiveTexture(GL_TEXTURE0);
//     m_material.diffuse->Bind();
//     glActiveTexture(GL_TEXTURE1);
//     m_material.specular->Bind();

//     auto modelTransform = glm::mat4(1.0f);
//     auto transform = projection * view * modelTransform;
//     m_program->SetUniform("transform", transform);
//     m_program->SetUniform("modelTransform", modelTransform);
//      m_model->Draw(m_program.get());
}
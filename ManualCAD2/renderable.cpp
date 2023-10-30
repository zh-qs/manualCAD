#include "renderable.h"
#include <glad/glad.h>
#include <stdexcept>
#include <string>

namespace ManualCAD
{
    void Renderable::assert_unbound()
    {
        GLint current_vao = -1;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
        if (current_vao != 0)
            throw std::runtime_error("VAO no " + std::to_string(current_vao) + " not unbound!");
    }
}

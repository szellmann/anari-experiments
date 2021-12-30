#pragma once

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <GL/glew.h>
#include <visionaray/gl/handle.h>
#include <visionaray/gl/program.h>
#include <visionaray/gl/shader.h>
#include <asg/asg.h>

#define ASG_SAFE_CALL(X) X

static std::string getExt(const std::string &fileName)
{
    int pos = fileName.rfind('.');
    if (pos == fileName.npos)
        return "";
    return fileName.substr(pos);
}

static void printSceneGraph(ASGObject rootObj, bool verbose = false)
{
    struct UserData {
        int level;
        bool verbose;
    };
    UserData data;
    data.level = 0;
    data.verbose = verbose;

    ASGVisitor visitor = asgCreateVisitor([](ASGVisitor self, ASGObject obj,
                                             void* userData) {

        UserData* data = (UserData*)userData;

        ASGType_t t;
        ASG_SAFE_CALL(asgGetType(obj,&t));

        const char* nameC_Str = "";
        ASG_SAFE_CALL(asgObjectGetName(obj,&nameC_Str));
        std::string name(nameC_Str);
        std::string nameOut;
        if (!name.empty()) {
            nameOut = ", name: ";
            nameOut.append(name);
            nameOut.append(" ");
        }

        std::string indent;
        for (int i=0; i<data->level; ++i) {
            indent.append("  ");
        }

        switch (t) {
            case ASG_TYPE_TRANSFORM: {
                std::cout << indent << "Transform" << nameOut;
                if (data->verbose) {
                    float matrix[12];
                    ASG_SAFE_CALL(asgTransformGetMatrix((ASGTransform)obj,matrix));
                    std::cout << ", matrix: [0]:" << matrix[0];
                    for (int i=1; i<12; ++i) {
                        std::cout << ", [" << i << "]: " << matrix[i];
                    }
                }
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_CAMERA: {
                std::cout << indent << "Camera" << nameOut;
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_SURFACE: {
                std::cout << indent << "Surface" << nameOut;
                std::cout << '\n';
                break;
            }

            case ASG_TYPE_SELECT: {
                std::cout << indent << "Select" << nameOut;
                std::cout << '\n';
                break;
            }

            default: {
                std::cout << indent << "Object" << nameOut << '\n';
                break;
            }
        }

        data->level++;
        ASG_SAFE_CALL(asgVisitorApply(self,obj));
        data->level--;

    },&data,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
    ASG_SAFE_CALL(asgObjectAccept(rootObj,visitor));
}

namespace gl=visionaray::gl;
struct TriangleGeomPipelineGL
{
    gl::buffer  vertex_buffer;
    gl::buffer  index_buffer;
    gl::program prog;
    gl::shader  vert;
    gl::shader  frag;
    GLuint      view_loc;
    GLuint      proj_loc;
    GLuint      vertex_loc;
    uint32_t    numElements;
};

static void createTriangleGeomPipelineGL(TriangleGeomPipelineGL& pipeline)
{
    // Setup edge shaders
    pipeline.vert.reset(glCreateShader(GL_VERTEX_SHADER));
    pipeline.vert.set_source(R"(
        attribute vec3 vertex;

        uniform mat4 view;
        uniform mat4 proj;


        void main(void)
        {
            gl_Position = proj * view * vec4(vertex, 1.0);
        }
        )");
    pipeline.vert.compile();
    if (!pipeline.vert.check_compiled())
        throw std::runtime_error("vert compile failed");

    pipeline.frag.reset(glCreateShader(GL_FRAGMENT_SHADER));
    pipeline.frag.set_source(R"(
        void main(void)
        {
            gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0);
        }
        )");
    pipeline.frag.compile();
    if (!pipeline.frag.check_compiled())
        throw std::runtime_error("frag compile failed");

    pipeline.prog.reset(glCreateProgram());
    pipeline.prog.attach_shader(pipeline.vert);
    pipeline.prog.attach_shader(pipeline.frag);

    pipeline.prog.link();
    if (!pipeline.prog.check_linked())
        throw std::runtime_error("prog link failed");

    pipeline.vertex_loc = glGetAttribLocation(pipeline.prog.get(), "vertex");
    pipeline.view_loc   = glGetUniformLocation(pipeline.prog.get(), "view");
    pipeline.proj_loc   = glGetUniformLocation(pipeline.prog.get(), "proj");

    // Setup vbo
    pipeline.vertex_buffer.reset(gl::create_buffer());
    pipeline.index_buffer.reset(gl::create_buffer());
}

static void updateTriangleGeomPipelineGL(ASGTriangleGeometry geom,
                                         TriangleGeomPipelineGL& pipeline)
                                        
{
    // Store OpenGL state
    GLint array_buffer_binding = 0;
    GLint element_array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_array_buffer_binding);

    float* vertices = NULL;
    uint32_t numVertices = 0;
    uint32_t* indices = NULL;
    uint32_t numIndices = 0;

    ASG_SAFE_CALL(asgTriangleGeometryGetVertices(geom,&vertices));
    ASG_SAFE_CALL(asgTriangleGeometryGetNumVertices(geom,&numVertices));
    ASG_SAFE_CALL(asgTriangleGeometryGetIndices(geom,&indices));
    ASG_SAFE_CALL(asgTriangleGeometryGetNumIndices(geom,&numIndices));

    glBindBuffer(GL_ARRAY_BUFFER, pipeline.vertex_buffer.get());
    glBufferData(GL_ARRAY_BUFFER,
                 numVertices * 3 * sizeof(float),
                 vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pipeline.index_buffer.get());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 numIndices * 3 * sizeof(uint32_t),
                 indices,
                 GL_STATIC_DRAW);

    pipeline.numElements = numIndices;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);

}

static void renderTriangleGeomPipelineGL(TriangleGeomPipelineGL& pipeline,
                                         visionaray::mat4 view, visionaray::mat4 proj)
{
    // Store OpenGL state
    GLint array_buffer_binding = 0;
    GLint element_array_buffer_binding = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_array_buffer_binding);

    pipeline.prog.enable();

    glUniformMatrix4fv(pipeline.view_loc, 1, GL_FALSE, view.data());
    glUniformMatrix4fv(pipeline.proj_loc, 1, GL_FALSE, proj.data());

    glBindBuffer(GL_ARRAY_BUFFER, pipeline.vertex_buffer.get());
    glEnableVertexAttribArray(pipeline.vertex_loc);
    glVertexAttribPointer(pipeline.vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pipeline.index_buffer.get());

    glDrawElements(GL_LINES, pipeline.numElements * 3, GL_UNSIGNED_INT, 0);

    pipeline.prog.disable();

    // Restore OpenGL state
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_binding);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);

}



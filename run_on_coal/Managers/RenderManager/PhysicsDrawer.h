#pragma once

namespace ROC
{

class PhysicsDrawer final : public btIDebugDraw
{
    enum PhysicsDrawerBufferIndex : size_t
    {
        PDBI_Vertex = 0U,
        PDBI_Color,

        PDBI_BufferCount
    };
    enum PhysicsDrawerBufferAttribute : size_t
    {
        PDBA_Vertex = 0U,
        PDBA_Color = 2U
    };
    GLuint m_VBO[PDBI_BufferCount];
    GLuint m_VAO;

    std::vector<glm::vec3> m_lines;
    std::vector<glm::vec3> m_colors;

    PhysicsDrawer(const PhysicsDrawer &that);
    PhysicsDrawer& operator=(const PhysicsDrawer &that);
public:
    PhysicsDrawer();
    ~PhysicsDrawer();

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {};
    virtual void reportErrorWarning(const char* warningString) {};
    virtual void draw3dText(const btVector3& location, const char* textString) {};
    virtual void setDebugMode(int debugMode) {};
    virtual int	getDebugMode() const { return (DBG_DrawWireframe | DBG_DrawConstraints | DBG_DrawConstraintLimits | DBG_DrawNormals); };
protected:
    void Draw(float f_width);

    friend class RenderManager;
};

}

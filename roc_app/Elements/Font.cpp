#include "stdafx.h"

#include "Elements/Font.h"
#include "GL/GLTexture2D.h"
#include "GL/GLArrayBuffer.h"
#include "GL/GLVertexArray.h"
#include "GL/GLSetting.h"

#include "Utils/MathUtils.h"

namespace ROC
{

extern const glm::vec2 g_EmptyVec2;
const int g_FontDefaultAtlasSize = 256;
const float g_FontDefaultAtlasOffset = 1.f / static_cast<float>(g_FontDefaultAtlasSize);
const GLint g_FontSwizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
const size_t g_FontCharacterVerticesCount = 6U;
const size_t g_FontTextBlockSize = 512U;

}

FT_Library ROC::Font::ms_library = FT_Library();

GLArrayBuffer *ROC::Font::ms_arrayBuffers[] = { nullptr };
GLVertexArray *ROC::Font::ms_vertexArray = nullptr;

std::vector<glm::vec3> ROC::Font::ms_vertices;
std::vector<glm::vec2> ROC::Font::ms_uv;

ROC::Font::Font()
{
    m_elementType = ET_Font;

    m_loaded = false;
    m_face = FT_Face();
    m_size = 0.f;

    m_atlasTexture = nullptr;
    m_atlasOffset = g_EmptyVec2;
    m_atlasSize = glm::ivec2(0);
    m_atlasPack = nullptr;

    m_charIter = m_charMap.begin();
    m_charMapEnd = m_charMap.end();

    m_filteringType = FFT_None;
}

ROC::Font::~Font()
{
    if(m_loaded)
    {
        if(m_atlasTexture)
        {
            m_atlasTexture->Destroy();
            delete m_atlasTexture;
        }
        delete m_atlasPack;
        FT_Done_Face(m_face);
    }
}

void ROC::Font::InitStaticResources()
{
    FT_Init_FreeType(&ms_library);

    ms_vertexArray = new GLVertexArray();
    ms_vertexArray->Create();
    ms_vertexArray->Bind();

    for(size_t i = 0U; i < FBI_BufferCount; i++) ms_arrayBuffers[i] = new GLArrayBuffer();

    ms_arrayBuffers[FBI_Vertex]->Create(sizeof(glm::vec3) * g_FontCharacterVerticesCount * g_FontTextBlockSize, nullptr, GL_DYNAMIC_DRAW);
    ms_arrayBuffers[FBI_Vertex]->Bind();
    ms_vertexArray->EnableAttribute(FBA_Vertex, 3, GL_FLOAT);
    ms_vertices.assign(g_FontCharacterVerticesCount * g_FontTextBlockSize, glm::vec3(0.f, 0.f, 1.f));

    ms_arrayBuffers[FBI_UV]->Create(sizeof(glm::vec2) * g_FontCharacterVerticesCount * g_FontTextBlockSize, nullptr, GL_DYNAMIC_DRAW);
    ms_arrayBuffers[FBI_UV]->Bind();
    ms_vertexArray->EnableAttribute(FBA_UV, 2, GL_FLOAT);
    ms_uv.assign(g_FontCharacterVerticesCount * g_FontTextBlockSize, g_EmptyVec2);
}

void ROC::Font::ReleaseStaticResources()
{
    FT_Done_FreeType(ms_library);

    for(size_t i = 0U; i < FBI_BufferCount; i++)
    {
        ms_arrayBuffers[i]->Destroy();
        delete ms_arrayBuffers[i];
    }

    ms_vertexArray->Destroy();
    delete ms_vertexArray;

    ms_vertices.clear();
    ms_uv.clear();
}

bool ROC::Font::Load(const std::string &f_path, int f_size, const glm::ivec2 &f_atlas, unsigned char f_filter)
{
    if(!m_loaded)
    {
        if(!FT_New_Face(ms_library, f_path.c_str(), 0, &m_face))
        {
            FT_Select_Charmap(m_face, ft_encoding_unicode);
            FT_Set_Pixel_Sizes(m_face, 0, f_size);
            m_size = static_cast<float>(f_size);
            m_filteringType = f_filter;
            btClamp<unsigned char>(m_filteringType, FFT_Nearest, FFT_Linear);

            if(MathUtils::IsPowerOfTwo(f_atlas.x))
            {
                m_atlasSize.x = f_atlas.x;
                m_atlasOffset.x = 1.f / static_cast<float>(f_atlas.x);
            }
            else
            {
                m_atlasSize.x = g_FontDefaultAtlasSize;
                m_atlasOffset.x = g_FontDefaultAtlasOffset;
            }
            if(MathUtils::IsPowerOfTwo(f_atlas.y))
            {
                m_atlasSize.y = f_atlas.y;
                m_atlasOffset.y = 1.f / static_cast<float>(f_atlas.y);
            }
            else
            {
                m_atlasSize.y = g_FontDefaultAtlasSize;
                m_atlasOffset.y = g_FontDefaultAtlasOffset;
            }

            // Generate atlas texture
            m_atlasTexture = new GLTexture2D();
            m_atlasTexture->Create(m_atlasSize.x, m_atlasSize.y, GL_RED, GL_RED, nullptr, GL_NEAREST + m_filteringType);
            m_atlasTexture->SetSwizzle(GL_TEXTURE_SWIZZLE_RGBA, g_FontSwizzleMask);

            // Generate atlas
            m_atlasPack = new rbp::MaxRectsBinPack();
            m_atlasPack->Init(m_atlasSize.x, m_atlasSize.y, false);

            m_loaded = true;
        }
    }
    return m_loaded;
}

bool ROC::Font::LoadChar(unsigned int f_char)
{
    bool l_result = false;
    if(!FT_Load_Char(m_face, f_char, FT_LOAD_RENDER))
    {
        FontCharacterData *l_charData = new FontCharacterData();
        l_charData->m_size.x = m_face->glyph->bitmap.width;
        l_charData->m_size.y = m_face->glyph->bitmap.rows;
        l_charData->m_advance = static_cast<float>(m_face->glyph->advance.x >> 6);
        l_charData->m_bearing.x = m_face->glyph->bitmap_left;
        l_charData->m_bearing.y = m_face->glyph->bitmap_top;

        if(l_charData->m_size.x > 0 && l_charData->m_size.y > 0)
        {
            rbp::Rect l_rectangle = m_atlasPack->Insert(l_charData->m_size.x, l_charData->m_size.y, rbp::MaxRectsBinPack::RectBestShortSideFit);
            if(l_rectangle.height > 0)
            {
                l_charData->m_atlasPosition.x = static_cast<float>(l_rectangle.x) * m_atlasOffset.x;
                l_charData->m_atlasPosition.y = static_cast<float>(l_rectangle.y) * m_atlasOffset.y;
                l_charData->m_atlasPosition.z = l_charData->m_atlasPosition.x + static_cast<float>(l_charData->m_size.x) * m_atlasOffset.x;
                l_charData->m_atlasPosition.w = l_charData->m_atlasPosition.y + static_cast<float>(l_charData->m_size.y) * m_atlasOffset.y;
                m_atlasTexture->Update(l_rectangle.x, l_rectangle.y, l_rectangle.width, l_rectangle.height, GL_RED, m_face->glyph->bitmap.buffer);
            }
        }

        m_charIter = m_charMap.insert(std::make_pair(f_char, l_charData)).first;
        m_charMapEnd = m_charMap.end();

        l_result = true;
    }
    return l_result;
}

unsigned char ROC::Font::GetFiltering() const
{
    return m_filteringType;
}

float ROC::Font::GetGlyphSize() const
{
    return m_size;
}

void ROC::Font::Draw(const sf::String &f_text, const glm::vec2 &f_pos)
{
    if(m_loaded)
    {
        GLSetting::Set(GL_BLEND, true);
        GLSetting::SetDepthMask(false);

        ms_vertexArray->Bind();
        m_atlasTexture->Bind();

        glm::vec4 l_linePos(f_pos, 0.f, 0.f); // [line_x, line_y, char_x, char_y]
        glm::tvec3<size_t> l_textRange(0U); // [range_min, range_max, subpart]
        size_t l_stringLength = f_text.getSize();

        while(l_textRange.x < l_stringLength)
        {
            size_t l_charCount = 0;
            l_textRange.y = glm::min((l_textRange.z + 1U)*g_FontTextBlockSize, l_stringLength);

            for(size_t i = l_textRange.x; i < l_textRange.y; i++)
            {
                unsigned int l_character = f_text[i];
                if(l_character == 0xA) // New line
                {
                    l_linePos.x = f_pos.x;
                    l_linePos.y -= m_size;
                    continue;
                }

                m_charIter = m_charMap.find(l_character);
                if(m_charIter == m_charMapEnd)
                {
                    if(!LoadChar(l_character)) continue;
                }

                FontCharacterData *l_charData = m_charIter->second;
                if(l_charData->m_size.x > 0 && l_charData->m_size.y > 0)
                {
                    size_t l_charVertexIndex = static_cast<size_t>(l_charCount * g_FontCharacterVerticesCount);

                    l_linePos.z = l_linePos.x + l_charData->m_bearing.x;
                    l_linePos.w = l_linePos.y - (l_charData->m_size.y - l_charData->m_bearing.y);

                    ms_vertices[l_charVertexIndex].x = ms_vertices[l_charVertexIndex + 1].x = ms_vertices[l_charVertexIndex + 3].x = l_linePos.z;
                    ms_vertices[l_charVertexIndex + 1].y = ms_vertices[l_charVertexIndex + 2].y = ms_vertices[l_charVertexIndex + 4].y = l_linePos.w;
                    ms_vertices[l_charVertexIndex + 2].x = ms_vertices[l_charVertexIndex + 4].x = ms_vertices[l_charVertexIndex + 5].x = l_linePos.z + l_charData->m_size.x;
                    ms_vertices[l_charVertexIndex].y = ms_vertices[l_charVertexIndex + 3].y = ms_vertices[l_charVertexIndex + 5].y = l_linePos.w + l_charData->m_size.y;

                    ms_uv[l_charVertexIndex].x = ms_uv[l_charVertexIndex + 1].x = ms_uv[l_charVertexIndex + 3].x = l_charData->m_atlasPosition.x;
                    ms_uv[l_charVertexIndex].y = ms_uv[l_charVertexIndex + 3].y = ms_uv[l_charVertexIndex + 5].y = l_charData->m_atlasPosition.y;
                    ms_uv[l_charVertexIndex + 2].x = ms_uv[l_charVertexIndex + 4].x = ms_uv[l_charVertexIndex + 5].x = l_charData->m_atlasPosition.z;
                    ms_uv[l_charVertexIndex + 1].y = ms_uv[l_charVertexIndex + 2].y = ms_uv[l_charVertexIndex + 4].y = l_charData->m_atlasPosition.w;

                    l_charCount++;
                }
                l_linePos.x += m_charIter->second->m_advance;
            }

            if(l_charCount > 0)
            {
                ms_arrayBuffers[FBI_Vertex]->Bind();
                ms_arrayBuffers[FBI_Vertex]->Update(0, sizeof(glm::vec3) * g_FontCharacterVerticesCount * l_charCount, ms_vertices.data());

                ms_arrayBuffers[FBI_UV]->Bind();
                ms_arrayBuffers[FBI_UV]->Update(0, sizeof(glm::vec2) * g_FontCharacterVerticesCount * l_charCount, ms_uv.data());

                ms_vertexArray->Draw(GL_TRIANGLES, static_cast<int>(g_FontCharacterVerticesCount * l_charCount));
            }
            l_textRange.x = ++l_textRange.z * g_FontTextBlockSize;
        }
    }
}

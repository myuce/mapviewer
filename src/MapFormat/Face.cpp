#include <cmath>
#include <numeric>
#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include "map.hpp"

vec3 Face::GetNormal()
{
    if (glm::length(normal) < 1e-6f)
    {
        vec3 e1 = p2 - p1;
        vec3 e2 = p3 - p1;
        normal = glm::normalize(glm::cross(e1, e2));
    }

    return normal;
}

float Face::GetDistance()
{
    if (std::fabs(distance) < 1e-6f)
    {
        vec3 n = GetNormal();
        distance = glm::dot(n, p1);
    }

    return distance;
}

vec3 Face::GetCenter()
{
    if (glm::length(center) < 1e-6f)
    {
        center = (p1 + p2 + p3) / 3.0f;
    }

    return center;
}

std::vector<vec3> Face::GetVertices()
{
    std::vector<vec3> verts = {};

    for (auto &idx : vertices)
    {
        verts.push_back(parentBrush->vertices[idx]);
    }

    return verts;
}

float Deg2Rad(float degrees)
{
    return degrees * (M_PI / 180.0f);
}

vec2 GetStandardUV(vec3 &vertex, Face *face)
{
    vec2 ret;
    vec2 textureSize = face->textureSize.x == 0.0f ? vec2(512.0f, 512.0f) : face->textureSize;
    vec3 normal = face->GetNormal();

    vec3 UP_VECTOR = vec3(0.0f, 0.0f, 1.0f);
    vec3 RIGHT_VECTOR = vec3(0.0f, 1.0f, 0.0f);
    vec3 FORWARD_VECTOR = vec3(1.0f, 0.0f, 1.0f);

    float du = std::fabs(glm::dot(normal, UP_VECTOR));
    float dr = std::fabs(glm::dot(normal, RIGHT_VECTOR));
    float df = std::fabs(glm::dot(normal, FORWARD_VECTOR));

    if (du >= dr && du >= df)
    {
        ret = vec2(vertex.x, -vertex.y);
    }
    else if (dr >= du && dr >= df)
    {
        ret = vec2(vertex.x, -vertex.z);
    }
    else if (df >= du && df >= dr)
    {
        ret = vec2(vertex.y, -vertex.z);
    }

    float angle = Deg2Rad(face->textureProjection.standard.rotation);
    ret = vec2(
        ret.x * cos(angle) - ret.y * sin(angle),
        ret.x * sin(angle) + ret.y * cos(angle));

    ret.x /= textureSize.x;
    ret.y /= textureSize.y;

    ret.x /= face->textureProjection.standard.xScale;
    ret.y /= face->textureProjection.standard.yScale;

    ret.x += face->textureProjection.standard.xOffset / textureSize.x;
    ret.y += face->textureProjection.standard.yOffset / textureSize.y;

    return ret;
}

vec2 GetValve220UV(vec3 &vertex, Face *face)
{
    vec2 textureSize = face->textureSize.x == 0.0f ? vec2(512.0f, 512.0f) : face->textureSize;

    return vec2(
        dot(vertex, face->textureProjection.valve220.uAxis) / (textureSize.x * face->textureProjection.valve220.xScale) +
            (face->textureProjection.valve220.xOffset / textureSize.x),
        dot(vertex, face->textureProjection.valve220.vAxis) / (textureSize.y * face->textureProjection.valve220.yScale) +
            (face->textureProjection.valve220.yOffset / textureSize.y));
}

vec2 Face::GetUV(vec3 &vert)
{
    vec2 uv;

    switch (projectionType)
    {
    case TextureProjectionType::Standard:
        uv = GetStandardUV(vert, this);
        break;
    case TextureProjectionType::Valve220:
        uv = GetValve220UV(vert, this);
        break;
    default:
        uv = vec2(0.0f);
        break;
    }

    return uv;
}

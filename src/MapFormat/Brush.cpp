#include <algorithm>
#include "map.hpp"

bool isEqual(vec3 &a, vec3 &b)
{
    return glm::length(a - b) < 0.001f;
}

bool isPointInList(vec3 &point, std::vector<vec3> &list)
{
    for (auto &v : list)
    {
        if (isEqual(point, v))
            return true;
    }
    return false;
}

bool isPointLegal(vec3 &p, Brush *brush)
{
    for (auto &face : brush->faces)
    {
        vec3 facing = normalize(p - face.GetCenter());
        if (dot(facing, face.GetNormal()) < -0.001f)
            return false;
    }
    return true;
}

vec3 Brush::GetCenter()
{
    if (glm::length(center) < 0.001f)
    {
        center = vec3(0.0f);

        for (auto &vertex : vertices)
        {
            center += vertex;
        }

        center /= static_cast<float>(vertices.size());
        return center;
    }

    return center;
}

AABB Brush::GetAABB()
{
    if (aabb.min == vec3(0.0f) && aabb.max == vec3(0.0f))
    {
        for (auto &vertex : vertices)
        {
            aabb.min = glm::min(aabb.min, vertex);
            aabb.max = glm::max(aabb.max, vertex);
        }
    }

    return aabb;
}

// — WindingForFace: build one giant quad in the face’s plane —
static std::vector<vec3> WindingForFace(Face &f, float large = 8192.0f)
{
    vec3 n = f.GetNormal();
    float d = f.GetDistance();
    // pick an arbitrary “right” axis not parallel to n
    vec3 up = (fabs(n.z) < 0.9f ? vec3(0, 0, 1) : vec3(1, 0, 0));
    vec3 u = glm::normalize(glm::cross(up, n));
    vec3 v = glm::cross(n, u);
    // center on the plane
    vec3 origin = n * d;
    // build a huge quad
    std::vector<vec3> w(4);
    w[0] = origin + (u + v) * large;
    w[1] = origin + (-u + v) * large;
    w[2] = origin + (-u + -v) * large;
    w[3] = origin + (u + -v) * large;
    return w;
}

// — ClipToPlane: Sutherland–Hodgman clip of polygon against plane —
static std::vector<vec3> ClipToPlane(const std::vector<vec3> &in,
                                     const vec3 &clipN, float clipD,
                                     float eps = 1e-3f)
{
    std::vector<vec3> out;
    auto side = [&](const vec3 &p)
    { return glm::dot(clipN, p) - clipD; };
    for (size_t i = 0, n = in.size(); i < n; ++i)
    {
        const vec3 &P = in[i];
        const vec3 &Q = in[(i + 1) % n];
        float sP = side(P), sQ = side(Q);
        if (sP >= -eps)
            out.push_back(P);
        if ((sP > 0) != (sQ > 0))
        {
            // edge crosses plane → compute intersection
            float t = sP / (sP - sQ);
            out.push_back(P + t * (Q - P));
        }
    }
    return out;
}

void Brush::CalculateGeometry()
{
    vertices.clear();

    // for each face, build & clip its winding
    for (Face &face : faces)
    {
        // 1) start with huge quad in this face’s plane
        auto poly = WindingForFace(face);

        // 2) clip against every *other* brush plane
        for (Face &other : faces)
            if (&other != &face)
                poly = ClipToPlane(poly,
                                   other.GetNormal(),
                                   other.GetDistance());

        // 3) collect each remaining vertex
        face.vertices.clear();
        for (auto &p : poly)
        {
            // add to brush‐wide unique list
            auto it = std::find_if(vertices.begin(), vertices.end(),
                                   [&](const vec3 &v)
                                   { return glm::length(v - p) < 1e-3f; });
            int idx;

            if (it == vertices.end())
            {
                idx = vertices.size();
                vertices.push_back(p);
            }
            else
            {
                idx = int(std::distance(vertices.begin(), it));
            }

            face.vertices.push_back(idx);
        }

        if (face.vertices.size() >= 3) {
            auto polyVerts = face.GetVertices();
            // grab three consecutive points in your wound polygon
            const vec3 &v0 = polyVerts[0];
            const vec3 &v1 = polyVerts[1];
            const vec3 &v2 = polyVerts[2];
        
            // compute the winding-normal of that little triangle
            vec3 windingNormal = glm::normalize( glm::cross(v1 - v0, v2 - v0) );
        
            // compare against the face’s true plane normal
            if (glm::dot(windingNormal, face.GetNormal()) > 0.0f) {
                // it’s backwards — flip the entire loop
                std::reverse(face.vertices.begin(), face.vertices.end());
            }
        }
    }
}

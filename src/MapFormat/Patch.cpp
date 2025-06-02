#include "map.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

// Evaluate 1D quadratic Bezier (same as yours)
static float BezierQuad1D(float b0, float b1, float b2, float t)
{
    float it = 1.0f - t;
    return b0 * (it * it) + 2.0f * b1 * (it * t) + b2 * (t * t);
}

// A lightweight evaluator that only computes pos+uv (no normal, no recursion).
static PatchVert EvaluatePatchPoint(
    const PatchVert cp[3][3],
    float u, float v)
{
    // first across U
    PatchVert tmp[3];
    for (int i = 0; i < 3; ++i)
    {
        tmp[i].position.x = BezierQuad1D(cp[i][0].position.x, cp[i][1].position.x, cp[i][2].position.x, u);
        tmp[i].position.y = BezierQuad1D(cp[i][0].position.y, cp[i][1].position.y, cp[i][2].position.y, u);
        tmp[i].position.z = BezierQuad1D(cp[i][0].position.z, cp[i][1].position.z, cp[i][2].position.z, u);
        tmp[i].uv.x       = BezierQuad1D(cp[i][0].uv.x,       cp[i][1].uv.x,       cp[i][2].uv.x,       u);
        tmp[i].uv.y       = BezierQuad1D(cp[i][0].uv.y,       cp[i][1].uv.y,       cp[i][2].uv.y,       u);
    }

    // then across V
    PatchVert out;
    out.position.x = BezierQuad1D(tmp[0].position.x, tmp[1].position.x, tmp[2].position.x, v);
    out.position.y = BezierQuad1D(tmp[0].position.y, tmp[1].position.y, tmp[2].position.y, v);
    out.position.z = BezierQuad1D(tmp[0].position.z, tmp[1].position.z, tmp[2].position.z, v);
    out.uv.x       = BezierQuad1D(tmp[0].uv.x,       tmp[1].uv.x,       tmp[2].uv.x,       v);
    out.uv.y       = BezierQuad1D(tmp[0].uv.y,       tmp[1].uv.y,       tmp[2].uv.y,       v);
    return out;
}

// Now the full patch evaluator that also computes a normal by finite‐difference
static PatchVert EvaluateQuadPatch(
    const std::vector<std::vector<PatchVert>> &cpGrid,
    float u, float v)
{
    // pull out the 3×3 block into a fixed‐size array for speed & simplicity
    PatchVert cp[3][3];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            cp[i][j] = cpGrid[i][j];

    // central sample
    PatchVert center = EvaluatePatchPoint(cp, u, v);

    // sample offsets for normal
    const float d = 0.001f;
    float u1 = glm::clamp(u + d, 0.0f, 1.0f);
    float v1 = glm::clamp(v + d, 0.0f, 1.0f);

    PatchVert pu = EvaluatePatchPoint(cp, u1, v);
    PatchVert pv = EvaluatePatchPoint(cp, u, v1);

    glm::vec3 du = pu.position - center.position;
    glm::vec3 dv = pv.position - center.position;

    center.normal = glm::normalize(glm::cross(du, dv));
    return center;
}

void Patch::CalculateGeometry()
{
    vertices.clear();
    if (controlPoints.empty()) return;

    int rows = (int)controlPoints.size();
    int cols = (int)controlPoints[0].size();
    int pV   = (rows - 1) / 2;
    int pU   = (cols - 1) / 2;
    if (pV < 1 || pU < 1) return;

    // determine subdivisions dynamically
    glm::vec3 c00 = controlPoints[0][0].position;
    glm::vec3 cNN = controlPoints[rows-1][cols-1].position;
    float diag = glm::length(cNN - c00);
    int rawN = int(diag * 0.1f);
    int N = std::clamp(rawN, 1, 5);
    int nV = pV * N;
    int nU = pU * N;

    vertices.resize(nV+1);
    for (int i = 0; i <= nV; ++i)
    {
        float vParam  = float(i) / float(nV);
        float vScaled = vParam * float(pV);
        int   subV    = std::min(pV-1, int(std::floor(vScaled)));
        float vLocal  = vScaled - float(subV);

        vertices[i].resize(nU+1);
        for (int j = 0; j <= nU; ++j)
        {
            float uParam  = float(j) / float(nU);
            float uScaled = uParam * float(pU);
            int   subU    = std::min(pU-1, int(std::floor(uScaled)));
            float uLocal  = uScaled - float(subU);

            // build a 3×3 control‐point block
            std::vector<std::vector<PatchVert>> block(3, std::vector<PatchVert>(3));
            for (int vv = 0; vv < 3; ++vv)
                for (int uu = 0; uu < 3; ++uu)
                    block[vv][uu] = controlPoints[subV*2 + vv][subU*2 + uu];

            // evaluate
            vertices[i][j] = EvaluateQuadPatch(block, uLocal, vLocal);
        }
    }
}

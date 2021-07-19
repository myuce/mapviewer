#include <cstdio>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <cmath>
#include "vec3.h"
#include <algorithm>

class Side
{
    public:
    vec3 p1, p2, p3;
    float xScale, yScale, rotation, xShift, yShift;
    char texture[256];
    int pointCount = 0;
    std::vector<vec3> points;

    Side(){}

    ~Side(){}

    vec3 normal()
    {
        vec3 ab = p2 - p1;
        vec3 ac = p3 - p1;
        return ab.cross(ac);
    }

    vec3 center()
    {
        return (p1 + p2 + p3) / 3;
    }

    float distance()
    {
        vec3 n = normal();
        return ((p1.x * n.x) + (p1.y * n.y) + (p1.z * n.z)) / sqrtf(powf(n.x, 2) + powf(n.y, 2) + powf(n.z, 2));
    }

    // calculate the center of a brush face by averaging the positions of each vertex
    vec3 pointCenter()
    {
        vec3 c;
        for (int i = 0; i < pointCount; i++)
            c = c + points[i];
        return c / pointCount;
    }

    // sort vertices clockwise by comparing their angles to the center
    void sortVertices()
    {
        vec3 c = pointCenter() + 1e-5; // in case the angle of every vertex is the same, the center  should be slightly off
        vec3 n = normal();
        std::stable_sort(points.begin(), points.end(), [&](vec3  lhs, vec3 rhs)
        {
            if (lhs == rhs)
                return false; // this line makes this an unstable sort
            
            vec3 ca = c - lhs;
            vec3 cb = c - rhs;
            vec3 caXcb = ca.cross(cb);
            
            return n.dot(caXcb) >= 0;
        });
    }
};

// calculate the intersection points of three planes
bool getPlaneIntersection(Side side1, Side side2, Side side3, vec3 &out)
{
    vec3 normal1 = side1.normal().normalize();
    vec3 normal2 = side2.normal().normalize();
    vec3 normal3 = side3.normal().normalize();
    float determinant = normal1.dot(normal2.cross(normal3));

    // can't intersect parallel planes
    if ((determinant <= 1e-5 and determinant >= -1e-5))
        return false;

    out = (
            normal2.cross(normal3) * side1.distance() +
            normal3.cross(normal1) * side2.distance() +
            normal1.cross(normal2) * side3.distance()
        ) / determinant;

    return true;
}

// avoid adding duplicate verts
bool inList(vec3 vec, std::vector<vec3> vecs)
{
    for (vec3 v: vecs)
    {
        if(v == vec)
            return true;
    }
    return false;
}

class Brush
{
    public:
    int sideCount = 0;
    std::vector<Side> sides;

    Brush(){}

    ~Brush(){}

    // check if a point is a part of the convex shape
    bool pointIsLegal(vec3 v)
    {
        for (int i = 0; i < sideCount; i++)
        {
            vec3 facing = (v - sides[i].center()).normalize();
            if (facing.dot(sides[i].normal().normalize()) < -1e-5)
                return false;
        }
        return true;
    }

    void getIntersectionPoints()
    {
        vec3 point;
        for (int i = 0; i < sideCount - 2; i++)
        {
            for (int j = 0; j < sideCount - 1; j++)
            {
                for (int k = 0; k < sideCount; k++)
                {
                    if (i != j && i != k && j != k)
                    {
                        if (getPlaneIntersection(sides[i], sides[j], sides[k], point))
                        {
                            if (pointIsLegal(point))
                            {
                                if (!inList(point, sides[i].points))
                                {
                                    sides[i].points.push_back(point);
                                    sides[i].pointCount++;
                                }

                                if (!inList(point, sides[j].points))
                                {
                                    sides[j].points.push_back(point);
                                    sides[j].pointCount++;
                                }
                                
                                if (!inList(point, sides[k].points))
                                {
                                    sides[k].points.push_back(point);
                                    sides[k].pointCount++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

class Entity
{
    public:
    int brushCount = 0;
    std::vector<Brush> brushes;
    std::map<std::string, std::string> kvp;
    Entity(){}
    ~Entity(){}
};

bool parseSide(Side &res, const char* sideStr)
{
    int count = sscanf(sideStr, "( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %s %f %f %f %f %f",
        &res.p1.x, &res.p1.y, &res.p1.z,
        &res.p2.x, &res.p2.y, &res.p2.z,
        &res.p3.x, &res.p3.y, &res.p3.z,
        res.texture,
        &res.xScale, &res.yScale,
        &res.rotation,
        &res.yShift, &res.yShift
    );
    if (count == 15)
    {
        return true;
    }
    return false;
}

bool parseKvp(std::map<std::string, std::string> &kv, const char* kvString)
{
    char kvp[1024];
    char key[512];
    char value[512];

    strcpy(kvp, kvString);
    
    char* tok = strtok(kvp, "\"");

    int i = 0;

    while(tok != NULL)
    {
        if (i == 0)
        {
            strcpy(key, tok);
        }
        else if (i == 2)
        {
            strcpy(value, tok);
        }
        i++;
        tok = strtok(NULL, "\"");
    }
    if (i == 4)
    {
        kv[key] = value;
        return true;
    }
    return false;
}

enum
{
    NONE = 0,
    ENTITY,
    BRUSH,
    PATCH
};

bool parseMap(std::vector<Entity> &entities, const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    if (file == NULL)
    {
        printf("Could not open file %s\n", fileName);
        return false;
    }

    printf("Loading map file %s...\n", fileName);

    char line[256];

    int l = 0;
    int CURRENT = NONE;

    Entity tempEnt = Entity();
    Brush tempBrush = Brush();
    Side tempSide = Side();

    while (fgets(line, sizeof(line), file))
    {
        l++;
        // check for what to expect first
        if (line[0] == '/')
        { // it's a comment
            continue;
        }
        else if (line[0] == '{')
        { // open bracket
            if (CURRENT == NONE)
            {
                CURRENT = ENTITY;
                continue;
            }
            else if (CURRENT == ENTITY)
            {
                CURRENT = BRUSH;
                continue;
            }
            else
            {
                printf("Parse error. Unexpected { on line %i.\n", l);
                fclose(file);
                return false;
            }
        }
        else if (line[0] == '}')
        { // close bracket
            if (CURRENT == BRUSH)
            {
                tempEnt.brushes.push_back(tempBrush);
                tempEnt.brushCount++;
                tempBrush = Brush();
                CURRENT = ENTITY;
                continue;
            }
            else if (CURRENT == ENTITY)
            {
                entities.push_back(tempEnt);
                tempEnt = Entity();
                CURRENT = NONE;
                continue;
            }
            else if (CURRENT == NONE)
            {
                printf("Parse error. Unexpected } on line %i.\n", l);
                fclose(file);
                return false;
            }
        }
        else if (line[0] == '"')
        { // kvp
            if (CURRENT != ENTITY)
            {
                printf("Parse error. Unexpected \" on line %i.\n", l);
                fclose(file);
                return false;
            }
            if (parseKvp(tempEnt.kvp, line))
            {
                continue;
            }
            else
            {
                printf("Parse error. Invalid KVP syntax on line %i.\n", l);
                fclose(file);
                return false;
            }
        }
        else if (line[0] == '(')
        { // brush side
            if (CURRENT != BRUSH)
            {
                printf("Parse error. Unexpected ( on line %i.\n", l);
                fclose(file);
                return false;
            }
            if (parseSide(tempSide, line))
            {
                tempBrush.sides.push_back(tempSide);
                tempBrush.sideCount++;
            }
            else
            {
                printf("Parse error. Invalid plane syntax on line %i.\n", l);
                fclose(file);
                return false;
            }
        }

    }

    fclose(file);
    return true;
}
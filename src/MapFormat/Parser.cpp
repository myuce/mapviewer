#include "Parser.hpp"
#include <cstdlib>
#include <cstdio>

static int entityCounter = 0;
static int geoCounter = 0;

#define EXPECT_TOKEN(lexer, token, expectedType, context)                                              \
    do                                                                                                 \
    {                                                                                                  \
        Token __token = (token);                                                                       \
        if ((__token).type != (expectedType))                                                          \
        {                                                                                              \
            printf("[Parse error] Expected %s in %s on line %d, got '%s' (TOKEN TYPE: %i)\n",          \
                   #expectedType, context, (lexer).getLine(), (__token).text.c_str(), (__token).type); \
            return false;                                                                              \
        }                                                                                              \
    } while (0)

#define ASSIGN_FLOAT(token, var, context)                                                                                              \
    do                                                                                                                                 \
    {                                                                                                                                  \
        Token _tok = (token);                                                                                                          \
        char *_endptr = nullptr;                                                                                                       \
        float _val = std::strtof(_tok.text.c_str(), &_endptr);                                                                         \
        if (_tok.text.empty() || _endptr == _tok.text.c_str() || *_endptr != '\0')                                                     \
        {                                                                                                                              \
            printf("[Parse error] Expected numeric string in %s on line %d, got '%s'\n", context, lexer.getLine(), _tok.text.c_str()); \
            return false;                                                                                                              \
        }                                                                                                                              \
        (var) = _val;                                                                                                                  \
    } while (0)

#define ASSIGN_INT(token, var, context)                                                                                                \
    do                                                                                                                                 \
    {                                                                                                                                  \
        Token _tok = (token);                                                                                                          \
        char *_endptr = nullptr;                                                                                                       \
        int _val = std::strtol(_tok.text.c_str(), &_endptr, 10);                                                                       \
        if (_tok.text.empty() || _endptr == _tok.text.c_str() || *_endptr != '\0')                                                     \
        {                                                                                                                              \
            printf("[Parse error] Expected numeric string in %s on line %d, got '%s'\n", context, lexer.getLine(), _tok.text.c_str()); \
            return false;                                                                                                              \
        }                                                                                                                              \
        (var) = _val;                                                                                                                  \
    } while (0)

static bool parseBrush(Lexer &lexer, Brush *brush)
{
    Token tok;

    while ((tok = lexer.next()).type != TokenType::RBRACE)
    {
        brush->faces.emplace_back(brush);
        Face &face = brush->faces.back();
        face.parentBrush = brush;

        for (int i = 0; i < 3; i++)
        {
            EXPECT_TOKEN(lexer, tok, TokenType::LPAREN, "brush");

            float x, y, z;
            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
            ASSIGN_FLOAT(tok, x, "brush");
            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
            ASSIGN_FLOAT(tok, y, "brush");
            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
            ASSIGN_FLOAT(tok, z, "brush");

            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RPAREN, "brush");

            switch (i)
            {
            case 0:
                face.p1 = vec3(x, y, z);
                break;
            case 1:
                face.p2 = vec3(x, y, z);
                break;
            case 2:
                face.p3 = vec3(x, y, z);
                break;
            };

            tok = lexer.next();
        }

        if (tok.type != TokenType::WORD && tok.type != TokenType::QUOTED_STRING)
        {
            EXPECT_TOKEN(lexer, tok, TokenType::WORD, "brush");
        }

        face.texture = tok.text;
        face.parentBrush->parentEntity->parentMap->textureSizes[face.texture] = vec2(0.0f);

        tok = lexer.next();
        lexer.pushBack(tok);

        if (tok.type == TokenType::LBRACKET)
        {
            face.projectionType = TextureProjectionType::Valve220;

            for (int i = 0; i < 2; i++)
            {
                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LBRACKET, "brush");

                float x, y, z, w;
                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
                ASSIGN_FLOAT(tok, x, "brush");
                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
                ASSIGN_FLOAT(tok, y, "brush");
                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
                ASSIGN_FLOAT(tok, z, "brush");
                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "brush");
                ASSIGN_FLOAT(tok, w, "brush");

                EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RBRACKET, "brush");

                switch (i)
                {
                case 0:
                    face.textureProjection.valve220.uAxis = vec3(x, y, z);
                    face.textureProjection.valve220.xOffset = w;
                    break;
                case 1:
                    face.textureProjection.valve220.vAxis = vec3(x, y, z);
                    face.textureProjection.valve220.yOffset = w;
                    break;
                };
            }

            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.valve220.rotation, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.valve220.xScale, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.valve220.yScale, "brush");
        }
        else
        {
            face.projectionType = TextureProjectionType::Standard;

            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.standard.xOffset, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.standard.yOffset, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.standard.rotation, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.standard.xScale, "brush");
            ASSIGN_FLOAT(tok = lexer.next(), face.textureProjection.standard.yScale, "brush");
        }

        tok = lexer.next();

        while (tok.type != TokenType::LPAREN && tok.type != TokenType::RBRACE)
        {
            ASSIGN_INT(tok, face.flags[face.flagCount++], "brush");
            tok = lexer.next();
        }

        lexer.pushBack(tok);
    }

    EXPECT_TOKEN(lexer, tok, TokenType::RBRACE, "brush");
    return true;
}

static bool parsePatch(Lexer &lexer, Patch *patch)
{
    Token tok;
    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LBRACE, "patch");

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::WORD, "patch");
    patch->texture = tok.text;
    patch->parentEntity->parentMap->textureSizes[patch->texture] = vec2(0.0f);

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LPAREN, "patch");

    ASSIGN_INT(tok = lexer.next(), patch->height, "patch");
    ASSIGN_INT(tok = lexer.next(), patch->width, "patch");

    ASSIGN_INT(tok = lexer.next(), patch->flags[0], "patch");
    ASSIGN_INT(tok = lexer.next(), patch->flags[1], "patch");
    ASSIGN_INT(tok = lexer.next(), patch->flags[2], "patch");

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RPAREN, "patch");

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LPAREN, "patch");

    patch->controlPoints.reserve(patch->width);

    for (int i = 0; i < patch->height; i++)
    {
        std::vector<PatchVert> row(patch->width);
        EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LPAREN, "patch");

        for (int j = 0; j < patch->width; j++)
        {
            PatchVert cp;

            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::LPAREN, "patch");

            ASSIGN_FLOAT(tok = lexer.next(), cp.position.x, "patch");
            ASSIGN_FLOAT(tok = lexer.next(), cp.position.y, "patch");
            ASSIGN_FLOAT(tok = lexer.next(), cp.position.z, "patch");

            ASSIGN_FLOAT(tok = lexer.next(), cp.uv.x, "patch");
            ASSIGN_FLOAT(tok = lexer.next(), cp.uv.y, "patch");

            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RPAREN, "patch");

            row[j] = cp;
        }

        patch->controlPoints.push_back(std::move(row));

        EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RPAREN, "patch");
    }

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RPAREN, "patch");

    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RBRACE, "patch");
    EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::RBRACE, "patch");

    return true;
}

static bool parseEntity(Lexer &lexer, Entity *entity)
{
    Token tok;

    while ((tok = lexer.next()).type != TokenType::RBRACE)
    {
        if (tok.type == TokenType::QUOTED_STRING)
        {
            std::string key = tok.text;
            EXPECT_TOKEN(lexer, tok = lexer.next(), TokenType::QUOTED_STRING, "entity");
            std::string value = tok.text;
            entity->properties[key] = value;

            if (key == "model")
            {
                entity->parentMap->models.insert(value);
            }
        }
        else if (tok.type == TokenType::LBRACE)
        {
            tok = lexer.next();

            if (tok.type == TokenType::LPAREN)
            {
                entity->brushes.emplace_back(geoCounter++, entity);
                Brush &brush = entity->brushes.back();
                brush.parentEntity = entity;
                lexer.pushBack(tok);

                if (!parseBrush(lexer, &brush))
                    return false;
            }
            else if (tok.type == TokenType::WORD && tok.text == "patchDef2")
            {
                entity->patches.emplace_back(geoCounter++, entity);
                Patch &patch = entity->patches.back();
                patch.parentEntity = entity;

                if (!parsePatch(lexer, &patch))
                    return false;
            }
        }
    }

    return true;
}

bool parseMap(Lexer &lexer, Map *map)
{
    Token tok;

    while ((tok = lexer.next()).type != TokenType::END)
    {
        EXPECT_TOKEN(lexer, tok, TokenType::LBRACE, "map file");
        map->entities.emplace_back(entityCounter++, map);
        Entity &entity = map->entities.back();
        entity.parentMap = map;

        if (!parseEntity(lexer, &entity))
            return false;

        EXPECT_TOKEN(lexer, tok, TokenType::LBRACE, "map file");
    }

    // Re-assign parents for entities, brushes etc. to avoid potential dangling pointers
    // this is a temporary fix until we can refactor the code to use a better solution
    for (auto &entity : map->entities)
    {
        entity.parentMap = map;
        for (auto &brush : entity.brushes)
        {
            brush.parentEntity = &entity;
            for (auto &face : brush.faces)
            {
                face.parentBrush = &brush;
            }
        }
        for (auto &patch : entity.patches)
        {
            patch.parentEntity = &entity;
        }
    }

    return true;
}

#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"

void DrawDebugRing(Vec2 const& center, float const& radius, float const& scale, float const& orientation, float const& thickness, Rgba8 const& color)
{
	float thetaDegrees = 360.0f / 50.0f;

	Vertex_PCU vertices[300];

	for (int index = 0; index < 50; index++)
	{
		float theta1 = index * thetaDegrees;
		float theta2 = (index + 1) * thetaDegrees;

		Vec2 vert1 = Vec2(radius * CosDegrees(theta1), radius * SinDegrees(theta1));
		Vec2 vert2 = Vec2(radius * CosDegrees(theta2), radius * SinDegrees(theta2));
		Vec2 vert3 = Vec2((radius + thickness) * CosDegrees(theta1), (radius + thickness) * SinDegrees(theta1));
		Vec2 vert4 = Vec2((radius + thickness) * CosDegrees(theta2), (radius + thickness) * SinDegrees(theta2));

		vertices[6 * index    ] = Vertex_PCU(vert1.x, vert1.y, color.r, color.g, color.b, color.a);
		vertices[6 * index + 1] = Vertex_PCU(vert3.x, vert3.y, color.r, color.g, color.b, color.a);
		vertices[6 * index + 2] = Vertex_PCU(vert4.x, vert4.y, color.r, color.g, color.b, color.a);
		vertices[6 * index + 3] = Vertex_PCU(vert4.x, vert4.y, color.r, color.g, color.b, color.a);
		vertices[6 * index + 4] = Vertex_PCU(vert2.x, vert2.y, color.r, color.g, color.b, color.a);
		vertices[6 * index + 5] = Vertex_PCU(vert1.x, vert1.y, color.r, color.g, color.b, color.a);
	}

	TransformVertexArrayXY3D(300, vertices, scale, orientation, center);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(300, vertices);
}

void DrawDebugLine(Vec2 const& startPos, Vec2 const& endPos, float const& thickness, Rgba8 const& color)
{
	Vec2 direction = endPos - startPos;
	Vec2 forward = direction.GetNormalized();

	forward.SetLength(thickness / 2.0f);

	Vec2 left = forward.GetRotated90Degrees();

	Vec2 endLeft = endPos + forward + left;
	Vec2 endRight = endPos + forward - left;
	Vec2 startLeft = startPos - forward + left;
	Vec2 startRight = startPos - forward - left;

	Vertex_PCU vertices[6];

	vertices[0] = Vertex_PCU(startLeft.x, startLeft.y, color.r, color.g, color.b, color.a);
	vertices[1] = Vertex_PCU(startRight.x, startRight.y, color.r, color.g, color.b, color.a);
	vertices[2] = Vertex_PCU(endRight.x, endRight.y, color.r, color.g, color.b, color.a);
	vertices[3] = Vertex_PCU(endRight.x, endRight.y, color.r, color.g, color.b, color.a);
	vertices[4] = Vertex_PCU(endLeft.x, endLeft.y, color.r, color.g, color.b, color.a);
	vertices[5] = Vertex_PCU(startLeft.x, startLeft.y, color.r, color.g, color.b, color.a);

	TransformVertexArrayXY3D(6, vertices, 1.0f, 0.0f, Vec2(0.0f, 0.0f));
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(6, vertices);
}

void DrawDebugDisc(Vec2 const& center, float radius, Rgba8 const& color)
{
	std::vector<Vertex_PCU> vertices;

	AddVertsForDisc2D(vertices, center, radius, color);

	TransformVertexArrayXY3D(static_cast<int>(vertices.size()), vertices.data(), 1.0f, 0.0f, Vec2::ZERO);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
}
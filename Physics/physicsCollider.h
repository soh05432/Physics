#pragma once

#include <Base.h>

#include <physicsObject.h>

#include <vector>
#include <array>

struct ContactPoint
{
private:

	Real m_depth;
	Vector4 m_posA; // Contact on A seen by A
	Vector4 m_posB; // Contact on B seen by B
	Vector4 m_norm; // Point from bodyA to bodyB

public:

	ContactPoint()
		: m_depth( 0.f ), m_posA(), m_posB(), m_norm() {}

	ContactPoint( Real depth, const Vector4& posA, const Vector4& posB, const Vector4& norm )
		: m_depth( depth ), m_posA( posA ), m_posB( posB ), m_norm( norm ) {}

	inline const Real getDepth() const { return m_depth; }
	inline const Vector4& getContactA() const { return m_posA; }
	inline const Vector4& getContactB() const { return m_posB; }
	inline const Vector4& getNormal() const { return m_norm; }

};

namespace ContactPointUtils
{
	void getContactDifference( const ContactPoint& cpA, const ContactPoint& cpB, Real& res );
}

class physicsCollider : public physicsObject
{
public:

	enum Types
	{
		CIRCLE_CIRCLE = 0,
		CIRCLE_BOX,
		BOX_BOX,
		CONVEX_CONVEX,
		NUM_COLLIDERS
	};
	
public:

    physicsCollider();

	static void collide( const std::shared_ptr<physicsShape>& shapeA,
						 const std::shared_ptr<physicsShape>& shapeB,
						 const Transform& transformA,
						 const Transform& transformB,
						 std::vector<ContactPoint>& contacts );
};

class physicsCircleCollider : public physicsCollider
{
private:

    physicsCircleCollider();

public:

	static void collide( const physicsShape* shapeA,
						 const physicsShape* shapeB,
						 const Transform& transformA,
						 const Transform& transformB,
						 std::vector<ContactPoint>& contacts );
};

class physicsCircleBoxCollider : public physicsCollider
{
private:

    physicsCircleBoxCollider();

public:

	static void collide( const physicsShape* shapeA,
						 const physicsShape* shapeB,
						 const Transform& transformA,
						 const Transform& transformB,
						 std::vector<ContactPoint>& contacts );
};

class physicsBoxCollider : public physicsCollider
{
private:

	physicsBoxCollider();

public:

	static void collide( const physicsShape* shapeA,
						 const physicsShape* shapeB,
						 const Transform& transformA,
						 const Transform& transformB,
						 std::vector<ContactPoint>& contacts );
};



class physicsConvexCollider: public physicsCollider
{
public:
	
	typedef std::array<Vector4, 3> SimplexVertex; // [0] = vertex, [1] = supportA, [2] = supportB
	typedef std::vector<SimplexVertex> Simplex;

	struct SimplexEdge
	{
		int index;
		Real distSq;
		Vector4 normal;
	};

	// Finds simplex vertex and it's support vertices local to A
	static void getSimplexVertex( const Vector4& direction,
								  const physicsShape* shapeA,
								  const physicsShape* shapeB,
								  const Transform& transformA,
								  const Transform& transformB,
								  SimplexVertex& simplexVert );

private:


	physicsConvexCollider();


	static void expandingPolytopeAlgorithm( const physicsShape* shapeA,
											const physicsShape* shapeB,
											const Transform& transformA,
											const Transform& transformB,
											Simplex& simplex,
											struct SimplexEdge& closestEdge );

	static void getClosestEdgeToOrigin( const Simplex& simplex, struct SimplexEdge& edge );

public:

	static void collide( const physicsShape* shapeA,
						 const physicsShape* shapeB,
						 const Transform& transformA,
						 const Transform& transformB,
						 std::vector<ContactPoint>& contacts );
};

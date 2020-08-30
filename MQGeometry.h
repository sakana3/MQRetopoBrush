#pragma once

#include "MQ3DLib.h"
#include "MQPlugin.h"
#include "libacc\\bvh_tree.h"

#define MAX_FACE_VERT 100

#define Trace( str, ... ) \
      { \
        TCHAR c[4096]; \
        sprintf_s( c , 4096, str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }

class TimeTracer
{
	std::chrono::system_clock::time_point  start;
	std::string messga;
public:
	TimeTracer( std::string messga = "hoge" )
	{
		this->messga = messga;
		start = std::chrono::system_clock::now();
	}

	~TimeTracer()
	{
		auto end = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		Trace("%s : %f\n" , messga.c_str() , elapsed);
	}
};


struct MQVector
{
public:
	float x, y, z;

	MQVector()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	MQVector(float v)
	{
		x = v;
		y = v;
		z = v;
	}
	MQVector(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	MQVector(const MQPoint& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
	}


	// operator
	MQVector& operator = (const MQVector& p)
	{
		x = p.x; y = p.y; z = p.z; return *this;
	}
	MQVector& operator = (const float& s)
	{
		x = y = z = s; return *this;
	}
	MQVector& operator += (const MQVector& p)
	{
		x += p.x; y += p.y; z += p.z; return *this;
	}
	MQVector& operator += (const float& s)
	{
		x += s; y += s; z += s; return *this;
	}
	MQVector& operator -= (const MQVector& p)
	{
		x -= p.x; y -= p.y; z -= p.z; return *this;
	}
	MQVector& operator -= (const float& s)
	{
		x -= s; y -= s; z -= s; return *this;
	}
	MQVector& operator *= (const MQVector& p)
	{
		x *= p.x; y *= p.y; z *= p.z; return *this;
	}
	MQVector& operator *= (float s)
	{
		x *= s; y *= s; z *= s; return *this;
	}
	MQVector& operator /= (float s)
	{
		x /= s; y /= s; z /= s; return *this;
	}
	MQVector operator + () const { return *this; }
	MQVector operator - () const { return MQVector(-x, -y, -z); }


	friend MQVector operator + (const MQVector& p1, const MQVector& p2)
	{
		return MQPoint(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
	}
	friend MQVector operator + (const MQVector& p, const float& s)
	{
		return MQPoint(p.x + s, p.y + s, p.z + s);
	}
	friend MQVector operator + (const float& s, const MQVector& p)
	{
		return MQPoint(p.x + s, p.y + s, p.z + s);
	}
	friend MQVector operator - (const MQVector& p1, const MQVector& p2)
	{
		return MQPoint(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
	}
	friend MQVector operator - (const MQVector& p, const float& s)
	{
		return MQPoint(p.x - s, p.y - s, p.z - s);
	}
	friend MQVector operator - (const float& s, const MQVector& p)
	{
		return MQPoint(s - p.x, s - p.y, s - p.z);
	}
	friend MQVector operator * (const MQVector& p1, const MQVector& p2)
	{
		return MQVector(p1.x*p2.x, p1.y*p2.y, p1.z*p2.z);
	}
	friend MQVector operator * (const MQVector& p, const float& s)
	{
		return MQVector(p.x*s, p.y*s, p.z*s);
	}
	friend MQVector operator * (const float& s, const MQVector& p)
	{
		return MQVector(p.x*s, p.y*s, p.z*s);
	}
	friend MQVector operator / (const MQVector& p, const float& s)
	{
		return MQPoint(p.x / s, p.y / s, p.z / s);
	}
	friend MQPoint operator / (const MQVector& p, const MQVector& s)
	{
		return MQPoint(p.x / s.x, p.y / s.y, p.z / s.z);
	}
	friend bool operator == (const MQVector& p1, const MQVector& p2)
	{
		return (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z);
	}
	friend bool operator != (const MQVector& p1, const MQVector& p2)
	{
		return (p1.x != p2.x || p1.y != p2.y || p1.z != p2.z);
	}


	MQVector cross(const MQVector& p) const
	{
		return MQPoint(
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x);
	}

	float& operator [](int n) const
	{
		return ((float *)&x)[n];
	}

	operator MQPoint2() const { return MQPoint2(x, y); }
	operator MQPoint() const { return MQPoint(x, y, z); }


	//	float norm() const { return x * x + y * y + z * z; }
	float length() const { return (float)sqrt(x * x + y * y + z * z); }
	void normalize() {
		float a = length();
		*this = (a > 0.0f) ? *this / a : MQVector(0, 0, 0);
	}
	MQVector normalized() const {
		float a = length();
		return (a > 0.0f) ? *this / a : MQVector(0, 0, 0);
	}

	float norm() const
	{
		return (float)sqrt(x * x + y * y + z * z);
	}

	float square_norm() const
	{
		return (x * x + y * y + z * z);
	}

	float dot(const MQVector& p) const
	{
		return x * p.x + y * p.y + z * p.z;
	}

	bool is_valid() const
	{
		return (x != NAN && y != NAN && z != NAN);
	}

	MQVector Lerp(MQVector v, float rate) const
	{
		float a = 1.0f - rate;
		float b = rate;
		return MQVector(x * a + v.x * b , y * a + v.y * b , z * a + v.z * b);
	}
};


class MQRay
{
public:
	MQRay() {}
	MQRay(const MQRay& ray) { origin = ray.origin; vector = ray.vector; }

	MQRay(const MQVector& origin, const MQVector& vector)
	{
		this->origin = origin;
		this->vector = vector.normalized();
	}

	MQRay(MQScene scene, const MQPoint2& position)
	{
		MQPoint p0(position.x, position.y, scene->GetFrontZ());
		MQPoint p1(position.x, position.y, scene->GetFrontZ() + 1);

		this->origin = scene->ConvertScreenTo3D(p0);
		this->vector = MQVector(scene->ConvertScreenTo3D(p1)) - this->origin;
		this->vector.normalize();
	}

	MQRay negative() const
	{
		return MQRay(origin, -vector);
	}


	MQVector origin;
	MQVector vector;

	std::pair<MQVector, MQVector> intersect(const MQRay& ray)
	{
		if (vector.cross(ray.vector).length() < 0.001)
		{
			return std::pair<MQVector, MQVector>(MQVector(NAN), MQVector(NAN));
		}
		auto Dv = vector.dot(ray.vector);
		auto D1 = (ray.origin - origin).dot(vector);
		auto D2 = (ray.origin - origin).dot(ray.vector);

		auto t1 = (D1 - D2 * Dv) / (1.0 - Dv * Dv);
		auto t2 = (D2 - D1 * Dv) / (Dv * Dv - 1.0);

		auto Q1 = origin + vector * t1;
		auto Q2 = ray.origin + ray.vector * t2;

		return std::pair<MQVector, MQVector>(Q1, Q2);
	}

};


static_assert(sizeof(MQVector) == sizeof(MQPoint), "size tigai mangana");


// ジオメトリの隣接情報を構築します。
class MQGeom
{
public:
	struct Vert;
	struct Edge;
	struct Face;
	struct Obj;

	typedef Vert* hVert;
	typedef Edge* hEdge;
	typedef Face* hFace;
	typedef std::shared_ptr<Obj> hObj;

	struct Vert
	{
		int id = -1;
		MQVector co;
		std::vector<hEdge> link_edges;
		std::vector<hFace> link_faces;
		hVert				mirror = NULL;
		Vert() {}
		bool is_border() const
		{
			return std::any_of(link_edges.begin(), link_edges.end(), [](const auto& t) { return t->is_border(); });
		}
	};
	struct Edge
	{
		int id;
		hVert verts[2];
		std::vector<hFace> link_faces;

		Edge() : id(-1) {}
		Edge(int _id, hVert a, hVert b) : id(_id)
		{
			if (a->id < b->id)
			{
				verts[0] = a;
				verts[1] = b;
			}
			else
			{
				verts[1] = a;
				verts[0] = b;
			}
		}

		bool operator == (const Edge& e) const { return e.verts[0] == verts[0] && e.verts[1] == verts[1]; }
		bool operator < (const Edge& e) const
		{
			if (verts[0]->id == e.verts[0]->id)
			{
				return verts[1]->id < e.verts[1]->id;
			}
			return verts[0]->id < e.verts[0]->id;
		}

		bool is_border() const
		{
			return link_faces.size() <= 1;
		}

		hVert other_vert(const hVert v)
		{
			if ( verts[0] == v )
			{
				return verts[1];
			}
			else if(verts[1] == v)
			{
				return verts[0];
			}
			return NULL;
		}
	};
	struct Face
	{
		int id = -1;
		std::vector<hVert> verts;
		std::vector<hEdge> link_edges;
		Face(int _id = -1) : id(_id) {}

		bool is_front( MQScene scene )
		{
			int num = verts.size();
			MQPoint* sp = (MQPoint*)alloca( sizeof(MQPoint) * num );
			for (int i = 0; i<num; i++) {
				sp[i] = scene->Convert3DToScreen( this->verts[i]->co );

				// 視野より手前にあれば表とみなさない
				if (sp[i].z <= 0) return false;
			}

			// 法線の向きで調べる
			if (num >= 3) {
				if ((sp[1].x - sp[0].x) * (sp[2].y - sp[1].y) - (sp[1].y - sp[0].y) * (sp[2].x - sp[1].x) >= 0) {
					return true;
				}
				else if (num >= 4) {
					if ((sp[2].x - sp[0].x) * (sp[3].y - sp[2].y) - (sp[2].y - sp[0].y) * (sp[3].x - sp[2].x) >= 0) {
						return true;
					}
				}
			}
			else if (num > 0) {
				return true;
			}

			return false;
		}
	};

	struct Obj
	{
		MQObject obj;
		std::vector<MQPoint> cos;
		std::vector<Vert> verts;
		std::vector<Edge> edges;
		std::vector<Face> faces;

		static hObj create(MQObject obj)
		{
			return hObj(new Obj(obj));
		}

		const Edge* find(Vert* a, Vert* b) const
		{
			Edge edge(-1, a, b);
			for (const auto& e : edges)
			{
				if (e == edge)
				{
					return &e;
				}
			}
			return NULL;
		}

		Vert* find_mirror(Vert* vert , float sqrt_dist = 0.000000001f )
		{
			if (vert->mirror == NULL)
			{
				auto mco = MQVector(-vert->co.x, vert->co.y, vert->co.z);
				for (auto& v : verts)
				{
					float d = (v.co - mco).square_norm();
					if (d < sqrt_dist)
					{
						vert->mirror = &v;
						v.mirror = vert;
						break;
					}
				}
			}
			return vert->mirror;
		}

	private:
		Obj(MQObject obj)
		{
			cos.clear();
			verts.clear();
			edges.clear();
			faces.clear();
			this->obj = obj;

			//頂点の取得
			int vcnt = obj->GetVertexCount();
			cos = std::vector<MQPoint>(vcnt);
			obj->GetVertexArray(cos.data());

			verts = std::vector<Vert>(vcnt);
			for (int vi = 0; vi < vcnt; vi++)
			{
				verts[vi].id = vi;
				verts[vi].co = cos[vi];
			}

			auto fcnt = obj->GetFaceCount();
			faces = std::vector<Face>(fcnt);
			for (int fi = 0; fi < fcnt; fi++)
			{
				auto pcnt = obj->GetFacePointCount(fi);
				int points[MAX_FACE_VERT];// = (int*)alloca(sizeof(int) * pcnt);
				obj->GetFacePointArray(fi, points);

				faces[fi].id = fi;
				faces[fi].verts.reserve(4);
				for (int pi = 0; pi < pcnt; pi++)
				{
					int a = points[pi];
					faces[fi].verts.push_back(&verts[a]);
				}
			}

			std::map<Edge, std::vector<Face*> > tmp_edges;
			for (int fi = 0; fi < fcnt; fi++)
			{
				Face* face = &faces[fi];
				//Vert.link_faces構築
				for (auto vert : face->verts)
				{
					vert->link_faces.push_back(face);
				}

				//Edge構築
				auto pcnt = face->verts.size();

				if (pcnt == 2)
				{
					auto a = face->verts[0];
					auto b = face->verts[1];
					Edge edge(-1, a, b);
					tmp_edges[edge].push_back(face);
				}
				else
				{
					for (int pi = 0; pi < pcnt; pi++)
					{
						auto a = face->verts[pi];
						auto b = face->verts[(pi + 1) % pcnt];
						Edge edge(-1, a, b);
						tmp_edges[edge].push_back(face);
					}
				}
			}

			//Edge構築
			edges.resize(edges.size());
			for (auto it = tmp_edges.begin(); it != tmp_edges.end(); ++it)
			{
				Edge edge(edges.size(), it->first.verts[0], it->first.verts[1]);
				edge.link_faces = it->second;
				edges.push_back(edge);
			}

			for (int ie = 0; ie < edges.size(); ie++)
			{
				Edge* edge = &edges[ie];
				for (auto face : edge->link_faces)
				{
					face->link_edges.push_back(edge);
				}
				edge->verts[0]->link_edges.push_back(edge);
				edge->verts[1]->link_edges.push_back(edge);
			}
		}
	};

	hObj obj = NULL;

	bool Update( MQObject mq_obj)
	{
		if (obj == NULL)
		{
			obj = Obj::create(mq_obj);
			return true;
		}
		return false;
	}

	void Clear()
	{
		this->obj = NULL;
	}
};

class MQBorderComponent
{
public:
	bool update = false;
	std::vector<MQGeom::Edge*> edges;
	std::vector<MQGeom::Vert*> verts;
	void Update(MQScene scene, MQGeom::hObj obj)
	{
		if (!update)
		{
			// ボーダーエッジを抽出
			edges.clear();
			edges.reserve(obj->edges.size());
			for (auto& edge : obj->edges)
			{
				if (edge.is_border())
				{

					if (scene == NULL || edge.link_faces[0]->is_front(scene))
					{
						edges.push_back(&edge);
					}
				}
			}

			//ボーダー頂点の抽出
			verts.clear();
			verts.reserve(edges.size() * 2);

			std::set<MQGeom::Vert*> vts;
			for (auto edge : edges)
			{
				vts.insert(edge->verts[0]);
				vts.insert(edge->verts[1]);
			}

			verts = std::vector<MQGeom::Vert*>(vts.begin(), vts.end());
			update = true;
		}
	}

	void Clear()
	{
		edges.clear();
		verts.clear();
		update = false;
	}
};

class MQSceneCache
{
public :
	class Scene
	{
	public:
		MQObject obj;
		std::vector<MQPoint> coords;
		std::vector<bool> in_screen;

		Scene() {}
		Scene(const Scene& screen)
		{
			obj = screen.obj;
			coords = screen.coords;
			in_screen = screen.in_screen;
		}

		Scene(MQScene scene, MQGeom::hObj obj)
		{
			this->obj = obj->obj;

			coords = std::vector<MQPoint>(obj->verts.size());
			in_screen = std::vector<bool>(obj->verts.size());
			for (int i = 0; i < obj->verts.size(); i++)
			{
				float w = 0.0f;
				coords[i] = scene->Convert3DToScreen(obj->cos[i], &w);
				in_screen[i] = w > 0;
			}
		}

		void UpdateVert(MQScene scene, int index, const MQPoint& p)
		{
			float w = 0.0f;
			coords[index] = scene->Convert3DToScreen( p , &w);
			in_screen[index] = w > 0;
		}
	};

	std::map<MQScene, std::shared_ptr< Scene> > scenes;

	std::shared_ptr< Scene> Get(MQScene scene, MQGeom::hObj obj)
	{
		if (scenes.find(scene) == scenes.end())
		{
			scenes[scene] = std::shared_ptr< Scene>( new Scene(scene, obj ) );
		}
		return scenes[scene];
	}


	void Clear()
	{
		scenes.clear();
	}
};

typedef acc::BVHTree< int, MQVector> MQBVHTree;

class MQSnap
{
public:
	class Tree
	{
	public:
		std::shared_ptr<MQBVHTree> bvh_tree;
		std::shared_ptr<std::vector<int>> triangle_map;

		Tree()
		{
		}

		Tree(const Tree& tree)
		{
			bvh_tree = tree.bvh_tree;
			triangle_map = tree.triangle_map;
		}

		Tree(MQDocument doc, MQObject obj)
		{
			std::vector<MQVector> verts(obj->GetVertexCount());
			obj->GetVertexArray((MQPoint *)verts.data());

			auto fcnt = obj->GetFaceCount();
			std::vector<int> triangles;
			triangles.reserve(fcnt * 2);
			triangle_map = std::shared_ptr<std::vector<int>>(new std::vector<int>());
			triangle_map->reserve(fcnt * 3 * 2);
			{
				TimeTracer timer;
				for (int fi = 0; fi < fcnt; fi++)
				{
					int pcnt = obj->GetFacePointCount(fi);
					if (pcnt >= 3)
					{
						int triCount = (pcnt - 2);
						int triSize = triCount * 3;
						int is[MAX_FACE_VERT]; //= (int*)alloca(sizeof(int) * pcnt);
						int tris[MAX_FACE_VERT];// = (int*)alloca(sizeof(int) * triSize);
						MQPoint points[MAX_FACE_VERT]; // = (MQPoint*)alloca(sizeof(MQPoint) * pcnt);
						obj->GetFacePointArray(fi, is);
						for (int i = 0; i < pcnt; i++)
						{
							points[i] = verts[is[i]];
						}

						doc->Triangulate(points, pcnt, tris, triSize);

						for (int it = 0; it < triSize; it++)
						{
							auto x = tris[it];
							triangles.push_back(is[x]);
						}
						for (int it = 0; it < triCount; it++)
						{
							triangle_map->push_back(fi);
						}
					}
				}
			}

			{
				TimeTracer timer;
				bvh_tree = MQBVHTree::create(triangles, verts, std::thread::hardware_concurrency());
			}
		}
	};

	MQSnap(MQDocument doc)
	{
		Update(doc);
	}

	MQSnap() {}
	MQSnap(MQSnap& orig) : trees(orig.trees) {  }

	void Update(MQDocument doc)
	{
		std::map<MQObject, std::shared_ptr<Tree> > new_trees;
		for (int io = 0; io < doc->GetObjectCount(); io++)
		{
			auto obj = doc->GetObject(io);

			if (obj->GetLocking() == TRUE && obj->GetVisible() != 0)
			{
				if (trees.find(obj) != trees.end())
				{
					new_trees[obj] = trees[obj];
				}
				else
				{
					new_trees[obj] = std::shared_ptr<Tree>( new Tree(doc, obj) );
				}
			}
		}
		trees = new_trees;
	}

	struct Hit
	{
		bool is_hit = false;
		int idx = -1;
		MQPoint position;
		float t;
	};

	Hit intersect(const MQRay& mqray) const
	{
		Hit result;
		result.t = std::numeric_limits<float>::max();
		result.position = mqray.origin;
		result.is_hit = false;

		float tmin = std::numeric_limits<float>::max();
		for (auto& tree : trees)
		{
			acc::Ray<MQVector> ray;
			ray.origin = mqray.origin;
			ray.dir = mqray.vector;
			ray.tmax = tmin;
			ray.tmin = 0.0f;
			MQBVHTree::Hit hit;
			if (tree.second->bvh_tree->intersect(ray, &hit))
			{
				result.position = ray.origin + ray.dir * hit.t;
				result.t = hit.t;
				result.idx = hit.idx;
				result.is_hit = true;
				tmin = hit.t;
			}
		}

		return result;
	}

	Hit colsest_point( const MQVector& p )
	{
		Hit hit;
		hit.t = std::numeric_limits<float>::max();
		hit.position = p;
		hit.is_hit = false;
		for (auto& tree : trees)
		{
			auto r = tree.second->bvh_tree->closest_point(p, hit.t);
			if (r.second < hit.t)
			{
				hit.position = r.first;
				hit.t = r.second;
				hit.is_hit = true;
			}
		}
		return hit;
	}

	bool check_view(MQScene scene, const MQPoint& pos, float thrdshold = 0.01f) const
	{
		MQVector screen_pos = scene->Convert3DToScreen(pos);

		auto view_ray = MQRay(scene, screen_pos);
		auto ray = MQRay(pos, view_ray.vector);
		MQPoint vp = view_ray.origin;
		MQSnap::Hit hitV = intersect(view_ray);
		if (!hitV.is_hit) return true;
		float d0 = (pos - vp).abs();
		float d1 = (hitV.position - vp).abs();
		if ( d0 < d1 + thrdshold)
		{
			return true;
		}

		auto hit0 = intersect(ray.negative());
		auto hit1 = intersect(ray);

		if (hit0.t > hit1.t) return false;

		if( hit0.idx == hitV.idx ) return true;

		auto d2 = hitV.t + hit0.t;

		return abs(d0-d2) < thrdshold;
	}

	std::map<MQObject, std::shared_ptr<Tree> > trees;
};



MQPoint2 IntersectLineAndLinePos(const MQPoint &p1, const MQPoint &p2, const MQPoint &p3, const MQPoint &p4)
{
	auto det = (p1.x - p2.x) * (p4.y - p3.y) - (p4.x - p3.x) * (p1.y - p2.y);
	auto t = ((p4.y - p3.y) * (p4.x - p2.x) + (p3.x - p4.x) * (p4.y - p2.y)) / det;
	auto x = t * p1.x + (1.0 - t) * p2.x;
	auto y = t * p1.y + (1.0 - t) * p2.y;
	return MQPoint2(x, y);
}


int IntersectLineAndLine(const MQPoint &p1, const MQPoint &p2, const MQPoint &p3, const MQPoint &p4)
{
	float s, t;
	s = (p1.x - p2.x) * (p3.y - p1.y) - (p1.y - p2.y) * (p3.x - p1.x);
	t = (p1.x - p2.x) * (p4.y - p1.y) - (p1.y - p2.y) * (p4.x - p1.x);
	if (s * t > 0)
		return false;

	s = (p3.x - p4.x) * (p1.y - p3.y) - (p3.y - p4.y) * (p1.x - p3.x);
	t = (p3.x - p4.x) * (p2.y - p3.y) - (p3.y - p4.y) * (p2.x - p3.x);
	if (s * t > 0)
		return false;
	return true;
}


bool PointInTriangle(const MQPoint& p, const MQPoint& v1, const MQPoint& v2, const MQPoint& v3)
{
#define CROSS(p1,p2,p3) ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y))
	bool b1 = CROSS(p, v1, v2) < 0.0f;
	bool b2 = CROSS(p, v2, v3) < 0.0f;
	bool b3 = CROSS(p, v3, v1) < 0.0f;
	return ((b1 == b2) && (b2 == b3));
}

bool PointInQuad(const MQPoint& p, const MQPoint& v0, const MQPoint& v1, const MQPoint& v2, const MQPoint& v3)
{
	auto r0 = PointInTriangle(p, v0, v1, v2);
	if (r0) return true;
	auto r1 = PointInTriangle(p, v2, v3, v0);
	return r1;
}

#pragma once
#include "Math.h"
#include <memory>
#include <unordered_map>
#include <queue>

class BezierCurve {
public:
	BezierCurve() {}
	BezierCurve(const std::vector<Vertex>& controlPoints) : controlPoints(controlPoints) {}

	Vertex GetCurvePoint(float t) {
		return GetCurvePoint(controlPoints, t);
	}

	void DegreeElevation(size_t elevation) {
		std::vector<Vertex> temp;
		temp = controlPoints;
		for (size_t e = 0; e < elevation; e++) {
			controlPoints.clear();
			controlPoints.push_back(temp[0]);
			int n = temp.size();
			for (size_t i = 1; i < n; i++) {
				float ratio = (float)i / n;
				controlPoints.push_back(VertexInterplote(temp[i], temp[i - 1], ratio));
			}
			controlPoints.push_back(temp[n - 1]);
			temp = controlPoints;
		}
	}

	void GetControlPoints(std::vector<Vertex>& controlPoints) {
		controlPoints = this->controlPoints;
	}

private:
	Vertex GetCurvePoint(const std::vector<Vertex>& points, float t) {
		size_t pointCount = points.size();
		std::vector<Vertex> newPoints(pointCount - 1);
		for (size_t i = 0; i < pointCount - 1; i++)
			newPoints[i] = VertexInterplote(points[i], points[i + 1], t);

		if (pointCount == 2)
			return newPoints[0];
		else
			return GetCurvePoint(newPoints, t);
	}
	
	std::vector<Vertex> controlPoints;
};

class BSpline {
public:
	BSpline(const std::vector<Vertex>& controlPoints, const std::vector<float>& knotVector)
		: controlPoints(controlPoints), knotVector(knotVector) {
		n = controlPoints.size() - 1;
		order = knotVector.size() - n - 1;
	}

	void AddKnot(float value) {
		size_t index = knotVector.size() - 1;
		bool found = false;
		for (size_t i = 0; i < knotVector.size(); i++) {
			if (value < knotVector[i]) {
				index = i - 1;
				found = true;
				break;
			}
		}

		n++;
		std::vector<Vertex> newPoints(n + 1);
		for (size_t i = 0; i <= index - order + 1; i++) {
			newPoints[i] = controlPoints[i];
		}
		for (size_t i = index - order + 2; i <= index; i++) {
			float basisFactor = BasisFactor(i, 1, value);
			newPoints[i].position = (1.0f - basisFactor) * controlPoints[i - 1].position
				+ basisFactor * controlPoints[i].position;
			newPoints[i].color = (1.0f - basisFactor) * controlPoints[i - 1].color
				+ basisFactor * controlPoints[i].color;
		}
		for (size_t i = index + 1; i <= n; i++) {
			newPoints[i] = controlPoints[i - 1];
		}

		if (found)
			knotVector.insert(knotVector.begin() + index + 1, value);
		else
			knotVector.push_back(value);
		controlPoints = newPoints;
	}

	Vertex GetCurvePoint(float t) {
		size_t j = std::distance(knotVector.begin(), std::upper_bound(knotVector.begin(), knotVector.end(), t)) - 1;
		std::vector<Vertex> newPoints;
		newPoints.insert(newPoints.end(), controlPoints.begin() + (j + 1 - order), controlPoints.begin() + (j + 1));
		return GetCurvePoint(newPoints, t, j + 1 - order);
	}

private:
	Vertex GetCurvePoint(const std::vector<Vertex>& points, float t, size_t index) {
		if (points.size() == 1) return points[0];
		size_t r = order - points.size() + 1;
		index++;

		std::vector<Vertex> newPoints;
		for (size_t i = 0; i < points.size() - 1; i++) {
			Vertex newPoint;
			float basisFactor = BasisFactor(index + i, r, t);
			newPoint.position = (1.0f - basisFactor) * points[i].position + basisFactor * points[i + 1].position;
			newPoint.color = (1.0f - basisFactor) * points[i].color + basisFactor * points[i + 1].color;
			newPoints.push_back(newPoint);
		}
		return GetCurvePoint(newPoints, t, index);
	}

	float BasisFactor(size_t i, size_t r, float t) {
		if (knotVector[i + order - r] - knotVector[i] <= 1e-6f) return 0.0f;
		return (t - knotVector[i]) / (knotVector[i + order - r] - knotVector[i]);
	}

	std::vector<Vertex> controlPoints;
	std::vector<float> knotVector;
	size_t n, order;
};

class BezierSurface {
public:
	BezierSurface() {}
	BezierSurface(Vector2f size, Vector2i tess) : size(size), tess(tess) {
		float horizontal = size.x / tess.x;
		float vertical = size.y / tess.y;
		for (int i = 0; i <= tess.x; i++) {
			float x = -size.x / 2.0f + horizontal * i;
			std::vector<Vertex> controlPoints;
			for (int j = 0; j <= tess.y; j++) {
				float y = -size.y / 2.0f + vertical * j;
				Vertex point;
				point.position = Vector4f(x, y, powf(sinf(j * 1.0f / M_PI), 10.0f), 1.0f);
				point.color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
				controlPoints.push_back(point);
			}
			curves.push_back(BezierCurve(controlPoints));
		}
	}

	void GetControlCurve(std::vector<BezierCurve>& curves) {
		curves = this->curves;
	}

	void GetSurfaceCurve(float t, BezierCurve& curve) {
		std::vector<Vertex> controlPoints;
		for (auto& curve : curves) {
			controlPoints.push_back(curve.GetCurvePoint(t));
		}
		curve = BezierCurve(controlPoints);
	}

private:
	std::vector<BezierCurve> curves;
	Vector2f size;
	Vector2i tess;
};

class TriangulateSurface {
public:
	TriangulateSurface() {}
	TriangulateSurface(const std::vector<Vertex>& vertices, size_t tess) : vertices(vertices) {
		controlPoints.resize(tess + 1);
		for (int i = tess; i >= 0; i--) {
			float height = 3.0f * powf(sinf(i * 1.0f / M_PI), 10.0f);
			controlPoints[i].resize(tess - i + 1);
			for (int j = tess - i; j >= 0; j--) {
				int k = tess - i - j;
				Vertex controlPoint;
				controlPoint.position =
					(float)i / tess * vertices[0].position +
					(float)j / tess * vertices[1].position +
					(float)k / tess * vertices[2].position;
				controlPoint.position.y += height;
				controlPoint.color =
					(float)i / tess * vertices[0].color +
					(float)j / tess * vertices[1].color +
					(float)k / tess * vertices[2].color;
				controlPoints[i][j] = controlPoint;
			}
		}
	}
	TriangulateSurface(const std::vector<Vertex>& vertices) : vertices(vertices) {
		controlPoints.resize(4);
		std::vector<Vertex> newPoints;
		for (size_t i = 0; i < 3; i++) {
			Vertex p = VertexInterplote(vertices[i], vertices[(i + 1) % 3], 1.0f / 3.0f);
			float distance = Dot((p.position - vertices[i].position).GetVector3f(), vertices[i].normal);
			p.position = Vector4f((p.position.GetVector3f() - distance * vertices[i].normal), 1.0f);
			newPoints.push_back(p);

			p = VertexInterplote(vertices[i], vertices[(i + 1) % 3], 2.0f / 3.0f);
			distance = Dot((p.position - vertices[i + 1].position).GetVector3f(), vertices[i + 1].normal);
			p.position = Vector4f((p.position.GetVector3f() - distance * vertices[i + 1].normal), 1.0f);
			newPoints.push_back(p);
		}
		Vertex center;
		for (auto& p : newPoints)
			center.position = center.position + 1.0f / 6.0f * p.position;
		for (auto& v : vertices) {
			center.position = center.position + 1.0f / 3.0f * v.position;
			center.color = center.color + 1.0f / 3.0f * v.color;
		}
		center.position = 0.5f * center.position;

		controlPoints = {
			{ vertices[0] },
			{ newPoints[5], newPoints[0] },
			{ newPoints[4], center, newPoints[1] },
			{ vertices[2], newPoints[3], newPoints[2], vertices[1] }
		};
	}

	Vertex GetSurfacePoint(float u, float v, float w) {
		int n = controlPoints.size() - 1;
		Vertex point;
		for (int i = n; i >= 0; i--) {
			for (int j = n - i; j >= 0; j--) {
				int k = n - i - j;
				float bernstein = Factorial(n) / (Factorial(i) * Factorial(j) * Factorial(k))
					* powf(u, i) * powf(v, j) * powf(w, k);
				point.position = point.position + bernstein * controlPoints[i][j].position;
				point.color = point.color + bernstein * controlPoints[i][j].color;
			}
		}
		return point;
	}

	Vertex GetSurfacePointRecursive(float u, float v, float w) {
		return GetSurfacePointRecursive(controlPoints, u, v, w);
	}

private:
	Vertex GetSurfacePointRecursive(const std::vector<std::vector<Vertex>>& referencePoints, float u, float v, float w) {
		int n = referencePoints.size() - 1;
		std::vector<std::vector<Vertex>> newPoints(n);
		n--;
		for (int i = n; i >= 0; i--) {
			newPoints[i].resize(n - i + 1);
			for (int j = n - i; j >= 0; j--) {
				Vertex point;
				point.position = u * referencePoints[i + 1][j].position
					+ v * referencePoints[i][j + 1].position
					+ w * referencePoints[i][j].position;
				point.color = u * referencePoints[i + 1][j].color
					+ v * referencePoints[i][j + 1].color
					+ w * referencePoints[i][j].color;
				newPoints[i][j] = point;
			}
		}
		if (n == 0)
			return newPoints[0][0];
		else
			return GetSurfacePointRecursive(newPoints, u, v, w);
	}

	std::vector<Vertex> vertices;
	std::vector<std::vector<Vertex>> controlPoints;
};

class HalfEdgeStructure {
public:
	struct HalfEdge;
	struct Face;
	struct Vertex {
		size_t id;
		::Vertex data;
		HalfEdge* edge;
	};
	struct HalfEdge {
		size_t id;
		Vertex* vertex;
		Face* face;
		HalfEdge* opposite;
		HalfEdge* next;
	};
	struct EdgeKey {
		EdgeKey() {}
		EdgeKey(size_t v0, size_t v1) : v0(v0), v1(v1) {}
		struct Hash {
			size_t operator()(const EdgeKey& key)const {
				return std::hash<size_t>()(key.v0) ^ std::hash<size_t>()(key.v1);
			}
		};
		bool operator==(const EdgeKey& key)const {
			return v0 == key.v0 && v1 == key.v1;
		}
		size_t v0, v1;
	};
	struct Face {
		size_t id;
		HalfEdge* edge;
		Vector3f normal;
	};
	HalfEdgeStructure() {}

	std::vector<Vertex*> GetVertices()const {
		std::vector<Vertex*> vertices;
		for (auto& vertex : this->vertices) {
			if (vertex) vertices.push_back(vertex.get());
		}
		return vertices;
	}
	std::vector<HalfEdge*> GetHalfEdges()const {
		std::vector<HalfEdge*> edges;
		for (auto& edge : halfEdges) {
			if (edge) edges.push_back(edge.get());
		}
		return edges;
	}
	std::vector<Face*> GetFaces()const {
		std::vector<Face*> faces;
		for (auto& face : this->faces) {
			if (face) faces.push_back(face.get());
		}
		return faces;
	}

	Vertex* AddVertex(const ::Vertex& vertex) {
		auto vert = std::make_unique<Vertex>();
		vert->data = vertex;
		vert->id = vertices.size();
		vertices.push_back(std::move(vert));
		return vertices.rbegin()->get();
	}

	Face* AddFace(Vertex** vertices, size_t num) {
		auto face = std::make_unique<Face>();
		std::vector<HalfEdge*> edges(num);

 		for (size_t i = 0; i < num; i++) {
			edges[i] = AddEdge(vertices[i % num], vertices[(i + 1) % num]);
		}
		for (size_t i = 0; i < num; i++) {
			edges[i]->next = edges[(i + 1) % num];
			edges[i]->face = face.get();
		}
		face->edge = edges[0];
		face->normal = Cross((vertices[1]->data.position - vertices[0]->data.position).GetVector3f(),
			(vertices[2]->data.position - vertices[1]->data.position).GetVector3f()).Normalize();

		face->id = faces.size();
		faces.push_back(std::move(face));
		return faces.rbegin()->get();
	}

	void DeleteVertex(Vertex* vertex) {
		std::vector<size_t> deleteEdgeID;
		auto neighborEdge = GetEdgesFromVertex(vertex);
		for (auto& edge : neighborEdge) {
			deleteEdgeID.push_back(edge->id);
			deleteEdgeID.push_back(edge->opposite->id);
			edgeMap.erase(EdgeKey(vertex->id, edge->vertex->id));
			edgeMap.erase(EdgeKey(edge->vertex->id, vertex->id));
		}

		auto faces = GetFacesFromVertex(vertex);
		for (auto& face : faces)
			this->faces[face->id].reset(nullptr);

		std::vector<Vertex*> neighbors;
		HalfEdge* edge = vertex->edge;
		neighbors.push_back(edge->vertex);
		size_t primeID = edge->vertex->id;
		bool newFace = false;
		while (true) {
			edge = edge->next;
			if (!edge) break;
			if (edge->vertex->id == primeID) {
				newFace = true;
				break;
			}
			if (edge->vertex->id == vertex->id) {
				edge = edge->opposite;
				continue;
			}
			neighbors.push_back(edge->vertex);
		}
		//防止有顶点失去指向的边
		for (size_t i = 0; i < neighbors.size(); i++) {
			neighbors[i]->edge = edgeMap[EdgeKey(neighbors[i]->id, neighbors[(i + 1) % neighbors.size()]->id)];
		}
		if (newFace) {
			Face* thisFace = AddFace(neighbors.data(), neighbors.size());
			auto newVert = GetVerticesFromFace(thisFace);
		}

		for (auto& id : deleteEdgeID)
			halfEdges[id].reset(nullptr);
		vertices[vertex->id].reset(nullptr);
	}

	Vertex* JoinVertex(Vertex* v0, Vertex* v1, ::Vertex v) {
		auto newVert = AddVertex(v);
		for (auto& edge : GetEdgesFromVertex(v0)) {
			if (edge->vertex->id != v1->id) {
				edgeMap.erase(EdgeKey(v0->id, edge->vertex->id));
				edgeMap.erase(EdgeKey(edge->vertex->id, v0->id));
				edge->opposite->vertex = newVert;
				edgeMap[EdgeKey(newVert->id, edge->vertex->id)] = edge;
				edgeMap[EdgeKey(edge->vertex->id, newVert->id)] = edge->opposite;
			}
		}
		for (auto& edge : GetEdgesFromVertex(v1)) {
			if (edge->vertex->id != v0->id) {
				edgeMap.erase(EdgeKey(v1->id, edge->vertex->id));
				edgeMap.erase(EdgeKey(edge->vertex->id, v1->id));
				edge->opposite->vertex = newVert;
				edgeMap[EdgeKey(newVert->id, edge->vertex->id)] = edge;
				edgeMap[EdgeKey(edge->vertex->id, newVert->id)] = edge->opposite;
			}
		}
		
		HalfEdge* deleteEdge = edgeMap[EdgeKey(v0->id, v1->id)];
		auto face = deleteEdge->face;
		if (face) {
			if (GetVerticesFromFace(face).size() > 3) {
				std::vector<Vertex*> faceVert;
				for (auto& v : GetVerticesFromFace(face)) {
					if (v->id != v0->id && v->id != v1->id)
						faceVert.push_back(v);
				}
				AddFace(faceVert.data(), faceVert.size());
			}
			faces[face->id].reset(nullptr);
		}
		face = deleteEdge->opposite->face;
		if (face) {
			if (GetVerticesFromFace(face).size() > 3) {
				std::vector<Vertex*> faceVert;
				for (auto& v : GetVerticesFromFace(face)) {
					if (v->id != v0->id && v->id != v1->id)
						faceVert.push_back(v);
				}
				AddFace(faceVert.data(), faceVert.size());
			}
			faces[face->id].reset(nullptr);
		}

		edgeMap.erase(EdgeKey(v0->id, v1->id));
		edgeMap.erase(EdgeKey(v1->id, v0->id));
		halfEdges[deleteEdge->opposite->id].reset(nullptr);
		halfEdges[deleteEdge->id].reset(nullptr);

		vertices[v0->id].reset(nullptr);
		vertices[v1->id].reset(nullptr);

		return newVert;
	}

	std::vector<HalfEdge*> GetEdgesFromVertex(const Vertex* vertex)const {
		std::vector<HalfEdge*> edges;
		HalfEdge* edge = vertex->edge;
		edges.push_back(edge);
		size_t primeID = edge->vertex->id;
		bool isBoundarty = true;

		while (true) {
			edge = edge->next;
			if (!edge) break;
			if (edge->vertex->id == primeID) {
				isBoundarty = false;
				break;
			}
			if (edge->vertex->id == vertex->id) {
				edge = edge->opposite;
				edges.push_back(edge);
			}
		}
		if (isBoundarty) {
			edge = vertex->edge->opposite;
			while (true) {
				edge = edge->next;
				if (!edge) break;
				edges.push_back(edge);
				if (edge->vertex->id = vertex->id) {
					edge = edge->opposite;
					edges.push_back(edge);
				}
			}
		}
		return edges;
	}

	std::vector<Vertex*> GetVerticesFromFace(const Face* face)const {
		std::vector<Vertex*> vertices;
		HalfEdge* edge = face->edge;
		size_t primeID = edge->vertex->id;
		do {
			edge = edge->next;
			vertices.push_back(edge->vertex);
		} while (edge->vertex->id != primeID);
		return vertices;
	}

	std::vector<Vertex*> GetVerticesFromEdge(const HalfEdge* edge)const {
		std::vector<Vertex*> vertices;
		auto faceVert = GetVerticesFromFace(edge->face);
		for (size_t i = 0; i < faceVert.size(); i++) {
			if (faceVert[i]->id == edge->vertex->id) {
				vertices.push_back(faceVert[i == 0 ? faceVert.size() - 1 : i - 1]);
				vertices.push_back(faceVert[i]);
			}
		}
		return vertices;
	}

	std::vector<Face*> GetFacesFromVertex(const Vertex* vertex)const {
		std::vector<Face*> faces;
		auto edges = GetEdgesFromVertex(vertex);
		for (auto& edge : edges) {
			faces.push_back(edge->face);
		}
		return faces;
	}

	std::vector<Vertex*> GetNeighborVertices(const Vertex* vertex)const {
		std::vector<Vertex*> neighbors;
		auto faces = GetFacesFromVertex(vertex);
		for (auto& face : faces) {
			auto vertices = GetVerticesFromFace(face);
			for (auto& vert : vertices) {
				bool isInsert = true;
				for (auto& neighbor : neighbors) {
					if (vert->id == neighbor->id)
						isInsert = false;
				}
				if (isInsert && vert->id != vertex->id) {
					neighbors.push_back(vert);
				}
			}
		}
		return neighbors;
	}

private:
	HalfEdge* AddEdge(Vertex* v0, Vertex* v1) {
		EdgeKey key(v0->id, v1->id);
		if (edgeMap.find(key) != edgeMap.end()) {
			return edgeMap[key];
		}

		auto edge = std::make_unique<HalfEdge>();
		auto edge_op = std::make_unique<HalfEdge>();

		edge->vertex = v1;
		edge->opposite = edge_op.get();
		edge->id = halfEdges.size();
		v0->edge = edge.get();
		edge_op->vertex = v0;
		edge_op->opposite = edge.get();
		edge_op->id = halfEdges.size() + 1;

		halfEdges.push_back(std::move(edge));
		halfEdges.push_back(std::move(edge_op));

		edgeMap[EdgeKey(v0->id, v1->id)] = (halfEdges.rbegin() + 1)->get();
		edgeMap[EdgeKey(v1->id, v0->id)] = halfEdges.rbegin()->get();

		return edgeMap[EdgeKey(v0->id, v1->id)];
	}

	std::vector<std::unique_ptr<Vertex>> vertices;
	std::vector<std::unique_ptr<Face>> faces;
	std::vector<std::unique_ptr<HalfEdge>> halfEdges;
	std::unordered_map<EdgeKey, HalfEdge*, EdgeKey::Hash> edgeMap;
};

std::unique_ptr<HalfEdgeStructure> LoopSubdivision(HalfEdgeStructure* originMesh) {
	auto resultMesh = std::make_unique<HalfEdgeStructure>();

	//用于存储新的顶点
	std::vector<HalfEdgeStructure::Vertex*> newVertices;

	//遍历并更新所有的旧顶点
	for (auto& originVertex : originMesh->GetVertices()) {
		Vertex newVertex;

		auto neighborVertices = originMesh->GetNeighborVertices(originVertex);
		size_t n = neighborVertices.size();
		float u = n == 3 ? 3.0f / 16.0f : 3.0f / (8.0f * n);

		newVertex.position = (1.0f - n * u) * originVertex->data.position;
		newVertex.color = (1.0f - n * u) * originVertex->data.color;

		for (auto& neighbor : neighborVertices) {
			newVertex.position = newVertex.position + u * neighbor->data.position;
			newVertex.color = newVertex.color + u * neighbor->data.color;
		}

		auto vert = resultMesh->AddVertex(newVertex);
		newVertices.push_back(vert);
	}

	//建立无序映射表用于存储新顶点和边的映射关系
	std::unordered_map<HalfEdgeStructure::EdgeKey, HalfEdgeStructure::Vertex*, HalfEdgeStructure::EdgeKey::Hash> vertexMap;
	
	//遍历每条半边并建立新的顶点
	for (auto& edge : originMesh->GetHalfEdges()) {
		auto v0 = edge->next->next->vertex;
		auto v1 = edge->vertex;
		HalfEdgeStructure::EdgeKey key(v0->id, v1->id);

		//防止重复建立
		if (vertexMap.find(key) != vertexMap.end()) continue;

		if (!edge->opposite->face) {	//边界插入点
			Vertex newVertex;
			newVertex.position = 0.5f * (v0->data.position + v1->data.position);
			newVertex.color = 0.5f * (v0->data.color + v1->data.color);

			auto vert = resultMesh->AddVertex(newVertex);
			vertexMap[HalfEdgeStructure::EdgeKey(v0->id, v1->id)] = vert;
			vertexMap[HalfEdgeStructure::EdgeKey(v1->id, v0->id)] = vert;
		}
		else {	//内部插入点
			auto v2 = edge->next->vertex;
			auto v3 = edge->opposite->next->vertex;
			Vertex newVertex;
			newVertex.position = 3.0f / 8.0f * (v0->data.position + v1->data.position)
				+ 1.0f / 8.0f * (v2->data.position + v3->data.position);
			newVertex.color = 3.0f / 8.0f * (v0->data.color + v1->data.color)
				+ 1.0f / 8.0f * (v2->data.color + v3->data.color);

			auto vert = resultMesh->AddVertex(newVertex);
			vertexMap[HalfEdgeStructure::EdgeKey(v0->id, v1->id)] = vert;
			vertexMap[HalfEdgeStructure::EdgeKey(v1->id, v0->id)] = vert;
		}
	}

	//遍历每个面片并为所有顶点重新建立拓扑关系（1个面片->4个面片）
	for (auto& face : originMesh->GetFaces()) {
		HalfEdgeStructure::HalfEdge* edges[3];
		edges[0] = face->edge;
		edges[1] = edges[0]->next;
		edges[2] = edges[1]->next;
		
		//将新产生的顶点相连组成一个面片
		HalfEdgeStructure::Vertex* center[3];
		for (size_t i = 0; i < 3; i++) {
			auto key = HalfEdgeStructure::EdgeKey(edges[i]->vertex->id, edges[(i + 2) % 3]->vertex->id);
			center[i] = vertexMap[key];
		}
		resultMesh->AddFace(center, 3);

		//每两个新顶点和一个旧顶点相连组成一个面片
		for (size_t i = 0; i < 3; i++) {
			HalfEdgeStructure::Vertex* triVertex[3];
			triVertex[0] = newVertices[edges[(i + 2) % 3]->vertex->id];
			triVertex[1] = center[i];
			triVertex[2] = center[(i + 2) % 3];
			resultMesh->AddFace(triVertex, 3);
		}
	}

	return std::move(resultMesh);
}

std::unique_ptr<HalfEdgeStructure> CatmullClarkSubdivision(HalfEdgeStructure* originMesh) {
	auto resultMesh = std::make_unique<HalfEdgeStructure>();

	//遍历所有面片并建立面中心点
	std::unordered_map<HalfEdgeStructure::EdgeKey, HalfEdgeStructure::Vertex*, HalfEdgeStructure::EdgeKey::Hash> faceVertMap;
	for (auto& face : originMesh->GetFaces()) {
		Vertex newVertex;

		auto faceVert = originMesh->GetVerticesFromFace(face);
		size_t n = faceVert.size();
		for (auto& v : faceVert) {
			newVertex.position = newVertex.position + v->data.position;
			newVertex.color = newVertex.color + v->data.color;
		}
		newVertex.position = 1.0f / n * newVertex.position;
		newVertex.color = 1.0f / n * newVertex.color;

		auto vert = resultMesh->AddVertex(newVertex);
		for (size_t i = 0; i < n; i++) {
			faceVertMap[HalfEdgeStructure::EdgeKey(faceVert[i]->id, faceVert[(i + 1) % n]->id)] = vert;
		}
	}

	//遍历所有半边并建立边中心点
	std::unordered_map<HalfEdgeStructure::EdgeKey, HalfEdgeStructure::Vertex*, HalfEdgeStructure::EdgeKey::Hash> edgeVertMap;
	for (auto& edge : originMesh->GetHalfEdges()) {
		auto edgeVert = originMesh->GetVerticesFromEdge(edge);
		auto v0 = edgeVert[0];
		auto v1 = edgeVert[1];
		if (edgeVertMap.find(HalfEdgeStructure::EdgeKey(v0->id, v1->id)) != edgeVertMap.end()) continue;

		size_t count = 2;
		Vertex newVertex;
		newVertex.position = v0->data.position + v1->data.position;
		newVertex.color = v0->data.color + v1->data.color;

		//是否存在面片中心点
		if (faceVertMap.find(HalfEdgeStructure::EdgeKey(v0->id, v1->id)) != faceVertMap.end()) {
			newVertex.position = newVertex.position + faceVertMap[HalfEdgeStructure::EdgeKey(v0->id, v1->id)]->data.position;
			newVertex.color = newVertex.color + faceVertMap[HalfEdgeStructure::EdgeKey(v0->id, v1->id)]->data.color;
			count++;
		}
		if (faceVertMap.find(HalfEdgeStructure::EdgeKey(v1->id, v0->id)) != faceVertMap.end()) {
			newVertex.position = newVertex.position + faceVertMap[HalfEdgeStructure::EdgeKey(v1->id, v0->id)]->data.position;
			newVertex.color = newVertex.color + faceVertMap[HalfEdgeStructure::EdgeKey(v1->id, v0->id)]->data.color;
			count++;
		}
		newVertex.position = 1.0f / count * newVertex.position;
		newVertex.color = 1.0f / count * newVertex.color;

		auto vert = resultMesh->AddVertex(newVertex);
		edgeVertMap[HalfEdgeStructure::EdgeKey(v0->id, v1->id)] = vert;
		edgeVertMap[HalfEdgeStructure::EdgeKey(v1->id, v0->id)] = vert;
	}

	//遍历并更新所有的旧顶点
	std::vector<HalfEdgeStructure::Vertex*> newVertices;
	for (auto& vertex : originMesh->GetVertices()) {
		size_t count = 4;
		Vertex newVertex;
		newVertex.position = 4.0f * vertex->data.position;
		newVertex.color = 4.0f * vertex->data.color;

		auto edges = originMesh->GetEdgesFromVertex(vertex);
		for (auto& edge : edges) {
			auto& edgeVert = edgeVertMap[HalfEdgeStructure::EdgeKey(vertex->id, edge->vertex->id)]->data;
			newVertex.position = newVertex.position + 2.0f * edgeVert.position;
			newVertex.color = newVertex.color + 2.0f * edgeVert.color;
			count += 2;

			auto& faceVert = faceVertMap[HalfEdgeStructure::EdgeKey(vertex->id, edge->vertex->id)]->data;
			newVertex.position = newVertex.position + faceVert.position;
			newVertex.color = newVertex.color + faceVert.color;
			count++;
		}
		newVertex.position = 1.0f / count * newVertex.position;
		newVertex.color = 1.0f / count * newVertex.color;

		auto vert = resultMesh->AddVertex(newVertex);
		newVertices.push_back(vert);
	}

	//遍历每个面片并为所有顶点重新建立拓扑关系（n边形面->n个四边形面）
	for (auto& face : originMesh->GetFaces()) {
		auto faceVert = originMesh->GetVerticesFromFace(face);
		auto center = faceVertMap[HalfEdgeStructure::EdgeKey(faceVert[0]->id, faceVert[1]->id)];
		size_t n = faceVert.size();

		for (size_t i = 0; i < n; i++) {
			HalfEdgeStructure::Vertex* vertices[] = {
				newVertices[faceVert[i]->id],
				edgeVertMap[HalfEdgeStructure::EdgeKey(faceVert[i]->id, faceVert[(i + 1) % n]->id)],
				center,
				edgeVertMap[HalfEdgeStructure::EdgeKey(faceVert[i == 0 ? n - 1 : i - 1]->id, faceVert[i]->id)],
			};
			resultMesh->AddFace(vertices, 4);
		}
	}

	return std::move(resultMesh);
}

void EdgeCollapsing(HalfEdgeStructure* originMesh, size_t n) {
	struct EdgeRecord {
		EdgeRecord() {}
		bool operator<(const EdgeRecord& record)const {
			return quadric > record.quadric;
		}
		HalfEdgeStructure::HalfEdge* edge;
		Vertex vertex;
		float quadric;
	};

	//计算面片代价矩阵的代码
	std::vector<Matrix4x4f> faceQuadric;
	for (auto& face : originMesh->GetFaces()) {
		float d = -Dot(face->normal, face->edge->vertex->data.position.GetVector3f());
		faceQuadric.push_back(Matrix4x4f(
			Vector4f(face->normal.x * face->normal.x, face->normal.x * face->normal.y, face->normal.x * face->normal.z, face->normal.x * d),
			Vector4f(face->normal.x * face->normal.y, face->normal.y * face->normal.y, face->normal.y * face->normal.z, face->normal.y * d),
			Vector4f(face->normal.x * face->normal.z, face->normal.y * face->normal.z, face->normal.z * face->normal.z, face->normal.z * d),
			Vector4f(face->normal.x * d, face->normal.y * d, face->normal.z * d, d * d)
		));
	}

	//计算顶点相邻面的代价矩阵之和
	std::vector<Matrix4x4f> vertexQuadric;
	for (auto& vertex : originMesh->GetVertices()) {
		auto faces = originMesh->GetFacesFromVertex(vertex);
		Matrix4x4f Q;
		for (auto& face : faces) {
			Q = Q + faceQuadric[face->id];
		}
		vertexQuadric.push_back(Q);
	}

	std::priority_queue<EdgeRecord> edgeRecords;
	for (auto& edge : originMesh->GetHalfEdges()) {
		auto vertices = originMesh->GetVerticesFromEdge(edge);
		Matrix4x4f quadric = vertexQuadric[vertices[0]->id] + vertexQuadric[vertices[1]->id];
		quadric.a4 = 0.0f; quadric.b4 = 0.0f; quadric.c4 = 0.0f; quadric.d4 = 1.0f;

		EdgeRecord record;
		record.vertex = VertexInterplote(vertices[0]->data, vertices[1]->data, 0.5f);
		auto det = quadric.Determinant();
		if (quadric.Determinant() < 1e-6f) {
			record.vertex.position = 0.5f * (vertices[0]->data.position + vertices[1]->data.position);
		}
		else {
			record.vertex.position = Multiply(Vector4f(0.0f, 0.0f, 0.0f, 1.0f), quadric.Inverse());
		}
		record.quadric = Dot(Multiply(record.vertex.position, quadric), record.vertex.position);
		record.edge = edge;
		edgeRecords.push(record);
	}

	for (size_t i = 0; i < n; i++) {
		auto& record = edgeRecords.top();
		auto v = originMesh->GetVerticesFromEdge(record.edge);
		auto newVert = originMesh->JoinVertex(originMesh->GetVerticesFromEdge(record.edge)[0], record.edge->vertex, record.vertex);
		edgeRecords.pop();
	}
}
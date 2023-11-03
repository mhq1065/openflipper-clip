#pragma once

#include <iostream>
#include <vector>
#define eps 1e-6

namespace myPoly {
	class Vertex;
	
	class Polygon
	{
	public:
		Vertex* first;
		int vertices;
		Vertex* _lastUnprocessed;
		bool _arrayVertices;
		Vertex* _firstIntersect = nullptr;

		Polygon(std::vector<std::pair<double, double>>  p, bool arrayVertices=true);
		void addVertex(const std::pair<double, double>& vertex);
		void insertVertex(Vertex* vertex, Vertex* start, Vertex* end);
		Vertex* getNext(Vertex* c);
		bool hasUnprocessed();
		Vertex* getFirstIntersect();
		std::vector<std::pair<double, double>>  getPoints();
		std::vector<std::vector<std::pair<double, double>> > clip(Polygon* clip, bool sourceForwards, bool clipForwards);
		void show();
	};

	class Vertex {
	public:
		double x, y;
		Vertex* next;
		Vertex* prev;
		Vertex* _corresponding;
		double _distance;
		bool _isEntry;
		bool _isIntersection;
		bool _visited;

		Vertex(std::pair<double, double> n) : x(n.first), y(n.second), next(nullptr), prev(nullptr), _corresponding(nullptr),
			_distance(0.0), _isEntry(true), _isIntersection(false), _visited(false) {};

		Vertex(double x, double y) : x(x), y(y), next(nullptr), prev(nullptr), _corresponding(nullptr),
			_distance(0.0), _isEntry(true), _isIntersection(false), _visited(false) {};
		Vertex createIntersection(double x, double y, double distance);
		void visit();
		bool equals(const Vertex& v);
		bool isInside(Polygon* p);
	};


	class segseg {
	public:
		double x;
		double y;
		double toSource;
		double toClip;

		segseg(Vertex* s1, Vertex* s2, Vertex* c1, Vertex* c2) {
			x = 0.0;
			y = 0.0;
			toSource = 0.0;
			toClip = 0.0;

			double d = (c2->y - c1->y) * (s2->x - s1->x) - (c2->x - c1->x) * (s2->y - s1->y);

			if (abs(d) < eps)
				return;

			toSource = ((c2->x - c1->x) * (s1->y - c1->y) - (c2->y - c1->y) * (s1->x - c1->x)) / d;
			toClip = ((s2->x - s1->x) * (s1->y - c1->y) - (s2->y - s1->y) * (s1->x - c1->x)) / d;

			if (valid()) {
				x = s1->x + toSource * (s2->x - s1->x);
				y = s1->y + toSource * (s2->y - s1->y);
			}
		}

		bool valid() {
			return (0 < toSource && toSource < 1) && (0 < toClip && toClip < 1);
		}
	};

	Vertex* createIntersection(double x, double y, double distance) {
		auto vertex = new Vertex(x, y);
		vertex->_distance = distance;
		vertex->_isIntersection = true;
		vertex->_isEntry = false;
		return vertex;
	}

	inline Polygon::Polygon(std::vector<std::pair<double, double>>  p, bool arrayVertices)
		: first(nullptr), vertices(0), _lastUnprocessed(nullptr), _arrayVertices(arrayVertices) {
		for (auto& point : p) {
			addVertex(point);
		}
	}
	
	void Polygon::addVertex(const std::pair<double, double>& vertex) {
		if (first == nullptr) {
			first = new Vertex(vertex);
			first->next = first;
			first->prev = first;
		}
		else {
			Vertex* next = first;
			Vertex* prev = next->prev;

			next->prev = new Vertex(vertex);
			next->prev->next = next;
			next->prev->prev = prev;
			prev->next = next->prev;
		}
		vertices++;
	}

	inline void Polygon::insertVertex(Vertex* vertex, Vertex* start, Vertex* end) {
		//Vertex* vertex = new Vertex(n);
		Vertex* prev = nullptr;
		Vertex* curr = start;

		while (!curr->equals(*end) && curr->_distance < vertex->_distance) {
			curr = curr->next;
		}

		vertex->next = curr;
		prev = curr->prev;

		vertex->prev = prev;
		prev->next = vertex;
		curr->prev = vertex;

		vertices++;
	}

	Vertex* Polygon::getNext(Vertex* v) {
		auto c = v;
		while (c->_isIntersection) c = c->next;
		return c;
	}
	
	//TODO
	inline bool Polygon::hasUnprocessed() {
		Vertex* v = this->_lastUnprocessed ? this->_lastUnprocessed : this->first;

		do {
			if (v->_isIntersection && !v->_visited) {
				this->_lastUnprocessed = v;
				return true;
			}

			v = v->next;
		} while (!v->equals(*this->first));

		this->_lastUnprocessed = nullptr;
		return false;
	}
	//TODO
	inline Vertex* Polygon::getFirstIntersect() {
		Vertex* v = this->_firstIntersect ? this->_firstIntersect : this->first;

		do {
			if (v->_isIntersection && !v->_visited) break;

			v = v->next;
		} while (!v->equals(*this->first));

		this->_firstIntersect = v;
		return v;
	}
	std::vector<std::pair<double, double>>  Polygon::getPoints() {
		std::vector<std::pair<double, double>>  points;
		Vertex* v = this->first;

		if (this->_arrayVertices) {
			do {
				points.push_back(std::make_pair(v->x, v->y));
				v = v->next;
			} while (v != this->first);
		}
		else {
			do {
				points.push_back(std::make_pair(v->x, v->y));
				v = v->next;
			} while (v != this->first);
		}

		return points;
	}

	//TODO
	inline std::vector<std::vector<std::pair<double, double>> > Polygon::clip(Polygon* clip, bool sourceForwards, bool clipForwards) {
		Vertex* sourceVertex = this->first;
		Vertex* clipVertex = clip->first;
		bool sourceInClip, clipInSource;

		bool isUnion = !sourceForwards && !clipForwards;
		bool isIntersection = sourceForwards && clipForwards;
		bool isDiff = !isUnion && !isIntersection;

		// calculate and mark intersections  
		do {
			if (!sourceVertex->_isIntersection) {
				Vertex* clipVertex = clip->first;

				do {
					if (!clipVertex->_isIntersection) {
						segseg i(
							sourceVertex,
							this->getNext(sourceVertex->next),
							clipVertex,
							clip->getNext(clipVertex->next)
						);

						if (i.valid()) {
							Vertex* sourceIntersection = createIntersection(i.x, i.y, i.toSource);
							Vertex* clipIntersection = createIntersection(i.x, i.y, i.toClip);

							sourceIntersection->_corresponding = clipIntersection;
							clipIntersection->_corresponding = sourceIntersection;

							this->insertVertex(sourceIntersection, sourceVertex, this->getNext(sourceVertex->next));
							clip->insertVertex(clipIntersection, clipVertex, clip->getNext(clipVertex->next));
						}
					}

					clipVertex = clipVertex->next;
				} while (!clipVertex->equals(*clip->first));
			}

			sourceVertex = sourceVertex->next;
		} while (!sourceVertex->equals(*this->first));

		// phase three - construct a list of clipped std::vector<std::pair<double, double>> gons  
		sourceVertex = this->first;
		clipVertex = clip->first;

		sourceInClip = sourceVertex->isInside(clip);
		clipInSource = clipVertex->isInside(this);

		sourceForwards ^= sourceInClip;
		clipForwards ^= clipInSource;

		do {
			if (sourceVertex->_isIntersection) {
				sourceVertex->_isEntry = sourceForwards;
				sourceForwards = !sourceForwards;
			}
			sourceVertex = sourceVertex->next;
		} while (!sourceVertex->equals(*this->first));

		do {
			if (clipVertex->_isIntersection) {
				clipVertex->_isEntry = clipForwards;
				clipForwards = !clipForwards;
			}
			clipVertex = clipVertex->next;
		} while (!clipVertex->equals(*clip->first));

		// phase three - construct a list of clipped std::vector<std::pair<double, double>> gons

		std::vector<std::vector<std::pair<double, double>> > list;

		while (this->hasUnprocessed()) {
			Vertex* current = this->getFirstIntersect();
			Polygon clipped(std::vector<std::pair<double, double>> (),this->_arrayVertices);
			clipped.addVertex(std::make_pair(current->x, current->y));

			do {
				current->visit();

				if (current->_isEntry) {
					do {
						current = current->next;
						clipped.addVertex(std::make_pair(current->x, current->y));
					} while (!current->_isIntersection);
				}
				else {
					do {
						current = current->prev;
						clipped.addVertex(std::make_pair(current->x, current->y));
					} while (!current->_isIntersection);
				}

				current = current->_corresponding;
			} while (!current->_visited);

			list.push_back(clipped.getPoints());
		}

		if (list.empty()) {
			if (isUnion) {
				if (sourceInClip)
					list.push_back(clip->getPoints());
				else if (clipInSource)
					list.push_back(this->getPoints());
				else {
					list.push_back(this->getPoints());
					list.push_back(clip->getPoints());
				}
			}
			else if (isIntersection) {
				if (sourceInClip)
					list.push_back(this->getPoints());
				else if (clipInSource)
					list.push_back(clip->getPoints());
			}
			else {
				if (sourceInClip) {
					list.push_back(clip->getPoints());
					list.push_back(this->getPoints());
				}
				else if (clipInSource) {
					list.push_back(this->getPoints());
					list.push_back(clip->getPoints());
				}
				else
					list.push_back(this->getPoints());
			}

		}

		return list;
	}

	inline void Polygon::show()
	{
		if (first == nullptr)return;
		auto t = first;
		printf("start show std::vector<std::pair<double, double>> gon\n");
		do{
			printf("%.2f %.2f\n", t->x, t->y);
			t = t->next;
		} while (t&&t!=first);
		printf("end show std::vector<std::pair<double, double>> gon\n");
	}

	void Vertex::visit() {
		_visited = true;
		if (_corresponding != nullptr && !_corresponding->_visited) {
			_corresponding->visit();
		}
	}
	
	bool Vertex::equals(const Vertex& v) {
		return x == v.x && y == v.y;
	}

	inline bool myPoly::Vertex::isInside(Polygon* p)
	{
		bool oddNodes = false;
		Vertex* vertex = p->first;
		Vertex* next = vertex->next;
		const double x = this->x;
		const double y = this->y;

		do {
			if ((vertex->y < y && next->y >= y ||
				next->y < y && vertex->y >= y) &&
				(vertex->x <= x || next->x <= x)) {
				oddNodes ^= (vertex->x + (y - vertex->y) /
					(next->y - vertex->y) * (next->x - vertex->x) < x);
			}

			vertex = vertex->next;
			next = vertex->next ? vertex->next : p->first;
		} while (!vertex->equals(*p->first));
		return oddNodes;
	}

	std::vector<std::vector<std::pair<double, double>> > Boolean(
		std::vector<std::pair<double, double>>polygonA,
		std::vector<std::pair<double, double>> polygonB,
		bool eA, bool eB)
	{
		auto source = new Polygon(polygonA);
		auto clip = new Polygon(polygonB);
		return source->clip(clip, eA, eB);
	}

	std::vector<std::vector<std::pair<double, double>>> Union(
		std::vector<std::pair<double, double>> polygonA,
		std::vector<std::pair<double, double>> polygonB) {
		return Boolean(polygonA, polygonB, false, false);
	}
	std::vector<std::vector<std::pair<double, double>>> Intersection(
		std::vector<std::pair<double, double>> polygonA,
		std::vector<std::pair<double, double>> polygonB) {
		return Boolean(polygonA, polygonB, true, true);
	}
	std::vector<std::vector<std::pair<double, double>>> Diff(
		std::vector<std::pair<double, double>> polygonA,
		std::vector<std::pair<double, double>> polygonB) {
		return Boolean(polygonA, polygonB, false, true);
	}
}
typedef std::pair<double, double> node_pair;
typedef std::vector<node_pair> vec;
struct return_type {
	int state;
	node_pair r;
	return_type(int state, node_pair r = node_pair(0, 0)) :state(state), r(r) {};
};
double distance(node_pair& v1, node_pair& v2) {
	double x = v1.first - v2.first;
	double y = v1.second - v2.second;
	return std::sqrt(x * x + y * y);
}
struct Node {
	node_pair vec;
	double alpha;
	bool intersect;
	Node* next;
	Node* prev;
	Node* nextPoly;
	Node* neighbor;
	bool entry;
	bool visited;
	Node(node_pair& v, double a = 0, bool i = false)
		: vec(v), alpha(a), intersect(i), next(nullptr), prev(nullptr) {};
	Node* nextNonIntersection() {
		Node* a = this;
		while (a && a->intersect) {
			a = a->next;
		}
		return a;
	}

	Node* last() {
		Node* a = this;
		while (a->next && a->next != this) {
			a = a->next;
		}
		return a;
	}

	void createLoop() {
		Node* last = this->last();
		last->prev->next = this;
		this->prev = last->prev;
	}

	Node* firstNodeOfInterest() {
		Node* a = this;

		if (a) {
			do {
				a = a->next;
			} while (a != this && (!a->intersect || (a->intersect && a->visited)));
		}

		return a;
	}

	void insertBetween(Node* first, Node* last) {
		Node* a = first;
		while (a != last && a->alpha < this->alpha) {
			a = a->next;
		}

		this->next = a;
		this->prev = a->prev;
		if (this->prev) {
			this->prev->next = this;
		}

		this->next->prev = this;
	}
	void show() {
		std::cout << vec.first << ' ' << vec.second << '\n';
	}
};



return_type segseg(node_pair a, node_pair b, node_pair c, node_pair d) {
	double x1 = a.first;
	double y1 = a.second;
	double x2 = b.first;
	double y2 = b.second;
	double x3 = c.first;
	double y3 = c.second;
	double x4 = d.first;
	double y4 = d.second;

	double denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

	if (denom == 0) {
		// 线段平行或重叠，没有交点  
		return return_type(-1);
	}

	double ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
	double ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

	if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1) {
		// 计算交点坐标  
		double x = x1 + ua * (x2 - x1);
		double y = y1 + ua * (y2 - y1);
		return return_type(0, node_pair(x, y));
	}

	// 线段相交，但交点不在两个线段上  
	return return_type(1);
}

Node* createLinkedList(vec& vecs) {
	int l = vecs.size();
	Node* ret = nullptr;
	Node* where = nullptr;
	for (int i = 0; i < l; i++) {
		auto current = vecs[i];
		if (!ret) {
			where = ret = new Node(current);
		}
		else {
			where->next = new Node(current);
			where->next->prev = where;
			where = where->next;
		}
	}

	return ret;
}


bool identifyIntersections(Node* subjectList, Node* clipList) {
	Node* subject;
	Node* clip;
	// 新建两个与头节点值相同的虚拟节点，与尾节点相连
	// TODO 存疑
	Node* auxs = subjectList->last();
	auxs->next = new Node(subjectList->vec);
	auxs->next->prev = auxs;

	Node* auxc = clipList->last();
	auxc->next = new Node(clipList->vec);
	auxc->next->prev = auxc;

	bool found = false;
	for (subject = subjectList; subject->next != nullptr; subject = subject->next) {
		if (!subject->intersect) {
			for (clip = clipList; clip->next; clip = clip->next) {
				if (!clip->intersect) {
					auto a = subject->vec;
					auto b = subject->next->nextNonIntersection()->vec;
					auto c = clip->vec;
					auto d = clip->next->nextNonIntersection()->vec;

					auto i = segseg(a, b, c, d);

					if (i.state == 0) {
						found = true;
						Node* intersectionSubject = new Node(i.r, distance(a, i.r) / distance(a, b), true);
						Node* intersectionClip = new Node(i.r, distance(c, i.r) / distance(c, d), true);
						intersectionSubject->neighbor = intersectionClip;
						intersectionClip->neighbor = intersectionSubject;
						intersectionSubject->show();
						intersectionSubject->insertBetween(subject, subject->next->nextNonIntersection());
						intersectionClip->insertBetween(clip, clip->next->nextNonIntersection());
					}
				}
			}
		}
	}

	return found;
}

void identifyIntersectionType(Node* subjectList, Node* clipList,
	bool (*clipTest)(const std::vector<double>&), bool (*subjectTest)(const std::vector<double>&),
	const std::string& type) {
	Node* subject = subjectList;
	Node* clip = clipList;
	bool se = clipTest(subjectList->vec) < 0;
	if (type == "and") {
		se = !se;
	}

	while (subject->next) {
		subject = subject->next;
		if (subject->intersect) {
			subject->entry = se;
			se = !se;
		}
	}

	bool ce = subjectTest(clipList->vec) > 0;
	if (type == "or") {
		ce = !ce;
	}

	while (clip->next) {
		clip = clip->next;
		if (clip->intersect) {
			clip->entry = ce;
			ce = !ce;
		}
	}
}
vec polygonBoolean(vec subjectPoly, vec clipPoly, int type) {
	auto subjectList = createLinkedList(subjectPoly);
	auto clipList = createLinkedList(clipPoly);

	auto isects = identifyIntersections(subjectList, clipList);
	if (isects) {
		// Phase 2: walk the resulting linked list and mark each intersection
//          as entering or exiting

	}
	return vec();
}

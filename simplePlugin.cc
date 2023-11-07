#include "simplePlugin.hh"

simplePlugin::simplePlugin():iterationsSpinbox_(0)
{
}
void simplePlugin::initializePlugin()
{
    // Create the Toolbox Widget
    _toolBox = new QWidget();
    QPushButton* uploadButton1 = new QPushButton("upload A", _toolBox);
    QPushButton* uploadButton2 = new QPushButton("upload B", _toolBox);
    QPushButton* unionClippingButton = new QPushButton("union");
    QPushButton* diffClippingButton1 = new QPushButton("diff(A-B)");
    QPushButton* diffClippingButton2 = new QPushButton("diff(B-A)");
    QPushButton* IntersectionClippingButton = new QPushButton("Intersection");
    QPushButton* clearButton = new QPushButton("clear");
    QLabel* tipLabel1 = new QLabel("file A", _toolBox);
    QLabel* tipLabel2 = new QLabel("file B", _toolBox);
    QPushButton* draw_button1 = new QPushButton("draw file A", _toolBox);
    QPushButton* draw_button2 = new QPushButton("draw file B", _toolBox);
    QGridLayout* gridLayout = new QGridLayout(_toolBox);

    clearButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    gridLayout->addWidget(uploadButton1, 0, 0);
    gridLayout->addWidget(tipLabel1, 0, 1);
    gridLayout->addWidget(draw_button1, 0, 2);
    gridLayout->addWidget(uploadButton2, 1, 0);
    gridLayout->addWidget(tipLabel2, 1, 1);
    gridLayout->addWidget(draw_button2, 1, 2);
    gridLayout->addWidget(unionClippingButton, 2, 0);
    gridLayout->addWidget(IntersectionClippingButton, 2, 1);
    gridLayout->addWidget(diffClippingButton1, 2, 2);
    gridLayout->addWidget(diffClippingButton2, 3, 0);
    gridLayout->addWidget(clearButton, 3, 1, 1, 2);
    QIcon* toolIcon = new QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "b.png");

    //connect(smoothButton, SIGNAL(clicked()), this, SLOT(simpleLaplace()));
    connect(uploadButton1, SIGNAL(clicked()), this, SLOT(onUploadButton1Clicked()));
    connect(uploadButton2, SIGNAL(clicked()), this, SLOT(onUploadButton2Clicked()));
    connect(unionClippingButton, SIGNAL(clicked()), this, SLOT(calcUnion()));
    connect(diffClippingButton1, SIGNAL(clicked()), this, SLOT(calcDiff1()));
    connect(diffClippingButton2, SIGNAL(clicked()), this, SLOT(calcDiff2()));
    connect(IntersectionClippingButton, SIGNAL(clicked()), this, SLOT(calcIntersection()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearDraw()));
    connect(draw_button1, SIGNAL(clicked()), this, SLOT(drawA()));
    connect(draw_button2, SIGNAL(clicked()), this, SLOT(drawB()));

    emit addToolbox(tr("boolcalc"), _toolBox, toolIcon);
}


void simplePlugin::onUploadButton1Clicked() {
    QString filePath = QFileDialog::getOpenFileName(this->_toolBox, "选择文件", "", "All Files (*.*);;Text Files (*.txt)");
    if (!filePath.isEmpty()) {
        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open the file.";
            return;
        }
        dataVector1.clear();
        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList values = line.split(" ");
            if (values.size() == 2) {
                bool ok1, ok2;
                float value1 = values[0].toFloat(&ok1);
                float value2 = values[1].toFloat(&ok2);
                if (ok1 && ok2) {
                    dataVector1.append(QPair<float, float>(value1, value2));
                }
            }
        }
        inputFile.close();
        QFileInfo fileInfo(filePath);
        QLayoutItem* item = qobject_cast<QGridLayout*>(_toolBox->layout())->itemAtPosition(0, 1);
        qobject_cast<QLabel*>(item->widget())->setText(fileInfo.fileName());
    }
}

void simplePlugin::onUploadButton2Clicked() {
    QString filePath = QFileDialog::getOpenFileName(this->_toolBox, "选择文件", "", "All Files (*.*);;Text Files (*.txt)");
    if (!filePath.isEmpty()) {
        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open the file.";
            return;
        }

        dataVector2.clear();
        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList values = line.split(" ");
            if (values.size() == 2) {
                bool ok1, ok2;
                float value1 = values[0].toFloat(&ok1);
                float value2 = values[1].toFloat(&ok2);
                if (ok1 && ok2) {
                    dataVector2.append(QPair<float, float>(value1, value2));
                }
            }
        }

        inputFile.close();
        QFileInfo fileInfo(filePath);
        QLayoutItem* item = qobject_cast<QGridLayout*>(_toolBox->layout())->itemAtPosition(1, 1);
        qobject_cast<QLabel*>(item->widget())->setText(fileInfo.fileName());
    }
}

void logPaths(PathsD& p) {
    for (auto i : p) {
        std::cout << i.size() << '\n';
        for (auto j : i) {
            std::cout << j.x << ' '  << j.y << '\n';
        }
    }
}
void getCPath(QVector<QPair<float, float>>&dataVector, PathsD& a) {
    PathD t;
    for (auto i:dataVector) {
        t.push_back(PointD(i.first, i.second));
    }
    a.push_back(t);
}

void initVector1(QVector<QPair<float, float>>&dataVector) {
    dataVector.append(QPair<float, float>(0, 0));
    dataVector.append(QPair<float, float>(2, 0));
    dataVector.append(QPair<float, float>(2, 2));
    dataVector.append(QPair<float, float>(0, 2));
    dataVector.append(QPair<float, float>(0, 0));

}
void initVector2(QVector<QPair<float, float>>& dataVector) {
    dataVector.append(QPair<float, float>(1, 1));
    dataVector.append(QPair<float, float>(3, 1));
    dataVector.append(QPair<float, float>(3, 3));
    dataVector.append(QPair<float, float>(1, 3));
    dataVector.append(QPair<float, float>(1, 1));
}

void simplePlugin::draw(PathsD& p){
    int newObjectId = -1;
    PluginFunctions::setDrawMode(ACG::SceneGraph::DrawModes::WIREFRAME);
    OpenMesh::Vec4f black(0.0f, 0.0f, 0.0f, 1.0f);

    emit addEmptyObject(DATA_POLY_MESH, newObjectId);
    emit log(LOGINFO, QString::number(newObjectId));
    // Get the newly created object
    PolyMeshObject* object;
    PluginFunctions::getObject(newObjectId, object);
    if (object) {
        emit log(LOGINFO, "draw object");

        // Now you can use the object as usual, e.g. Get the node

        PolyMesh* mesh = PluginFunctions::polyMesh(object);
        std::vector<PolyMesh::VertexHandle> face_vhandles;
        for (auto i : p) {
            face_vhandles.clear();
            for (auto j : i) {
                auto v0 = mesh->add_vertex(PolyMesh::Point(j.x, j.y, 0));
                face_vhandles.push_back(v0);
            }
            auto t = mesh->add_face(face_vhandles);
            //mesh->set_color(t, black);


            // 更新网格对象  
            mesh->update_normals();
            mesh->request_vertex_normals();
            mesh->update_face_normals();
            emit updatedObject(newObjectId, UPDATE_COLOR);
        }
        //mesh->request_face_colors();
        //mesh-> request_vertex_normals();
        //mesh->set_color(face1, PolyMesh::Color(255, 0, 0, 1));

        // 更新网格对象  
        mesh->update_normals();
        mesh->update_face_normals();
        emit updatedObject(newObjectId, UPDATE_ALL);

        double a[3]{0}, b[3]{0}, c[3]{};
        for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
            auto vertex = mesh->point(*v_it);
            printf("%.2f %.2f %.2f\n", vertex[0], vertex[1], vertex[2]);
            for (int i = 0; i < 3; i++) {
                a[i] = std::min(vertex[i], a[i]);
                b[i] = std::max(vertex[i], b[i]);
            }
        }
        for (int i = 0; i < 3; i++) {
            c[i] = (a[i] + b[i]) / 2;
        }
        //PluginFunctions::setSceneCenter(ACG::Vec3d(c[0],c[1],c[2]),PluginFunctions::ALL_VIEWERS);
        printf(" c %.2f %.2f %.2f\n", c[0], c[1], c[2]);
    }
    else {
        emit log(LOGERR, "ERR TO DRAW");
    }
}

void simplePlugin::drawA() {
    PathsD a;
    getCPath(this->dataVector1, a);
    draw(a);
}

void simplePlugin::drawB() {
    PathsD a;
    getCPath(this->dataVector2, a);
    draw(a);
}
void simplePlugin::calcUnion()
{
    //initVector1(this->dataVector1);
    //initVector2(this->dataVector2);
    emit log(LOGINFO, "calcUnion");
    PathsD a, b, solution;
    getCPath(this->dataVector1,a);
    getCPath(this->dataVector2,b);
    solution = Union(a, b, FillRule::NonZero);
    logPaths(solution);
    draw(solution);
    emit log(LOGINFO, "calcUnion success");
    //exampleFunction();
}

void simplePlugin::calcDiff1()
{
    //initVector1(this->dataVector1);
    //initVector2(this->dataVector2);
    emit log(LOGINFO, "calcDiff");
    PathsD a, b, solution;
    getCPath(this->dataVector1, a);
    getCPath(this->dataVector2, b);
    solution = Difference(a, b, FillRule::NonZero);
    logPaths(solution);
    draw(solution);
    emit log(LOGINFO, "calcDiff success");
    //exampleFunction();
}

void simplePlugin::calcDiff2()
{
    //initVector1(this->dataVector1);
    //initVector2(this->dataVector2);
    emit log(LOGINFO, "calcXOR");
    PathsD a, b, solution;
    getCPath(this->dataVector1, a);
    getCPath(this->dataVector2, b);
    solution = Difference(b, a, FillRule::NonZero);
    logPaths(solution);
    draw(solution);
    emit log(LOGINFO, "calcXOR success");
}

void simplePlugin::calcIntersection()
{
    //initVector1(this->dataVector1);
    //initVector2(this->dataVector2);
    emit log(LOGINFO, "calcXOR");
    PathsD a, b, solution;
    getCPath(this->dataVector1, a);
    getCPath(this->dataVector2, b);
    solution = Intersect(b, a, FillRule::NonZero);
    logPaths(solution);
    draw(solution);
    emit log(LOGINFO, "calcXOR success");
}

void simplePlugin::clearDraw()
{
    emit deleteAllObjects();

}

void simplePlugin::exampleFunction() {
  int newObjectId = -1;
  // Emit the signal, that we want to create a new object of the specified type plane

  emit addEmptyObject(DATA_POLY_MESH, newObjectId);
  emit log(LOGINFO, QString::number(newObjectId));
  // Get the newly created object
  PolyMeshObject* object;
  PluginFunctions::getObject(newObjectId,object);
  if(object) {
      // Now you can use the object as usual, e.g. Get the node
       emit log(LOGINFO, "draw object");

       PolyMesh* mesh = PluginFunctions::polyMesh(object);
	   auto v0 = mesh->add_vertex(PolyMesh::Point(0, 0, 0));
       auto v1 = mesh->add_vertex(PolyMesh::Point(0, 1, 0));
       auto v2 = mesh->add_vertex(PolyMesh::Point(1, 1, 0));
       auto v3 = mesh->add_vertex(PolyMesh::Point(1, 0, 0));

       std::vector<PolyMesh::VertexHandle> face_vhandles;
       face_vhandles.push_back(v0);
       face_vhandles.push_back(v1);
       face_vhandles.push_back(v2);
       face_vhandles.push_back(v3);
       PolyMesh::FaceHandle face1 = mesh->add_face(face_vhandles);
       //mesh->request_face_colors();
       mesh-> request_vertex_normals();
       //mesh->set_color(face1, PolyMesh::Color(255, 0, 0, 1));

       // 更新网格对象  
       mesh->update_normals();
       mesh->update_face_normals();
       emit updatedObject(newObjectId, UPDATE_ALL);
      // ...
  } else {
    // Something went wrong when creating the object.
  }
}

void simplePlugin::simpleLaplace()
{
  for (PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS); o_it != PluginFunctions::objectsEnd();
      ++o_it) {
    if (o_it->dataType(DATA_TRIANGLE_MESH)) {
      // Get the mesh to work on
      TriMesh* mesh = PluginFunctions::triMesh(*o_it);
      // Property for the active mesh to store original point positions
      OpenMesh::VPropHandleT<TriMesh::Point> origPositions;
      // Add a property to the mesh to store original vertex positions
      mesh->add_property(origPositions, "simplePlugin_Original_Positions");
      for (int i = 0; i < iterationsSpinbox_->value(); ++i) {
        // Copy original positions to backup ( in vertex property )
        TriMesh::VertexIter v_it, v_end = mesh->vertices_end();
        for (v_it = mesh->vertices_begin(); v_it != v_end; ++v_it) {
          mesh->property(origPositions, *v_it) = mesh->point(*v_it);
        }
        // Do one smoothing step (For each point of the mesh ... )
        for (v_it = mesh->vertices_begin(); v_it != v_end; ++v_it) {
          TriMesh::Point point = TriMesh::Point(0.0, 0.0, 0.0);
          // Flag, to skip boundary vertices
          bool skip = false;
          // ( .. for each outgoing halfedge .. )
          TriMesh::VertexOHalfedgeIter voh_it(*mesh, *v_it);
          for (; voh_it.is_valid(); ++voh_it) {
            // .. add the (original) position of the neighbour ( end of the outgoing halfedge )
            point += mesh->property(origPositions, mesh->to_vertex_handle(*voh_it));
            // Check if the current Halfedge is a boundary halfedge
            // If it is, abort and keep the current vertex position
            if (mesh->is_boundary(*voh_it)) {
              skip = true;
              break;
            }
          }
          // Devide by the valence of the current vertex
          point /= mesh->valence(*v_it);
          if (!skip) {
            // Set new position for the mesh if its not on the boundary
            mesh->point(*v_it) = point;
          }
        }
      }     // Iterations end
      // Remove the property
      mesh->remove_property(origPositions);
      mesh->update_normals();
      emit updatedObject(o_it->id(),UPDATE_GEOMETRY);
    } else if (o_it->dataType(DATA_POLY_MESH)) {
      // Get the mesh to work on
      PolyMesh* mesh = PluginFunctions::polyMesh(*o_it);
      // Property for the active mesh to store original point positions
      OpenMesh::VPropHandleT<PolyMesh::Point> origPositions;
      // Add a property to the mesh to store original vertex positions
      mesh->add_property(origPositions, "simplePlugin_Original_Positions");
      for (int i = 0; i < iterationsSpinbox_->value(); ++i) {
        // Copy original positions to backup ( in Vertex property )
        PolyMesh::VertexIter v_it, v_end = mesh->vertices_end();
        for (v_it = mesh->vertices_begin(); v_it != v_end; ++v_it) {
          mesh->property(origPositions, *v_it) = mesh->point(*v_it);
        }
        // Do one smoothing step (For each point of the mesh ... )
        for (v_it = mesh->vertices_begin(); v_it != v_end; ++v_it) {
          PolyMesh::Point point = PolyMesh::Point(0.0, 0.0, 0.0);
          // Flag, to skip boundary vertices
          bool skip = false;
          // ( .. for each Outoing halfedge .. )
          PolyMesh::VertexOHalfedgeIter voh_it(*mesh, *v_it);
          for (; voh_it.is_valid(); ++voh_it) {
            // .. add the (original) position of the Neighbour ( end of the outgoing halfedge )
            point += mesh->property(origPositions, mesh->to_vertex_handle(*voh_it));
            // Check if the current Halfedge is a boundary halfedge
            // If it is, abort and keep the current vertex position
            if (mesh->is_boundary(*voh_it)) {
              skip = true;
              break;
            }
          }
          // Devide by the valence of the current vertex
          point /= mesh->valence(*v_it);
          if (!skip) {
            // Set new position for the mesh if its not on the boundary
            mesh->point(*v_it) = point;
          }
        }
      }     // Iterations end
      // Remove the property
      mesh->remove_property(origPositions);
      mesh->update_normals();
      emit updatedObject(o_it->id(),UPDATE_GEOMETRY);
    } else {
      emit log(LOGERR, "Data type not supported.");
    } // Switch data type
  }
}

#include "simplePlugin.hh"
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ObjectTypes/PolyMesh/PolyMeshTypes.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>  
#include <OpenFlipper/BasePlugin/BaseInterface.hh>  
#include <OpenFlipper/common/GlobalOptions.hh>  
#include <OpenFlipper/common/Types.hh>  
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/BaseObject.hh>
#include <QString>  
#include <OpenFlipper/common/Types.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <OpenFlipper/common/ObjectTypeDLLDefines.hh>
#include <qdebug.h>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>

simplePlugin::simplePlugin():iterationsSpinbox_(0)
{
}
void simplePlugin::initializePlugin()
{
    // Create the Toolbox Widget
    _toolBox = new QWidget();
    QPushButton* uploadButton1 = new QPushButton("upload 1", _toolBox);
    QPushButton* uploadButton2 = new QPushButton("upload 2", _toolBox);

    QLabel* tipLabel1 = new QLabel("file 1", _toolBox);
    QLabel* tipLabel2 = new QLabel("file 2", _toolBox);
    QPushButton* unionClippingButton = new QPushButton ("union");
    QPushButton* diffClippingButton = new QPushButton ("diff");
    QPushButton* XORClippingButton = new QPushButton ("XOR");
    QPushButton* clearButton = new QPushButton ("clear");

    QGridLayout* gridLayout = new QGridLayout(_toolBox);

    gridLayout->addWidget(uploadButton1, 0, 0);
    gridLayout->addWidget(tipLabel1, 0, 1);
    gridLayout->addWidget(uploadButton2, 1, 0);
    gridLayout->addWidget(tipLabel2, 1, 1);
    gridLayout->addWidget(unionClippingButton, 2, 0);
    gridLayout->addWidget(diffClippingButton, 2, 1);
    gridLayout->addWidget(XORClippingButton, 3, 0);
    gridLayout->addWidget(clearButton, 3, 1);
    QIcon* toolIcon = new QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "b.png");

    //connect(smoothButton, SIGNAL(clicked()), this, SLOT(simpleLaplace()));
    connect(uploadButton1, SIGNAL(clicked()), this, SLOT(onUploadButton1Clicked()));
    connect(uploadButton2, SIGNAL(clicked()), this, SLOT(onUploadButton2Clicked()));
    connect(unionClippingButton, SIGNAL(clicked()), this, SLOT(calcUnion()));
    connect(diffClippingButton, SIGNAL(clicked()), this, SLOT(calcDiff()));
    connect(XORClippingButton, SIGNAL(clicked()), this, SLOT(calcXOR()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearAll()));

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
    }
}

void logPaths(Paths64& p) {
    for (auto i : p) {
        std::cout << i.size() << '\n';
        for (auto j : i) {
            std::cout << j.x << ' '  << j.y << '\n';
        }
    }
}
void getCPath(QVector<QPair<float, float>>&dataVector, Paths64& a) {
    Path64 t;
    for (auto i:dataVector) {
        t.push_back(Point64(i.first, i.second));
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

void simplePlugin::draw(Paths64 p){
    int newObjectId = -1;

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
            for (auto j : i) {
                auto v0 = mesh->add_vertex(PolyMesh::Point(j.x, j.y, 0));
                face_vhandles.push_back(v0);
            }
            mesh->add_face(face_vhandles);
            mesh->request_vertex_normals();
            //mesh->set_color(face1, PolyMesh::Color(255, 0, 0, 1));

            // 更新网格对象  
            mesh->update_normals();
            mesh->update_face_normals();

        }
        //mesh->request_face_colors();
        //mesh-> request_vertex_normals();
        //mesh->set_color(face1, PolyMesh::Color(255, 0, 0, 1));

        // 更新网格对象  
        mesh->update_normals();
        mesh->update_face_normals();
        emit updatedObject(newObjectId, UPDATE_ALL);
    }
    else {
        emit log(LOGERR, "ERR TO DRAW");
    }
}
void simplePlugin::calcUnion()
{
    initVector1(this->dataVector1);
    initVector2(this->dataVector2);
    Paths64 a, b, solution;
    getCPath(this->dataVector1,a);
    getCPath(this->dataVector2,b);
    logPaths(a);
    logPaths(b);
    solution = Union(a, b, FillRule::NonZero);
    logPaths(solution);
    emit log(LOGINFO, "abc123");
    draw(solution);
    //exampleFunction();
}

void simplePlugin::calcDiff()
{
    initVector1(this->dataVector1);
    initVector2(this->dataVector2);
    Paths64 a, b, solution;
    getCPath(this->dataVector1, a);
    getCPath(this->dataVector2, b);
    logPaths(a);
    logPaths(b);
    solution = Difference(a, b, FillRule::NonZero);
    logPaths(solution);
    emit log(LOGINFO, "abc123");
    draw(solution);
    //exampleFunction();
}

void simplePlugin::calcXOR()
{
    initVector1(this->dataVector1);
    initVector2(this->dataVector2);
    Paths64 a, b, solution;
    getCPath(this->dataVector1, a);
    getCPath(this->dataVector2, b);
    logPaths(a);
    logPaths(b);
    solution = Xor(a, b, FillRule::NonZero);
    logPaths(solution);
    emit log(LOGINFO, "abc123");
    draw(solution);
}

void simplePlugin::clearAll()
{
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

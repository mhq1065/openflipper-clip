
#include "simplePlugin.hh"
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include "OpenFlipper/BasePlugin/PluginFunctions.hh"
simplePlugin::simplePlugin() :
        iterationsSpinbox_(0)
{
}
void simplePlugin::initializePlugin()
{
    // Create the Toolbox Widget
    _toolBox = new QWidget();
    QPushButton* uploadButton1 = new QPushButton("Upload", _toolBox);
    QPushButton* uploadButton2 = new QPushButton("Upload", _toolBox);
    QLabel* tipLabel1 = new QLabel("this is one", _toolBox);
    QLabel* tipLabel2 = new QLabel("Tip 2", _toolBox);
    QPushButton* calculateButton = new QPushButton("Calculate", _toolBox);

    QGridLayout* gridLayout = new QGridLayout(_toolBox);

    gridLayout->addWidget(uploadButton1, 0, 0);
    gridLayout->addWidget(tipLabel1, 0, 1);
    gridLayout->addWidget(uploadButton2, 1, 0);
    gridLayout->addWidget(tipLabel2, 1, 1);
    gridLayout->addWidget(calculateButton, 2, 0, 1, 2);
    QIcon* toolIcon = new QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "b.png");
    //connect(smoothButton, SIGNAL(clicked()), this, SLOT(simpleLaplace()));
    connect(uploadButton1, SIGNAL(clicked()), this, SLOT(onUploadButton1Clicked()));
    connect(uploadButton2, SIGNAL(clicked()), this, SLOT(onUploadButton2Clicked()));
    emit addToolbox(tr("boolcalc"), _toolBox, toolIcon);
}
void simplePlugin::onUploadButton1Clicked() {
    // 处理上传按钮1的点击事件  
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
    // 处理上传按钮2的点击事件  
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

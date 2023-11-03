#ifndef SIMPLEPLUGIN_HH
#define SIMPLEPLUGIN_HH
#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>  
#include <OpenFlipper/common/Types.hh>
#include <OpenFlipper/common/ObjectTypeDLLDefines.hh>
#include <OpenFlipper/common/GlobalOptions.hh>  
#include <OpenFlipper/common/BaseObject.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/PolyMesh/PolyMeshTypes.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>
#include <ACG/Scenegraph/TransformNode.hh>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>
#include <qdebug.h>
#include "PolygonClipping/PolygonClipping.h"

using namespace PolyClip;

class simplePlugin : public QObject, BaseInterface, ToolboxInterface, LoggingInterface, LoadSaveInterface
{
	Q_OBJECT
		Q_INTERFACES(BaseInterface)
		Q_INTERFACES(ToolboxInterface)
		Q_INTERFACES(LoggingInterface)
		Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.examples.SmootherPlugin")
		Q_INTERFACES(LoadSaveInterface)
signals:
	//BaseInterface
	void updateView();
	void updatedObject(int _identifier, const UpdateType& _type);
	//LoggingInterface
	void log(Logtype _type, QString _message);
	void log(QString _message);
	// ToolboxInterface
	void addToolbox(QString _name, QWidget* _widget, QIcon* _icon);
	void addToolbox(QString _name, QWidget* _widget);
	// LoadSaveInterface
	void addEmptyObject(DataType _type, int& _id);
	void deleteAllObjects();
public:
	simplePlugin();
	// BaseInterface
	QString name() { return (QString("bool calc")); };
	QString description() { return (QString("bool calc")); };
private:
	QWidget* _toolBox;
	QSpinBox* iterationsSpinbox_;
	QVector<QPair<float, float>>dataVector1, dataVector2;
	void exampleFunction();
private slots:
	// BaseInterface
	void initializePlugin();
	void simpleLaplace();
	void onUploadButton1Clicked();
	void onUploadButton2Clicked();
	void draw(std::vector<std::vector<PolyClip::Point2d>> p);
	void calcUnion();
	void calcDiff();
	void calcIntersection();
	void clearDraw();

public slots:
	QString version() { return QString("1.0"); };
};
#endif 
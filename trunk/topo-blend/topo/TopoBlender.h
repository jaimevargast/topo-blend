#pragma once

#include <QObject>
#include "DynamicGraph.h"

namespace Structure{
	typedef QPair<Node*,Node*> QPairNodes;
	typedef QPair<Link*,Link*> QPairLink;
	typedef QPair<Scalar, QPairLink> ScalarLinksPair;

	struct Graph;

	class TopoBlender : public QObject
	{
		Q_OBJECT
	public:
		explicit TopoBlender(Graph * graph1, Graph * graph2, QObject *parent = 0);

		Graph * g1;
		Graph * g2;

		DynamicGraph source;
		DynamicGraph active;
		DynamicGraph target;

		Graph * blend();
		void materializeInBetween( Graph * graph, double t = 0.0 );

		QList< ScalarLinksPair > badCorrespondence( QString activeNodeID, QString targetNodeID, 
			QMap<Link*, Vec4d> & coord_active, QMap<Link*, Vec4d> & coord_target );

	public slots:
		void bestPartialCorrespondence();

	public:
		// DEBUG:
		std::vector< Vector3 > debugPoints;
		std::vector< PairVector3 > debugLines;
		void drawDebug();

	signals:

	};
}

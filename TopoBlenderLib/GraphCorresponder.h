#pragma once;

#include "SurfaceMeshTypes.h"
#include "StructureGraph.h"

#include <vector>

typedef std::pair< std::vector<QString>, std::vector<QString> > VECTOR_PAIR;
Q_DECLARE_METATYPE(VECTOR_PAIR);

class GraphCorresponder : public QObject
{
	Q_OBJECT

public:
    GraphCorresponder(Structure::Graph *source, Structure::Graph *target);

	// Helper functions
	QString sgName();
	QString tgName();
	template <class Type>
	void initializeMatrix(std::vector< std::vector<Type> > & M, Type value);
	void normalizeMatrix(std::vector< std::vector<float> > & M);
	bool minElementInMatrix(std::vector< std::vector<float> > &M, int &row, int &column, float &minValue);

	// Landmarks
	std::vector<VECTOR_PAIR> landmarks;
	std::vector<bool> sIsLandmark, tIsLandmark;
	void addLandmarks(std::vector<QString> sParts, std::vector<QString> tParts);
	void removeLandmarks(int pos, int n);
	void saveLandmarks(QString filename);
	void loadLandmarks(QString filename);

	// Hausdorff distance
	float supInfDistance(std::vector<Vector3> &A, std::vector<Vector3> &B);
	float HausdorffDistance(std::vector<Vector3> &A, std::vector<Vector3> &B);

	// Distance matrices
	void computeValidationMatrix();
	void computeHausdorffDistanceMatrix(std::vector< std::vector<float> > & M);
	void computeSizeDiffMatrix(std::vector< std::vector<float> > & M);
	void computeOrientationDiffMatrix(std::vector< std::vector<float> > & M);
	void computeDistanceMatrix();

	// Part to Part
	void computePartToPartCorrespondences();
	void saveCorrespondences(QString filename);
	void LoadCorrespondences(QString filename);

	// Point to Point 
	void correspondTwoNodes(Structure::Node *sNode, Structure::Node *tNode);
	void correspondTwoCurves(Structure::Curve *sCurve, Structure::Curve *tCurve);
	void correspondTwoSheets(Structure::Sheet *sSheet, Structure::Sheet *tSheet);

	// Find non-corresponding nodes
	std::vector<QString> nonCorresSource(); // to kill
	std::vector<QString> nonCorresTarget(); // to grow

	// Result
	std::vector<bool> sIsCorresponded, tIsCorresponded;
	std::vector<VECTOR_PAIR> correspondences;
	std::vector<std::vector<float> > corrScores;

public slots:
	void visualizePart2PartDistance(int sourceID);
	void computeCorrespondences();

public:
	Structure::Graph *sg, *tg;
	std::vector< std::vector<float> > disM;
	std::vector< std::vector<bool> > validM;
};
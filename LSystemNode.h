#pragma once

#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnMeshData.h>

#include <maya/MIOStream.h>

#include <cylinder.h>
#include <LSystem.h>
#include <maya/MGlobal.h>
#include <maya/MNodeMessage.h>

MStatus returnStatus;
MCallbackId callback;

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}


class LSystemNode : public MPxNode
{
public:
	LSystemNode() {};
	virtual ~LSystemNode() { MMessage::removeCallback(callback); };

	virtual void postConstructor();
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static  void*	creator();
	static  MStatus initialize();

	static MObject  angle;
	static MObject  step;
	static MObject  grammar;
	static MObject  ifFile;

	static MObject  time;
	static MObject  outputMesh;
	static MTypeId  id;

	LSystem lSystem;
	bool fromFile;
	static std::string directoryPath;
	virtual void initLSystem();
	virtual void setFileBool(bool b);
	virtual void setGrammar(const std::string& grammarProgram);
};

MObject LSystemNode::angle;
MObject LSystemNode::step;
MObject LSystemNode::grammar;
MObject LSystemNode::ifFile;

MObject LSystemNode::time;
MObject LSystemNode::outputMesh;
MTypeId LSystemNode::id(0x80000);

std::string LSystemNode::directoryPath;

void* LSystemNode::creator()
{
	LSystemNode::directoryPath = DIRMACRO;
	for (int i = 0; i < directoryPath.length(); i++) {
		if (directoryPath[i] == '\\') {
			directoryPath[i] = '/';
		}
	}
	LSystemNode::directoryPath += "plants/";

	MGlobal::displayInfo(directoryPath.c_str());
	MGlobal::displayInfo(LSystemNode::directoryPath.c_str());

	LSystemNode* lSysNode = new LSystemNode; 
	lSysNode->initLSystem();
	lSysNode->setFileBool(false);
	return lSysNode;
}

// Set data for specific instances
void LSystemNode::initLSystem() { lSystem = LSystem(); }

void LSystemNode::setFileBool(bool b) { fromFile = b; }

void LSystemNode::setGrammar(const std::string& grammarProgram) {
	if (fromFile) {
		lSystem.loadProgram(LSystemNode::directoryPath + grammarProgram);
	}
	else {
		lSystem.loadProgramFromString(grammarProgram);
	}
}

void changeAttrCallback(MNodeMessage::AttributeMessage msg,
	MPlug& plug, MPlug& otherPlug, void* clientData)
{
	// Narrow down when to use this callback
	if (!(msg & MNodeMessage::kAttributeSet) &&
		!(msg & MNodeMessage::kIncomingDirection)) {
		return;
	}
	LSystemNode* lSysNode = (LSystemNode*)clientData;

	if (plug == lSysNode->ifFile) {
		// Get whether to read by filename
		bool ifF = plug.asBool(&returnStatus);
		if (MS::kSuccess == returnStatus) {
			MGlobal::displayInfo(std::to_string(ifF).c_str());
			lSysNode->setFileBool(ifF);
		}
		else return;
	}

	if (plug == lSysNode->grammar) {
		MStatus returnStatus;

		// Get grammar
		MString g = plug.asString(&returnStatus);
		if (MS::kSuccess == returnStatus) {
			MGlobal::displayInfo(g);
		}
		else return;

		// Set grammar
		lSysNode->setGrammar(g.asChar());

		MGlobal::displayInfo("Changed grammar string to");
		MGlobal::displayInfo(g);
	}
}

// Deals with callback setup after the MObject-Node link is complete
void LSystemNode::postConstructor() {
	MStatus returnStatus;
	MObject thisNode = this->thisMObject();
	callback = MNodeMessage::addAttributeChangedCallback(thisNode, changeAttrCallback, this, &returnStatus);
}

MStatus LSystemNode::initialize()
{
	MFnNumericAttribute numAttr;
	MFnUnitAttribute unitAttr;
	MFnTypedAttribute typedAttr;

	MStatus returnStatus;

	// Create Attributes
	LSystemNode::step = numAttr.create("step", "st",
		MFnNumericData::kDouble, 0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode step size attribute\n");

	LSystemNode::angle = numAttr.create("angle", "an",
		MFnNumericData::kDouble, 0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode angle attribute\n");

	LSystemNode::grammar = typedAttr.create("grammar", "gr",
		MFnData::kString, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode grammar attribute\n");

	LSystemNode::ifFile = numAttr.create("fromFile", "fi", 
		MFnNumericData::kBoolean, 0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating grammar file checkbox attribute\n");


	LSystemNode::time = unitAttr.create("time", "tm",
		MFnUnitAttribute::kTime, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode time attribute\n");

	LSystemNode::outputMesh = typedAttr.create("outputMesh", "out",
		MFnData::kMesh, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode mesh attribute\n");
	typedAttr.setStorable(false);


	// Add them
	returnStatus = addAttribute(LSystemNode::step);
	McheckErr(returnStatus, "ERROR adding step size attribute\n");

	returnStatus = addAttribute(LSystemNode::angle);
	McheckErr(returnStatus, "ERROR adding angle attribute\n");

	returnStatus = addAttribute(LSystemNode::grammar);
	McheckErr(returnStatus, "ERROR adding grammar attribute\n");

	returnStatus = addAttribute(LSystemNode::ifFile);
	McheckErr(returnStatus, "ERROR adding step grammar attribute\n");


	returnStatus = addAttribute(LSystemNode::time);
	McheckErr(returnStatus, "ERROR adding time attribute\n");

	returnStatus = addAttribute(LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR adding outputMesh attribute\n");


	// Connect changes to the mesh output
	returnStatus = attributeAffects(LSystemNode::step,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in step attributeAffects\n");

	returnStatus = attributeAffects(LSystemNode::angle,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in angle attributeAffects\n");

	returnStatus = attributeAffects(LSystemNode::grammar,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in grammar attributeAffects\n");

	returnStatus = attributeAffects(LSystemNode::time,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in time attributeAffects\n");

	return MS::kSuccess;
}

MStatus LSystemNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus;

	// Update LSystem with the following values
	if (plug == outputMesh) {
		// Get step
		MDataHandle stepData = data.inputValue(step, &returnStatus);
		McheckErr(returnStatus, "Error getting step data handle\n");
		lSystem.setDefaultStep(stepData.asDouble());

		// Get angle
		MDataHandle angleData = data.inputValue(angle, &returnStatus);
		McheckErr(returnStatus, "Error getting angle data handle\n");
		lSystem.setDefaultAngle(angleData.asDouble());

		// Grammar updates are handled in a callback above

		// Get time/iterations - connections are handled in the MEL script, 
		//  - I scrapped time1 since it updated too much for my computer
		MDataHandle timeData = data.inputValue(time, &returnStatus);
		McheckErr(returnStatus, "Error getting time data handle\n");
		MTime time = timeData.asTime();

		// Get output object
		MDataHandle outputHandle = data.outputValue(outputMesh, &returnStatus);
		McheckErr(returnStatus, "ERROR getting polygon data handle\n");

		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		MGlobal::displayInfo("Updated Values");

		// Run Turtle and get resulting geometry data
		MPointArray points;
		MIntArray faceCounts;
		MIntArray faceConnects;

		CylinderMesh::generateGeometry(time.as(MTime::kFilm), &lSystem,
			points, faceCounts, faceConnects);
		MGlobal::displayInfo("Processed LSystem");

		MFnMesh mesh;
		MObject newMesh = mesh.create(points.length(), faceCounts.length(),
			points, faceCounts, faceConnects,
			newOutputData, &returnStatus);

		McheckErr(returnStatus, "ERROR creating new cylinder");
		MGlobal::displayInfo("Loaded geometry");

		outputHandle.set(newOutputData);
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}
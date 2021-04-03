#include "LSystemCmd.h"


#include <maya/MGlobal.h>
#include <list>
LSystemCmd::LSystemCmd() : MPxCommand()
{
}

LSystemCmd::~LSystemCmd() 
{
}

MStatus LSystemCmd::doIt( const MArgList& args )
{
	LSystem lSystem = LSystem();
	MStatus status;
	unsigned int iterations = 1;
	std::string id = "";

	// View desired arguements
	for (int i = 0; i < args.length(); i++) {
		// Check for Step Size arguement
		if (MString("-step") == args.asString(i, &status) &&
			MS::kSuccess == status) {

			double stepSize = args.asDouble(++i, &status);
			if (MS::kSuccess == status) { lSystem.setDefaultStep(stepSize); }
		}

		// Check for Angle arguement
		else if (MString("-angle") == args.asString(i, &status) &&
			MS::kSuccess == status) {

			double angle = args.asDouble(++i, &status);
			if (MS::kSuccess == status) { lSystem.setDefaultAngle(angle); }
		}
	
		// Check for Grammar arguement
		else if (MString("-grammar") == args.asString(i, &status) &&
			MS::kSuccess == status) {
			
			MString grammar = args.asString(++i, &status);
			if (MS::kSuccess == status) { lSystem.loadProgramFromString(grammar.asChar()); }
		}

		// Check for Iterations arguement
		else if (MString("-iterations") == args.asString(i, &status) &&
			MS::kSuccess == status) {

			int iters = args.asInt(++i, &status);
			if (MS::kSuccess == status) { iterations = iters; }
		}

		// Check for ID arguement - ADDED FOR NAMING PURPOSES
		else if (MString("-id") == args.asString(i, &status) &&
			MS::kSuccess == status) {

			int idLoad = args.asInt(++i, &status);
			if (MS::kSuccess == status) { id = std::to_string(idLoad); }
		}
	}

	// message in Maya output window
	cout << "Implementing LSystem!\nGrammar:\n" + lSystem.getGrammarString() + 
		"\nIterations: " + std::to_string(iterations) + 
		"\nStep: " + std::to_string(lSystem.getDefaultStep()).c_str() +
		"\nAngle: " + std::to_string(lSystem.getDefaultAngle()).c_str() << endl;
	std::cout.flush();

	// message in scriptor editor
	MGlobal::displayInfo("Implementing LSystem!");
	MGlobal::displayInfo("Grammar:");
	MGlobal::displayInfo(lSystem.getGrammarString().c_str());
	MGlobal::displayInfo("Iterations:");
	MGlobal::displayInfo(std::to_string(iterations).c_str());
	MGlobal::displayInfo("Step:");
	MGlobal::displayInfo(std::to_string(lSystem.getDefaultStep()).c_str());
	MGlobal::displayInfo("Angle:");
	MGlobal::displayInfo(std::to_string(lSystem.getDefaultAngle()).c_str());

	// Run Turtle and get resulting geometry data
	MPointArray points;
	MIntArray faceCounts;
	MIntArray faceConnects;

	CylinderMesh::generateGeometry(iterations, &lSystem,
		points, faceCounts, faceConnects);
	MGlobal::displayInfo("Processed LSystem");


	// Uses nodes for this method too, specifically in order to name and shade this tree, 
	// except this type doesnt include the dynamic LSystemNode
	MDGModifier mod;
	MFnDependencyNode fn;
	MFnMesh mesh;
	MFnSet fnSet;

	MObject transNode = fn.create("transform");

	MObject meshNode = mesh.create(points.length(), faceCounts.length(),
		points, faceCounts, faceConnects, transNode);

	// Set custom ID number if provided
	if (!id.empty()) {
		mod.renameNode(transNode, ("LSystem" + id).c_str());
		mod.renameNode(meshNode, ("LSystemShape" + id).c_str());
		mod.doIt();
	}

	// Can apply a custom shading group here
	/*
	MSelectionList shadeGroup;
	MObject shading = fnSet.create(shadeGroup, MFnSet::kRenderableOnly, &status);
	fnSet.addMember(meshNode);
	*/

	// Instead use a MEL command to link to existing initialShadingGroup
	MString cmdStr = "sets - add initialShadingGroup " + mesh.name() + ";";
	MGlobal::executeCommand(cmdStr);

	MGlobal::displayInfo("Set up LSystem dependencies");
    return MStatus::kSuccess;
}


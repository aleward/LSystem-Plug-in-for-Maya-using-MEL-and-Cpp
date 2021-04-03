#include <LSystemNode.h>

#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MDGModifier.h>
#include <maya/MPlugArray.h>
#include <maya/MVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>
#include <list>

#include "LSystemCmd.h"

MString LSystemCmd::openWinItem("Open LSystem Dialog");

MStatus initializePlugin( MObject obj )
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "MyPlugin", "1.0", "Any");

    // Register Command
    status = plugin.registerCommand( "LSystemCmd", LSystemCmd::creator );
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

	// Register Node
	status = plugin.registerNode("LSystemNode", LSystemNode::id,
		LSystemNode::creator, LSystemNode::initialize);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	// Since I save the .mll file elsewhere, plugin.loadPath() does not correspond to the Project Directory
	// To play it safe, I registered ProjectDir as a macro for easier & consistent import of mel
	std::string pluginSource = DIRMACRO; // Can replace with project directory if this doesnt work for you
	// Replace all \ with /
	for (int i = 0; i < pluginSource.length(); i++) {
		if (pluginSource[i] == '\\') {
			pluginSource[i] = '/';
		}
	}

	char buffer[2048];
	pluginSource = "source \"" + pluginSource + "LSystemDialog.mel\";";
	sprintf_s(buffer, 2048, pluginSource.c_str(), plugin.loadPath());
	MGlobal::executeCommand(buffer, true);

	// Torn between deleting and regenerating or hiding and showing
	const MString openWinCmd = "makeWindow();";//"showWindow $window;";
	plugin.addMenuItem(LSystemCmd::openWinItem, "mainCreateMenu", openWinCmd, "");
	
    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

	// De-register Command
    status = plugin.deregisterCommand( "LSystemCmd" );
    if (!status) {
	    status.perror("deregisterCommand");
	    return status;
    }

	// De-register Node
	status = plugin.deregisterNode(LSystemNode::id);
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	MStringArray menuNames = MStringArray();
	menuNames.append("Open_LSystem_Dialog");
	plugin.removeMenuItem(menuNames);

    return status;
}



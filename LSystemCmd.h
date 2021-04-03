#ifndef CreateLSystemCmd_H_
#define CreateLSystemCmd_H_

// MEL - For getting relative file path:
// Added DIRMACRO=R"($(ProjectDir))" to Preprocessor Definitions

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <string>
#include <LSystem.h>
#include <cylinder.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MFnPhongShader.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>

class LSystemCmd : public MPxCommand
{
public:
    LSystemCmd();
    virtual ~LSystemCmd();
    static void* creator() { return new LSystemCmd(); }
    MStatus doIt( const MArgList& args );

	static MString openWinItem;
};

#endif
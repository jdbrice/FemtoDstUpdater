import os
import subprocess
import platform

env = Environment()

SConscript( "modules/FemtoDstFormat/SConstruct" )

env.Append(CPPPATH    = [ "modules/" ])
env.Append(CXXFLAGS   = ['-std=c++11'])


# ROOT
ROOTCFLAGS      = subprocess.check_output( ['root-config',  '--cflags'] ).rstrip().split( " " )
ROOTLIBS        = subprocess.check_output( ["root-config",  "--libs"] )
ROOT_SYS        = os.environ.get( "ROOTSYS", "" )
JDB_LIB			= os.environ[ "JDB_LIB" ]

env.Append(CPPPATH   = [ ROOT_SYS + "/include/", JDB_LIB + "/include" ])
env.Append(CXXFLAGS  = ROOTCFLAGS )
env.Append(LINKFLAGS = ROOTCFLAGS )
env.Append(LIBS 			= [ "libRooBarbCore.a", "libRooBarbConfig.a", "libRooBarbTasks.a", "libRooBarbRootAna.a", "libRooBarbUnitTest.a", "libRooBarbExtra.a" ] )
env.Append(LIBPATH 		= [ JDB_LIB + "/lib/" ] )
env[ "_LIBFLAGS" ] = env[ "_LIBFLAGS" ] + " " + ROOTLIBS + " "


# REMOVE "-pthread" from clang link options in OS X
# assuming darwin=clang but must be a better way...
if "Darwin" in platform.platform() :
	env[ "LINKFLAGS" ].remove( "-pthread" )


env.Program( target="fdstWriter.app", source=["modules/FemtoDstFormat/DictionaryFemtoDst.cpp", "modules/CandidateDstFormat/DictionaryCandidateDst.cpp", "Engine.cpp"] )

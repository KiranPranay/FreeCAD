/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/
 
#include "PreCompiled.h"
#ifndef _PreComp_
# include <fcntl.h>
# include <STEPControl_Reader.hxx>
# include <TopoDS_Shape.hxx>
# include <TopTools_HSequenceOfShape.hxx>
#endif

#include <Base/Console.h>
#include <Base/Sequencer.h>

#include "FeatureImportStep.h"


using namespace Import;

void FeatureImportStep::InitLabel(const TDF_Label &rcLabel)
{
    addProperty("String","FileName");

}


Standard_Integer FeatureImportStep::Execute(void)
{
    Base::Console().Log("FeaturePartImportStep::Execute()\n");

  try{

    STEPControl_Reader aReader;
    TopoDS_Shape aShape;

    std::string FileName = getPropertyString("FileName");

    if( FileName == "") 
      return 1;

    int i=_open(FileName.c_str(),O_RDONLY);
    if( i != -1)
    {
        _close(i);
    }else{
      setError("File not readable");
      return 1;
    }

    // just do show the wait cursor when the Gui is up
    Base::Sequencer().start("Load IGES", 1);
    Base::Sequencer().next();
    
    Handle(TopTools_HSequenceOfShape) aHSequenceOfShape = new TopTools_HSequenceOfShape;
    if (aReader.ReadFile((const Standard_CString)FileName.c_str()) != IFSelect_RetDone)
    {
      setError("File not readable");
      return 1;
    }
  
    // Root transfers
    Standard_Integer nbr = aReader.NbRootsForTransfer();

    for ( Standard_Integer n = 1; n<= nbr; n++)
    {
      printf("STEP: Transferring Root %d\n",n);
      aReader.TransferRoot(n);
      // Collecting resulting entities
      Standard_Integer nbs = aReader.NbShapes();
      if (nbs == 0) {
        aHSequenceOfShape.Nullify();
        return 1;
      } else {
        for (Standard_Integer i =1; i<=nbs; i++) 
        {
          printf("STEP:   Transferring Shape %d\n",n);
          aShape=aReader.Shape(i);
          aHSequenceOfShape->Append(aShape);
        }
      }
    }

    setShape(aShape);
    Base::Sequencer().stop();
  }
  catch(...){
    Base::Sequencer().halt();
    Base::Console().Error("FeaturePartImportStep::Execute() failed!");
    return 1;
  }

  return 0;
}


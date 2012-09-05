/***************************************************************************
cramp.hpp

  A C++ wrapper for the RAMP code.

  Use this library to parse an mzXML file in a non-sequential way, by
  taking advantage of the index element.  

  (C) 2004 by Brian Pratt, Insilicos LLC 

  Based on mzXML2Other, which has this copyright:
	 -------------------
	 begin					 : Wed Apr 2
	 copyright				 : (C) 2002 by Pedrioli Patrick, ISB, Proteomics
	 email					 : ppatrick@student.ethz.ch
 ***************************************************************************/

/***************************************************************************
*																								  *
*	 This program is free software; you can redistribute it and/or modify  *
*	 it under the terms of the GNU Library or "Lesser" General Public 	  *
*	 License (LGPL) as published by the Free Software Foundation;			  *
*	 either version 2 of the License, or (at your option) any later		  *
*	 version.																				  *
*																								  *
***************************************************************************/


#ifndef CRAMP_HPP_INCLUDED
#define CRAMP_HPP_INCLUDED

#include <iostream>
#include <string>
#include <string>
#include <stdlib.h>

using namespace std;

#include "ramp.h" // stuff we expose to C, structs etc

enum eWhatToRead { RAMP_RUNINFO, RAMP_HEADER , RAMP_PEAKS };

class rampScanInfo; // forward reference
class rampPeakList; // forward reference
class rampRunInfo; // forward reference
class rampInfo; // forward reference

class cRamp {
public:
   /**
    * constructor
    * @param fileName: Name of the msxml file
    *
    */
      cRamp( const char* fileName );
      virtual ~cRamp();

   /**
    * This function performs a non-sequential parsing operation on an indexed
    * msxml file to obtain minimal info on the msRun contained in the file.  

    *
    * @return cRampRunInfo* is dynamically allocate and becomes property of the caller, who
    *         is responsible for its deallocation!!
    */

   rampRunInfo* getRunInfo (  );


   /**
    * This function performs a non-sequential parsing operation on an indexed
    * msxml file to obtain minimal header info for a numbered scan (thus minimizing parse time).  

    *
    * @param whichScan: Number of the scan we want to read from
    * @return cRampScanInfo* is dynamically allocate and becomes property of the caller, who
    *         is responsible for its deallocation!! returns just the minimal header info num, msLevel and retentionTime
    */

   rampScanInfo* getScanHeaderInfo ( int whichScan  );


   /**
    * This function performs a non-sequential parsing operation on an indexed
    * msxml file to obtain peak info for a numbered scan.
    *
    * @param fileName: Name of the msxml file
    * @param whichScan: Number of the scan we want to read from
    * @return rampPeakList* is dynamically allocate and becomes property of the caller, who
    *         is responsible for its deallocation!! returns everything found in scan, precursorMz and peaks
    */

   rampPeakList* getPeakList ( int whichScan );

   /**
    * This function performs a non-sequential parsing operation on an indexed
    * msxml file to obtain everything found in scan, precursorMz and peaks (longest parse time).
    *
    * @param fileName: Name of the msxml file
    * @param whichScan: Number of the scan we want to read from
    * @return pData is dynamically allocate and becomes property of the caller, who
    *         is responsible for its deallocation!!
    */

    rampScanInfo* getScanInfo ( int whichScan) ;

    /**
      * checks the status of the object
      */
    bool OK() {
       return NULL!=m_handle;
    }
private:
   std::string m_filename;
   RAMPFILE *m_handle;
   rampRunInfo *m_runInfo; // scan count etc
   ramp_fileoffset_t *m_scanOffsets; // scan offset table
   rampInfo *do_ramp(ramp_fileoffset_t arg, eWhatToRead whatToRead);
};

class rampInfo { // abstract base class for things we can say about the XML file
public:
   rampInfo() {
   };
   virtual ~rampInfo() {
   };
};

typedef struct {
   RAMPREAL mz;
   RAMPREAL intensity;
} rampPeakInfoStruct;

class rampPeakList : public rampInfo {
public:
   rampPeakList(RAMPFILE *m_handle, ramp_fileoffset_t index); // populate from file at this position
private:
   int m_peaksCount;
   rampPeakInfoStruct *m_pPeaks; // as allocated by malloc()
public:
   rampPeakList(const rampPeakList &rhs) { // copy constructor
      *this = rhs;
   }

   void operator = (const rampPeakList &rhs) { // assignment operator
      init();
      if ((m_peaksCount = rhs.m_peaksCount) != 0) {
         m_pPeaks = (rampPeakInfoStruct *)malloc( sizeof(rampPeakInfoStruct) * rhs.m_peaksCount);
         if (m_pPeaks) {
            memmove(m_pPeaks,rhs.m_pPeaks,rhs.m_peaksCount*sizeof(rampPeakInfoStruct)); // bitwise copy
         } else {
            m_peaksCount = 0;
         }
      }
   }

   virtual ~rampPeakList() {
      free (m_pPeaks);
   }
protected:
   void init() {
      m_peaksCount = 0;
      m_pPeaks = NULL;
   }

public:

   int getPeakCount() const {
      return m_peaksCount;
   }
   rampPeakInfoStruct* getPeak(int n) {
      if ((n < 0) || (n>=m_peaksCount)) {
         return NULL;
      }
      return &m_pPeaks[n];
   }
};

class rampScanInfo: public rampInfo  {
public:
   //
   // constructors, destructors
   //
   rampScanInfo(RAMPFILE *m_handle, ramp_fileoffset_t index, int seqNum); // populate from file at this position, assign this sequence number

   rampScanInfo(rampScanInfo &rhs) { // copy constructor - note this moves a pointer from rhs to *this
      memmove(this,&rhs,sizeof(rhs));
   };
   virtual ~rampScanInfo() {
   }

   void init() {
      // Set the optional fields to -1 resp. na
      setRetentionTimeSeconds(-1);
      m_data.seqNum = -1; // this isn't optional, but serves as an "is unitialized" flag
      m_data.acquisitionNum = -1; // scan number as declared in file (may be gaps)
      m_data.lowMZ = -1;
      m_data.highMZ = -1;
      m_data.basePeakMZ = -1;
      m_data.basePeakIntensity = -1;
      m_data.totIonCurrent = -1;
      m_data.precursorMZ = -1;
      m_data.precursorScanNum = -1;
      m_data.precursorCharge = -1;
      m_data.collisionEnergy = -1;
      m_data.ionisationEnergy = -1;    
      m_data.filePosition = -1;
   }
   //
   // data members
   //
   ScanHeaderStruct m_data;
public:
   void setRetentionTimeSeconds(double t) {
      m_data.retentionTime = t;
   }
   double getRetentionTimeSeconds() const {
      return m_data.retentionTime;
   }
   int getPeakCount() const {
      return m_data.peaksCount;
   }

};

class rampRunInfo: public rampInfo  {
public:
   //
   // constructors, destructors
   //
   rampRunInfo(RAMPFILE *m_handle); // populate from file

   rampRunInfo(const rampRunInfo &rhs) { // copy constructor
      memmove(this,&rhs,sizeof(rhs));
      if (rhs.m_scanOffsets) { // need a deepcopy
         if (NULL!=(m_scanOffsets = (ramp_fileoffset_t *)malloc(m_data.scanCount*sizeof(ramp_fileoffset_t)))) {
            memmove(m_scanOffsets,rhs.m_scanOffsets,m_data.scanCount*sizeof(ramp_fileoffset_t));
         }
      }
   };
   virtual ~rampRunInfo() {
      free(m_scanOffsets);
   }

   void init() {
      m_data.scanCount = -1; // unknown
      m_scanOffsets = NULL;
   }
   //
   // data members
   //
   RunHeaderStruct m_data;
   ramp_fileoffset_t *m_scanOffsets;
};



#endif // ifndef CRAMP_HPP_INCLUDED

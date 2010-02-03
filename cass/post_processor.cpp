/*
 * 	CASS post-processor for LCLS
 *	Reads in XTC files and does preliminary data processing (including data export)
 * 	
 * 	post_processor.cpp skeleton written by Lutz Foucar 
 *	
 *	Adapted by Anton Barty to dump pnCCD images in the XTC file 
 *	into separate HDF5 files for later analysis
 */
 
#include "post_processor.h"
#include "cass_event.h"
#include "pnccd_event.h"
#include "machine_event.h"
#include "remi_event.h"
#include "vmi_event.h"
#include "pdsdata/xtc/Dgram.hh"
#include <time.h>
#include <hdf5.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <QFileInfo>
#include <QtGui/QLabel>
#include <QtGui>

#if H5_VERS_MAJOR < 2
#if H5_VERS_MINOR < 8
#define H5Dcreate1(A,B,C,D,E) H5Dcreate(A,B,C,D,E)
#define H5Gcreate1(A,B,C) H5Gcreate(A,B,C)
#define H5Lcreate_soft(A,B,C,D,E) 0
#endif
#endif


/* 
 *	Routine to print out info about current XTC iteration.
 * 	Used mainly as a verbose debug mode
 */
void postProcess_printinfo(cass::CASSEvent &cassevent) {

  	// Datagram reference pointer
		Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());

  	
	// Extract time and eventID from cassevent.id()
		// uint64_t bunchId = cassevent.datagrambuffer()->seq.clock().seconds();
		// bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
		//uint64_t eventID = cassevent.id();
		//time_t seconds = (time_t) (eventID>>32);
		//printf("Lutz time = %s",ctime(&seconds));

    // Extract time and fiducial directly from the datagram
		time_t eventTime = datagram->seq.clock().seconds();
		int32_t eventFiducial = datagram->seq.stamp().fiducials();
		

	// This information is always printed 
		printf("Time = %s", ctime(&eventTime));
		printf("casseventID = 0x%llX, fiducial = %i\n",(unsigned long long)cassevent.id(), eventFiducial);
		//printf("energy = %f\tEbeam charge = %f\n",cassevent.MachineDataEvent().energy(), cassevent.MachineDataEvent().EbeamCharge());
 
 
	// This information printed only in verbose mode
		if(cass::globalOptions.verbose) {
			printf("wavelength = %f nm\n",cass::PostProcessor::calculateWavelength(cassevent));
	
			int nframes = cassevent.pnCCDEvent().detectors().size();
			printf("nframes = %i\n",nframes);   
			//cassevent.pnCCDEvent().detectors()[0].correctedFrame().size());
			
			if (nframes >= 1) {
				printf("CCD1, Size = %ix%i\n",cassevent.pnCCDEvent().detectors()[0].rows(),
					cassevent.pnCCDEvent().detectors()[0].columns());
			}
			if (nframes == 2) {
				printf("CCD2, Size = %ix%i\n",cassevent.pnCCDEvent().detectors()[1].rows(),
					cassevent.pnCCDEvent().detectors()[1].columns());
			}
		}
}


/*
 *	Filter events to determine whether it's worth saving
 *	(mostly blank for now - needs to be filled in)
 */
int16_t postProcess_filter(cass::CASSEvent &) {

	/*
	 *	Filter according to time/date
	 */
		int16_t timeDate = 1;
		
	/*
	 *	Filter according to hit
	 */
		int16_t hit = 1;
		
	/*
	 *	Return result of filtering
	 */
		int16_t result = timeDate + hit;
		return(result);
}

/*
 *	export current pnCCD frames to HDF5 file
 */
void postProcess_writeHDF5(cass::CASSEvent &cassevent) {
  static QString xtcfile = QString("Unkown");

	/*
	 *	Simply return if there are no CCD frames!
	 */
  int nframes = cassevent.pnCCDEvent().detectors().size();
  if (nframes == 0) {
    printf("No pnCCD frames in this event:  skipping HDF5 write step...\n");
    return;
  }
  
  
  /*
   *	Create filename based on date, time and LCLS fiducial for this image
   */
  char outfile[1024];
  char buffer1[1024];
  char buffer2[1024];
  //  char buffer3[1024];
  /* Check if we get a valid filename. Otherwise just use previous filename */
  if(cassevent.filename() && cassevent.filename()[0] != 0){
    xtcfile =  QFileInfo(cassevent.filename()).baseName();
  }
  /*  const char* xtcfile = QFileInfo(cassevent.filename()).baseName().toAscii().constData();*/
  printf("xtcfile = %s\n",xtcfile.toAscii().constData());
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  // time_t eventTimeNs = datagram->seq.clock().nanoseconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  setenv("TZ","US/Pacific",1);
  struct tm *timeinfo=localtime( &eventTime );
  unsetenv("TZ");
  strftime(buffer1,80,"%Y_%b%d",timeinfo);
  strftime(buffer2,80,"%H%M%S",timeinfo);
  //strncpy(buffer3, strpbrk(xtcfile,"-")+1,5); 
  //printf("%s \n",xtcfile);
  //printf(outfile,"%s\n",cassevent.filename());


  sprintf(outfile,"LCLS_%s_%s_%i_pnCCD.h5",buffer1,buffer2,eventFiducial);
  //if(QFile::exists(outfile)){
  // sprintf(outfile,"%s_%i_pnCCD-part2.h5",buffer,eventFiducial);
  //}
  printf("Writing data to: %s\n",outfile);
  
  
  /* 
   *	Create the HDF5 file
   */
  hid_t 	hdf_fileID;
  hid_t 	dataspace_id;
  hid_t 	dataset_id;
  hid_t 	datatype;
  hsize_t dims[2];
  herr_t 	hdf_error;
  hid_t   gid;
  /* check if file exists */
  
  hdf_fileID = H5Fcreate(outfile,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  
  
  /*
   *	Write pnCCD frames to '/data' part of HDF5 file
   */
  gid = H5Gcreate1(hdf_fileID,"data",0);
  
  // Save each pnCCD frame in the XTC data set
  int skipped = 0;
  for(int i=0; i<nframes; i++) {
    int rows = cassevent.pnCCDEvent().detectors()[i].rows();
    int columns = cassevent.pnCCDEvent().detectors()[i].columns();
    if(!rows || !columns){
      skipped++;
      printf("pnCCD frame with ilogical size %dx%d!\n",columns,rows);
      continue;
    }
    int16_t *data = &cassevent.pnCCDEvent().detectors()[i].correctedFrame()[0];
    
    char fieldname[100]; 
    sprintf(fieldname,"/data/data%i",i-skipped);
    
    dims[0] = rows;
    dims[1] = columns;
    dataspace_id = H5Screate_simple( 2, dims, NULL);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    if( H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data)< 0){
      printf("Error when writing data %i...\n",i);
      return;
    }
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }
  
  // Save information on number of pnCCD frames saved
  dims[0] = 1;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  //dataspace_id = H5Screate(H5S_SCALAR);
  int adjusted_nframes = nframes-skipped;
  dataset_id = H5Dcreate1(hdf_fileID, "/data/nframes", H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&adjusted_nframes );
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  
  // Create symbolic link from /data/data0 to /data/data 
  // (to maintain our convention of /data/data always containing data)
  hdf_error = H5Lcreate_soft( "/data/data0", hdf_fileID, "/data/data",0,0);
  
  
  /*
   *	Write pnCCD configurations
   */
  H5Gclose(gid);
  gid = H5Gcreate1(hdf_fileID,"pnCCD",0);
  dims[0] = 1;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  //dataspace_id = H5Screate(H5S_SCALAR);
  
  dataset_id = H5Dcreate1(hdf_fileID, "/pnCCD/n_CCDs", H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&nframes );
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  
  skipped = 0;
  for(int i=0; i<nframes; i++) {
    char fieldname[100]; 
    int ccd_index = i-skipped;
    int rows = cassevent.pnCCDEvent().detectors()[i].rows();
    int columns = cassevent.pnCCDEvent().detectors()[i].columns();
    
    if(!rows || !columns){
      printf("pnCCD frame with ilogical size %dx%d!\n",columns,rows);
      skipped++;
      continue;
    }
    sprintf(fieldname,"/pnCCD/pnCCD%i",ccd_index);
    hid_t gid = H5Gcreate1(hdf_fileID,fieldname,0);
    dataspace_id = H5Screate_simple( 1, dims, NULL );
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/rows",ccd_index);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.pnCCDEvent().detectors()[i].rows() );
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/columns",ccd_index);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.pnCCDEvent().detectors()[i].columns() );
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/originalrows",ccd_index);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.pnCCDEvent().detectors()[i].originalrows() );
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/originalcolumns",ccd_index);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.pnCCDEvent().detectors()[i].originalcolumns() );
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/row_binning",ccd_index);
    int16_t rbin = cassevent.pnCCDEvent().detectors()[0].originalrows()/cassevent.pnCCDEvent().detectors()[i].rows();
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rbin);
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/column_binning",ccd_index);
    int16_t cbin = cassevent.pnCCDEvent().detectors()[0].originalcolumns()/cassevent.pnCCDEvent().detectors()[i].columns();
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cbin);
    H5Dclose(dataset_id);
    
    sprintf(fieldname,"/pnCCD/pnCCD%i/integral",ccd_index);
    dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_LONG, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.pnCCDEvent().detectors()[i].integral() );
    H5Dclose(dataset_id);
    
    
    H5Sclose(dataspace_id);
    H5Gclose(gid);
    
  }
  
  /*
   *	Write LCLS event information
   */
  gid = H5Gcreate1(hdf_fileID,"LCLS",0);
  dims[0] = 1;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  //dataspace_id = H5Screate(H5S_SCALAR);
  
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/machineTime", H5T_NATIVE_UINT32, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &eventTime );
  H5Dclose(dataset_id);
  
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/fiducial", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &eventFiducial );
  H5Dclose(dataset_id);
  
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/energy", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().energy() );
  H5Dclose(dataset_id);
  
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamCharge", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamCharge() );
  H5Dclose(dataset_id);
  

  // Gas detector values
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_11_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().f_11_ENRC() );
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_12_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().f_12_ENRC() );
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_21_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().f_21_ENRC() );
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_22_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().f_22_ENRC() );
  H5Dclose(dataset_id);
    
    
  double resonantPhotonEnergy = cass::PostProcessor::calculatePhotonEnergy(cassevent); 
  double resonantPhotonEnergyNoEnergyLossCorrection = cass::PostProcessor::calculatePhotonEnergyWithoutLossCorrection(cassevent);
  double wavelength_nm = -1; 
  double wavelength_A = -1;
  
  if(resonantPhotonEnergy){
	  wavelength_nm = 1239.8/resonantPhotonEnergy;
	  wavelength_A = 10*wavelength_nm;
  }

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamL3Energy", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamL3Energy());
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_energy_eV", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &resonantPhotonEnergy);
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_energy_eV_no_energy_loss_correction", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &resonantPhotonEnergyNoEnergyLossCorrection);
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_wavelength_nm", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &wavelength_nm);
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_wavelength_A", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &wavelength_A);
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamLTUPosX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamLTUPosX());
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamLTUPosY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamLTUPosY());
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamLTUAngX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamLTUAngX());
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamLTUAngY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamLTUAngY());
  H5Dclose(dataset_id);
  dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/EbeamPkCurrBC2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamPkCurrBC2());
  H5Dclose(dataset_id);
 
  H5Sclose(dataspace_id);
  
  
  // Time in human readable format
  // Strings are a little tricky --> this could be improved!
  char* timestr;
  setenv("TZ","US/Pacific",1);
  timestr = ctime(&eventTime);
  unsetenv("TZ");
  dataspace_id = H5Screate(H5S_SCALAR);
  datatype = H5Tcopy(H5T_C_S1);  
  H5Tset_size(datatype,strlen(timestr)+1);
  dataset_id = H5Dcreate1(hdf_fileID, "LCLS/eventTimeString", datatype, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, timestr );
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  hdf_error = H5Lcreate_soft( "/LCLS/eventTimeString", hdf_fileID, "/LCLS/eventTime",0,0);


  // Put the XTC filename somewhere
  dataspace_id = H5Screate(H5S_SCALAR);
  datatype = H5Tcopy(H5T_C_S1);  
  printf("xtcfile = %s\n",xtcfile.toAscii().constData());
  H5Tset_size(datatype,xtcfile.length()+1);
  dataset_id = H5Dcreate1(hdf_fileID, "LCLS/xtcFilename", datatype, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, xtcfile.toAscii().constData() );
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);



  H5Gclose(gid);
  H5Fflush(hdf_fileID,H5F_SCOPE_LOCAL);
  H5Fclose(hdf_fileID); 
  H5close();
}


/*
 *	This is the callback function for each XTC iteration
 */
void cass::PostProcessor::postProcess(cass::CASSEvent &cassevent)
{
  if(globalOptions.onlyAppendWavelength){
    appendWavelength(cassevent);
    return;
  }
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  if(cass::globalOptions.startTime.isValid()){
    //    printf("Start Time %s\n",cass::globalOptions.startTime.toString().toAscii().constData());
    //    printf("Event Time %s\n",QDateTime::fromTime_t(eventTime).toString().toAscii().constData());
    //    printf("startTime valid\n");
  }
  if(cass::globalOptions.endTime.isValid()){
    //    printf("%s\n",cass::globalOptions.endTime.toString().toAscii().constData());
    //    printf("endTime valid\n");
  }
  if(cass::globalOptions.startTime.isValid() && 
     QDateTime::fromTime_t(eventTime).time() < cass::globalOptions.startTime.time()){
    //      printf("Skipping frame before startTime at postProcess\n");
    return;
  }
  if(cass::globalOptions.endTime.isValid() && 
     QDateTime::fromTime_t(eventTime).time() > cass::globalOptions.endTime.time()){
    //    printf("Skipping frame after endTime at postProcess\n");
    return;
  }

  if(cass::globalOptions.eventCounter % cass::globalOptions.skipPeriod != 0){
    //      printf("Skipping frame not in the beggining of period\n");
      return;
  }

  postProcess_printinfo(cassevent);
  
  calculateWavelength(cassevent);

  if(cass::globalOptions.outputAllEvents || isGoodImage(cassevent)){	  
    //	  integrateByQ(cassevent);
    //	  extractEnergy(cassevent);
    if(cass::globalOptions.justIntegrateImages == false){
      postProcess_writeHDF5(cassevent);
    }
    if(cass::globalOptions.justIntegrateImages == true ||
       cass::globalOptions.alsoIntegrateImages){
      addToIntegratedImage(cassevent);
      if(cass::globalOptions.eventCounter % 10 == 0){
	finishProcessing();
      }
    }
  }
  printf("\n");
}

void cass::PostProcessor::appendWavelength(cass::CASSEvent &cassevent){
  double wavelength = calculateWavelength(cassevent);

	/*
	 *	Simply return if there are no CCD frames!
	 */
  //int nframes = cassevent.pnCCDEvent().detectors().size();
  /*  if (nframes == 0) {
    printf("No pnCCD frames in this event:  skipping wavelength append step...\n");
        return;
	}*/
  
  
  /*
   *	Create filename based on date, time and LCLS fiducial for this image
   */
  char outfile[1024];
  char buffer[1024];
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  //		time_t eventTimeNs = datagram->seq.clock().nanoseconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  setenv("TZ","US/Pacific",1);
  struct tm *timeinfo=localtime( &eventTime );
  unsetenv("TZ");
  strftime(buffer,80,"LCLS_%Y_%b%d_%H%M%S",timeinfo);
  sprintf(outfile,"%s_%i_pnCCD.h5",buffer,eventFiducial);
  if(!QFile::exists(outfile)){
    //    printf("File %s does not exist!\n",outfile);
    return;
  }
  hid_t hdf_fileID = H5Fopen(outfile,  H5F_ACC_RDWR, H5P_DEFAULT);
  hsize_t dims = 1;
  hid_t dataspace_id = H5Screate_simple( 1, &dims, NULL );
  hid_t dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/wavelength", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &wavelength);
  H5Dclose(dataset_id);
  H5close();  
}


double cass::PostProcessor::calculatePhotonEnergyWithoutLossCorrection(cass::CASSEvent &cassevent){
  /* 
     Calculate the resonant photon energy (without any energy loss corrections)
     Use the simple expression in e.g. Ayvazyan, V. et al. (2005). This expression requires
     1) electron energy (in the xtc files)
     2) undulator period (~3cm for LCLS)
     3) undulator K (~3.5 at the LCLS)
  */
  
  const double   ebEnergy = cassevent.MachineDataEvent().EbeamL3Energy();
  if(!ebEnergy){
    return NAN;
  }
  const double   K = 3.5;  // K of the undulator (provided by Marc Messerschmidt)
  const double   lambda = 3.0e7; // LCLS undulator period in nm
  const double   hc = 1239.84172; // in eV*nm
  double gamma = ebEnergy/(0.510998903);  // electron energy in rest mass units (E/mc^2)
  double resonantPhotonEnergy = hc*2*gamma*gamma/(lambda*(1+K*K/2)); // resonant photon wavelength in same units as undulator period)
  //  printf("Resonant photon energy, Rick's calculation, no energy loss corrections: %g eV\n",resonantPhotonEnergy);
  //  return 1e10*1.98644e-25/(resonantPhotonEnergy*1.602176e-19);  <-- this is off by a factor of 10
  return resonantPhotonEnergy;
}

bool cass::PostProcessor::isGoodImage(cass::CASSEvent &cassevent){
  bool good = true;
  long long integral;
  if(cass::globalOptions.useIntegrationThreshold){
    integral = integrateImage(cassevent,cass::globalOptions.justIntegrateImagesThreshold);    
  }else{
    integral = integrateImage(cassevent);
  }
  if(integral <= 0){
    return false;
  }
  if(cass::globalOptions.useIntegrationThreshold && integral <
     cass::globalOptions.useIntegrationThreshold*3){
    /* don't accept image with just a couple of high pixels*/
    return false;
  }

  double stdDev = stdDevImage(cassevent,integral);
  if(stdDev < sqrt(1000)){
    return false;
  }
  if(cass::globalOptions.outputHitsToFile && good){
    Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
    long long int bunchId = datagram->seq.clock().seconds();
    bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
    FILE * fp = fopen(cass::globalOptions.hitsOutputFile.toAscii().constData(),"a");
    fprintf(fp,"%llu\n",bunchId);
  }
  return good;
}

long long cass::PostProcessor::integrateImage(cass::CASSEvent &cassevent, float threshold){
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  //		time_t eventTimeNs = datagram->seq.clock().nanoseconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  setenv("TZ","US/Pacific",1);
  struct tm *timeinfo=localtime( &eventTime );
  unsetenv("TZ");
  char buffer[1024];
  char outfile[1024];
  strftime(buffer,80,"LCLS_%Y_%b%d_%H%M%S",timeinfo);
  sprintf(outfile,"%s_%i_pnCCD.h5",buffer,eventFiducial);

  long long ret = 0;
  int nframes = cassevent.pnCCDEvent().detectors().size();
  for(int frame=0; frame<nframes; frame++) {
    if(cass::globalOptions.discardCCD[frame]){
      continue;
    }
    int rows = cassevent.pnCCDEvent().detectors()[frame].rows();
    int columns = cassevent.pnCCDEvent().detectors()[frame].columns();
    int16_t *data = &cassevent.pnCCDEvent().detectors()[frame].correctedFrame()[0];
    int i = 0;
    for(int y = 0;y<rows;y++){
      for(int x = 0;x<columns;x++){
	if(cass::globalOptions.useSignalMask[frame] == false || 
	   (cass::globalOptions.signalMask[frame].pixel(x,y) & 0xffffff)){
	   if(!threshold || threshold < data[i]){
	    ret += data[i];
	  }
	}
	i++;
      }
    }
  }
  if(ret){
    printf("***** SIGNAL %lld for %s\n",ret,outfile);
  }
  return ret;
}


double cass::PostProcessor::stdDevImage(cass::CASSEvent &cassevent,long long integral){
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  //		time_t eventTimeNs = datagram->seq.clock().nanoseconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  setenv("TZ","US/Pacific",1);
  struct tm *timeinfo=localtime( &eventTime );
  unsetenv("TZ");
  char buffer[1024];
  char outfile[1024];
  strftime(buffer,80,"LCLS_%Y_%b%d_%H%M%S",timeinfo);
  sprintf(outfile,"%s_%i_pnCCD.h5",buffer,eventFiducial);
  
  double ret = 0;
  int nframes = cassevent.pnCCDEvent().detectors().size();
  int totalSize = 0;
  for(int frame=0; frame<nframes; frame++) {
    if(cass::globalOptions.discardCCD[frame]){
      continue;
    }
    int rows = cassevent.pnCCDEvent().detectors()[frame].rows();
    int columns = cassevent.pnCCDEvent().detectors()[frame].columns();    
    for(int x = 0;x<columns;x++){
      for(int y = 0;y<rows;y++){	
	if(cass::globalOptions.useSignalMask[frame] == false || 
	   (cass::globalOptions.signalMask[frame].pixel(x,y) & 0xffffff)){
	  totalSize++;
	}
      }
    }
  }
  double average = (double)integral/totalSize;
  for(int frame=0; frame<nframes; frame++) {
    if(cass::globalOptions.discardCCD[frame]){
      continue;
    }
    int rows = cassevent.pnCCDEvent().detectors()[frame].rows();
    int columns = cassevent.pnCCDEvent().detectors()[frame].columns();
    int16_t *data = &cassevent.pnCCDEvent().detectors()[frame].correctedFrame()[0];
    int i = 0;
    for(int x = 0;x<columns;x++){
      for(int y = 0;y<rows;y++){	
	if(cass::globalOptions.useSignalMask[frame] == false || 
	   cass::globalOptions.signalMask[frame].pixel(x,y) & 0xffffff){
	  
	  ret += (data[i]-average)*(data[i]-average);
	  i++;
	}
      }
    }
  }
  ret /= totalSize;
  ret = sqrt(ret);
  if(ret){
    printf("***** STDDEV %f for %s\n",ret,outfile);
  }
  return ret; 
}

void cass::PostProcessor::integrateByQ(cass::CASSEvent &cassevent)
{
  int nframes = cassevent.pnCCDEvent().detectors().size();
  if (nframes == 0) {
    printf("No pnCCD frames in this event:  skipping HDF5 write step...\n");
    return;
  }
  printf("Processing %s\n",cassevent.filename());
  for(int frame=0; frame<nframes; frame++) {
    int rows = cassevent.pnCCDEvent().detectors()[frame].rows();
    int columns = cassevent.pnCCDEvent().detectors()[frame].columns();
    int16_t *data = &cassevent.pnCCDEvent().detectors()[frame].correctedFrame()[0];
    int max_q = (int)(ceil(sqrt(rows/2.0*rows/2.0+columns/2.0*columns/2.0)));
    int nbins = max_q;
    float * bins = new float[nbins];
    float * bin_distance = new float[nbins];
    int * bin_members = new int[nbins];

    for(int i = 0;i<nbins;i++){
      bin_members[i] = 0;
      bins[i] = 0;
      bin_distance[i] = (float)max_q*i/nbins;
    }
    int i = 0;
    for(int x = 0;x<columns;x++){
      for(int y = 0;y<rows;y++){
	float rel_x = x-columns/2.0;
	float rel_y = y-rows/2.0;
	int bin = (int)(nbins*sqrt(rel_x*rel_x+rel_y*rel_y)/max_q);
	bin_members[bin]++;
	bins[bin] += data[i];
	i++;
      }
    }
    for(int i = 0;i<nbins;i++){
      if(bin_members[i]){
	bins[i] /= bin_members[i];
      }
    }    
    appendIntegratedByQ(cassevent,bin_distance,bins,nbins,frame);
  }
}

void cass::PostProcessor::appendIntegratedByQ(cass::CASSEvent &cassevent,float * x, float * y,int n, int frame){
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  char outfile[1024];
  sprintf(outfile,"%s_I_by_Q.csv",QFileInfo(cassevent.filename()).baseName().toAscii().constData());
  FILE * fp = fopen(outfile,"a");
  for(int i = 0;i<n;i++){
    fprintf(fp,"%u,%d,%d,%f,%f\n",(unsigned int)eventTime,eventFiducial,frame,x[i],y[i]);
  }  
  fclose(fp);
}

void cass::PostProcessor::extractEnergy(cass::CASSEvent &cassevent){
  //  int nframes = cassevent.pnCCDEvent().detectors().size();
  char outfile[1024];
  sprintf(outfile,"%s_energy.csv",QFileInfo(cassevent.filename()).baseName().toAscii().constData());
  //sprintf(outfile,"dummy_energy.csv");

  FILE * fp = fopen(outfile,"a");
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  printf("processing %d\n",datagram->seq.stamp().fiducials());

  time_t eventTime = datagram->seq.clock().seconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  double f11 = cassevent.MachineDataEvent().f_11_ENRC();
  double f12 = cassevent.MachineDataEvent().f_12_ENRC();
  double f21 = cassevent.MachineDataEvent().f_21_ENRC();
  double f22 = cassevent.MachineDataEvent().f_22_ENRC();
  fprintf(fp,"%u %d %f %f %f %f\n",(unsigned int)eventTime,eventFiducial,f11,f12,f21,f22);
  fclose(fp);
}
					      

namespace cass{
  void PostProcessor::addToIntegratedImage(CASSEvent &cassevent){
    int nframes = cassevent.pnCCDEvent().detectors().size();
    if(nframes == 0){
      /* this is not really a pnCCD event */
      return;
    }
    if(firstIntegratedImage){
      firstIntegratedImage = false;
      integratedImage = HDRImage(cassevent);
    }else{
      integratedImage.addToImage(cassevent);
    }
  }

  PostProcessor::PostProcessor()   
  {
    printf("Post_processor creator called here\n");
    firstIntegratedImage = true;
  }

  HDRImage::HDRImage(){
    m_nframes = 0;
  }

  HDRImage::HDRImage(CASSEvent & cassevent){
    m_nframes = cassevent.pnCCDEvent().detectors().size();
    for(int i=0; i<m_nframes; i++) {
      m_rows.append(cassevent.pnCCDEvent().detectors()[i].rows());
      m_columns.append(cassevent.pnCCDEvent().detectors()[i].columns());
      if(m_rows.last()*m_columns.last() == 0){
	/* dummy */
	m_data.append(new double[1]);	
	m_nImagesAdded.append(new int[1]);	
      }else{
	m_data.append(new double[m_rows.last()*m_columns.last()]);	
	m_nImagesAdded.append(new int[m_rows.last()*m_columns.last()]);	
	for(int j = 0;j<m_rows.last()*m_columns.last();j++){
	  m_data.last()[j] = 0;
	  m_nImagesAdded.last()[j] = 0;
	}
      }
    }
  }
  HDRImage::~HDRImage(){
    for(int i = 0;i<m_nframes;i++){
      if(m_data[i]){
	//	delete m_data[i];
      }
    }
  }
      
  void HDRImage::addToImage(CASSEvent &cassevent){
    int nframes = cassevent.pnCCDEvent().detectors().size();
    if(m_nframes != nframes){
      printf("Number of frames doesn't match!\n");
    }
    for(int i=0; i<nframes; i++) {
	int rows = cassevent.pnCCDEvent().detectors()[i].rows();
	int columns = cassevent.pnCCDEvent().detectors()[i].columns();
	if(rows != m_rows[i]){
	  printf("Number of rows doesn't match!\n");
	}
	if(columns != m_columns[i]){
	  printf("Number of cols doesn't match!\n");
	}
	int16_t *data = &cassevent.pnCCDEvent().detectors()[i].correctedFrame()[0];
	int j = 0;
	for(int x = 0;x<columns;x++){
	  for(int y = 0;y<rows;y++){	
	    if(cass::globalOptions.nImagesToAverage){
	      m_data[i][j] *= (1-1.0/cass::globalOptions.nImagesToAverage);
	    }
	    /*	    if(cass::globalOptions.useSignalMask[i] == false || 
	      (cass::globalOptions.signalMask[i].pixel(x,y) & 0xffffff)){*/
	    
	    if((cass::globalOptions.useIntegrationFloor &&
		data[j] > cass::globalOptions.integrationFloor) ||
	       !cass::globalOptions.useIntegrationThreshold ||
	       data[j] > cass::globalOptions.justIntegrateImagesThreshold){
	      if(!cass::globalOptions.useIntegrationCeiling ||
		 data[j] < cass::globalOptions.integrationCeiling){
		m_data[i][j] += data[j];
		m_nImagesAdded[i][j]++;
	      }
	    }
	    j++;
	  }
	}
    }

  }
  void HDRImage::outputImage(const char * filename){
    hid_t 	hdf_fileID;
    hid_t 	dataspace_id;
    hid_t 	dataset_id;
    hsize_t dims[2];
    hdf_fileID = H5Fcreate(filename,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hid_t gid = H5Gcreate1(hdf_fileID,"data",0);
    // Save each pnCCD frame in the XTC data set
    int skipped = 0;
    int nframes = m_nframes;
    for(int i=0; i<nframes; i++) {
      int rows = m_rows[i];
      int columns = m_columns[i];
      if(!rows || !columns){
	skipped++;
	continue;
      }
      double *data = m_data[i];

      char fieldname[100]; 
      sprintf(fieldname,"/data/data%i",i-skipped);
      
      dims[0] = rows;
      dims[1] = columns;
      dataspace_id = H5Screate_simple( 2, dims, NULL);
      dataset_id = H5Dcreate1(hdf_fileID, fieldname, H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
      if( H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data)< 0){
	printf("Error when writing data %i...\n",i);
	return;
      }
      H5Dclose(dataset_id);
      H5Sclose(dataspace_id);
    }
    dims[0] = 1;
    dataspace_id = H5Screate_simple( 1, dims, NULL );
    int adjusted_nframes = nframes-skipped;
    dataset_id = H5Dcreate1(hdf_fileID, "/data/nframes", H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&adjusted_nframes );
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    H5Lcreate_soft( "/data/data0", hdf_fileID, "/data/data",0,0);
    H5Gclose(gid);
    H5Fclose(hdf_fileID);    
  }

  QImage HDRImage::toQImage(int frame,double maxModifier,double minModifier,int useLog){
    if(m_nframes <= frame){
      return QImage();
    }
    double min = m_data[frame][0];
    double max = m_data[frame][0];
    for(int i = 0;i<m_rows[frame]*m_columns[frame];i++){
      if(max < m_data[frame][i]){
	max = m_data[frame][i];
      }
      if(min > m_data[frame][i]){
	min = m_data[frame][i];
      }      
    }
    if(useLog){
      max = exp(maxModifier*(log(max-min)))+min;
      min = exp(minModifier*(log(max-min)))+min;
    }else{
      max = maxModifier*(max-min)+min;
      min = minModifier*(max-min)+min;
    }
    if(!(isfinite(max) && isfinite(min))){
      return QImage();
    }
    int colormap = SpColormapJet;
    if(useLog){
      colormap |= SpColormapLogScale;
    }
    unsigned char * colormap_data = sp_image_get_false_color(frame,colormap,min,max);
    return QImage(colormap_data,m_columns[frame],m_rows[frame],QImage::Format_RGB32);
  }
    

  HDRImage::sp_rgb HDRImage::colormap_rgb_from_value(double value, int colormap){
    sp_rgb ret = {0,0,0};
    if(colormap & SpColormapGrayScale){       
      ret.r = value;
      ret.g = value;
      ret.b = value;
    }else if(colormap & SpColormapTraditional){       
      ret.r = sqrt(value);
      ret.g = value*value*value;
      ret.b = sin(value*2*M_PI);
    }else if(colormap & SpColormapHot){
      ret.r = 3*value;
      ret.g = 3*value-1;
      ret.b = 3*value-2;
    }else if(colormap & SpColormapRainbow){
      ret.r = fabs(2*value-0.5);
      ret.g = sin(value*M_PI);
      ret.b = cos(value*M_PI/2);
    }else if(colormap & SpColormapJet){
      if(value < 1/8.0){
	ret.r = 0;
	ret.g = 0;
	ret.b = (value+1.0/8.0)*4;	   
      }else if(value < 3/8.0){
	ret.r = 0;
      ret.g = (value-1.0/8.0)*4;
      ret.b = 1;
      }else if(value < 5/8.0){
	ret.r = (value-3.0/8.0)*4;
	ret.g = 1;
	ret.b = 1-(value-3.0/8.0)*4;
      }else if(value < 7/8.0){
	ret.r = 1;
	ret.g = 1-(value-5.0/8.0)*4;
	ret.b = 0;
      }else if(value <= 1.01){
	ret.r = 1-(value-7.0/8.0)*4;;
	ret.g = 0;
	ret.b = 0;
      }
    }
    
    ret.r = qMin((double)1,ret.r);
    ret.g = qMin((double)1,ret.g);
    ret.b = qMin((double)1,ret.b);
    ret.r = qMax((double)0,ret.r);
    ret.g = qMax((double)0,ret.g);
    ret.b = qMax((double)0,ret.b);
    ret.r *= 255;
    ret.g *= 255;
    ret.b *= 255;
    
    if(colormap & SpColormapWheel){
      hsv_to_rgb(360*value,1.0,1.0,&ret.r,&ret.g,&ret.b);
    }
    return ret;
  }

  void HDRImage::hsv_to_rgb(double H,double S,double V,double * R,double *G,double *B){
    if( V == 0 ){ 
      *R = 0;
      *G = 0;
      *B = 0; 
    }else if( S == 0 ) {                                                                   
      *R = V;                                                            
      *G = V;                                                            
    *B = V;                                                            
    } else {                                                                   
      const double hf = H / 60.0;                                       
      const int    i  = (int) floor( hf );                              
      const double f  = hf - i;                                         
      const double pv  = V * ( 1 - S );                                 
      const double qv  = V * ( 1 - S * f );                             
      const double tv  = V * ( 1 - S * ( 1 - f ) );                     
      switch( i ){                                                               
      case 0: 
	*R = V; 
	*G = tv;
	*B = pv;
	break; 
      case 1:
	*R = qv;
	*G = V;
	*B = pv;
	break;
      case 2:
	*R = pv; 
	*G = V;
	*B = tv;
	break; 
      case 3: 
	*R = pv;
	*G = qv;
	*B = V;
	break;
      case 4:  
	*R = tv; 
	*G = pv;
	*B = V;
	break;  
      case 5:
	*R = V;
	*G = pv;
	*B = qv; 
	break;
      case 6: 
	*R = V;
	*G = tv;    
	*B = pv; 
	break; 
      case -1:  
	*R = V;
	*G = pv; 
	*B = qv;
	break;
      default:
	fprintf(stderr,"i Value error in HSV to *R*G*B conversion, Value is %d",i);
	break;
      }									
    }									
    *R *= 255.0F;                                                        
    *G *= 255.0F;                                                        
    *B *= 255.0F;  
  }


  void HDRImage::colormap_create_table(sp_rgb color_table[256],int colormap){
    for(int i = 0;i<256;i++){
      double value = i/255.0;
    color_table[i] = colormap_rgb_from_value(value,colormap);
    }
  }
  
  unsigned char * HDRImage::sp_image_get_false_color(int frame,int color, double min, double max){

    int i;
    //int x,y;
    double log_of_scale;
    sp_rgb color_table[256];
    double scale,offset,max_v,min_v,value;
    //double phase;
    unsigned char * out = new unsigned char[m_rows[frame]*m_columns[frame]*4];
  
  /*fclose(fp);
    return 0;*/
  max_v = m_data[frame][0];
  min_v = m_data[frame][0];
  
  colormap_create_table(color_table,color);
  
  
  max_v = max;
  min_v = min;
  scale = 65535/(max_v-min_v);
  offset = min_v;
  i = 0;
  log_of_scale = log(65536);
  /* this is a special kind of color */
  for(int i = 0;i<m_rows[frame]*m_columns[frame];i++){
    /* traditional color scale taken from gnuplot manual */
    value = qMin(m_data[frame][i],max_v);
    value = qMax(value,min_v);
    value -= offset;
    value *= scale;
    
    if(color & SpColormapLogScale){
      value = log(value+1)/log_of_scale;
    }else{
      value /= 65535;
    }
    value *= 255;
    out[i*4+2] =  color_table[(int)value].r;
    out[i*4+1] = color_table[(int)value].g;
    out[i*4] = color_table[(int)value].b;    
    i++;
  }
  return out;
  }

}


/* 
   Returns the photon energy in eV. 
   It uses Rick K. code at psexport:/reg/neh/home/rkirian/ana2 
*/
double cass::PostProcessor::calculatePhotonEnergy(cass::CASSEvent &cassevent){
  /*
   * Get electron beam parameters from beamline data
   */     

  //double fEbeamCharge = cassevent.MachineDataEvent().EbeamCharge();    // in nC
  double fEbeamL3Energy = cassevent.MachineDataEvent().EbeamL3Energy();  // in MeV 
  double fEbeamPkCurrBC2 = cassevent.MachineDataEvent().EbeamPkCurrBC2(); // in Amps

  /*
   * calculate the resonant photon energy
   */

  // Get the present peak current in Amps
  double peakCurrent = fEbeamPkCurrBC2;
  
  // Get present beam energy [GeV]
  double DL2energyGeV = 0.001*fEbeamL3Energy;

  // wakeloss prior to undulators
  double LTUwakeLoss = 0.0016293*peakCurrent;

  // Spontaneous radiation loss per segment
  double SRlossPerSegment = 0.63*DL2energyGeV;

  // wakeloss in an undulator segment
  double wakeLossPerSegment = 0.0003*peakCurrent;

  // energy loss per segment
  double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;

  // energy in first active undulator segment [GeV]
  double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss - 0.0005*energyLossPerSegment;

  // Calculate the resonant photon energy of the first active segment
  double photonEnergyeV = 44.42*energyProfile*energyProfile;

  return photonEnergyeV;
}

double cass::PostProcessor::calculateWavelength(cass::CASSEvent &cassevent){
  double E = calculatePhotonEnergy(cassevent);
  if(E){
    return 1398.8/E;
  }
  return -1;
}

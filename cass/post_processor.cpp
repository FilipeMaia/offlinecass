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
		

	// Print information 
		printf("Time = %s", ctime(&eventTime));
		printf("casseventID = 0x%lX, fiducial = %i\n", cassevent.id(), eventFiducial);
		printf("energy = %f\t%f\n",cassevent.MachineDataEvent().energy(), cassevent.MachineDataEvent().EbeamCharge());

 
	// Information on pnCCD frames
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
		char buffer[1024];
		Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
		time_t eventTime = datagram->seq.clock().seconds();
		//		time_t eventTimeNs = datagram->seq.clock().nanoseconds();
		int32_t eventFiducial = datagram->seq.stamp().fiducials();
		struct tm *timeinfo=localtime( &eventTime );
		strftime(buffer,80,"LCLS_%Y_%b%d_%H%M%S",timeinfo);
		sprintf(outfile,"%s_%i_pnCCD.h5",buffer,eventFiducial);
		if(QFile::exists(outfile)){
		  sprintf(outfile,"%s_%i_pnCCD-part2.h5",buffer,eventFiducial);
		}
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

		/* check if file exists */

		hdf_fileID = H5Fcreate(outfile,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);


	/*
	 *	Write pnCCD frames to '/data' part of HDF5 file
	 */
		H5Gcreate1(hdf_fileID,"data",0);

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
	    int adjusted_nframes = nframes=skipped;
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
		H5Gcreate1(hdf_fileID,"pnCCD",0);
	    dims[0] = 1;
	    dataspace_id = H5Screate_simple( 1, dims, NULL );

	    dataset_id = H5Dcreate1(hdf_fileID, "/pnCCD/n_CCDs", H5T_NATIVE_SHORT, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&nframes );
	    H5Dclose(dataset_id);

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
			H5Gcreate1(hdf_fileID,fieldname,0);
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
	 	}

	/*
	 *	Write LCLS event information
	 */
		H5Gcreate1(hdf_fileID,"LCLS",0);
	    dims[0] = 1;
	    dataspace_id = H5Screate_simple( 1, dims, NULL );

	    dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/machineTime", H5T_NATIVE_UINT32, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &eventTime );
	    H5Dclose(dataset_id);

	    dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/fiducial", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &eventFiducial );
	    H5Dclose(dataset_id);

	    dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/energy", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().energy() );
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


	    dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamCharge", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cassevent.MachineDataEvent().EbeamCharge() );
	    H5Dclose(dataset_id);

		H5Sclose(dataspace_id);

		// Strings are a little tricky --> this could be improved!
		char* timestr;
		timestr = ctime(&eventTime);
		dataspace_id = H5Screate(H5S_SCALAR);
	  	datatype = H5Tcopy(H5T_C_S1);  
	  	H5Tset_size(datatype,strlen(timestr));
	    dataset_id = H5Dcreate1(hdf_fileID, "LCLS/eventTime", datatype, dataspace_id, H5P_DEFAULT);
	    H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, timestr );
	    H5Dclose(dataset_id);
	    H5Sclose(dataspace_id);
	    H5Fclose(hdf_fileID);

		
}


/*
 *	This is the callback function for each XTC iteration
 */
void cass::PostProcessor::postProcess(cass::CASSEvent &cassevent)
{
	postProcess_printinfo(cassevent);

	if(isGoodImage(cassevent)){	  
	  openOutputFiles(cassevent);
	  //	  integrateByQ(cassevent);
	  //	  extractEnergy(cassevent);
	  postProcess_writeHDF5(cassevent);
	}
	printf("\n");
}

bool cass::PostProcessor::isGoodImage(cass::CASSEvent &cassevent){
  bool good = true;
  if(integrateImage(cassevent) < 0){
    good = false;
  }
  return good;
}

long long cass::PostProcessor::integrateImage(cass::CASSEvent &cassevent){
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent.datagrambuffer());
  time_t eventTime = datagram->seq.clock().seconds();
  //		time_t eventTimeNs = datagram->seq.clock().nanoseconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();
  struct tm *timeinfo=localtime( &eventTime );
  char buffer[1024];
  char outfile[1024];
  strftime(buffer,80,"LCLS_%Y_%b%d_%H%M%S",timeinfo);
  sprintf(outfile,"%s_%i_pnCCD.h5",buffer,eventFiducial);

  long long ret = 0;
  int nframes = cassevent.pnCCDEvent().detectors().size();
  for(int frame=0; frame<nframes; frame++) {
    int rows = cassevent.pnCCDEvent().detectors()[frame].rows();
    int columns = cassevent.pnCCDEvent().detectors()[frame].columns();
    int16_t *data = &cassevent.pnCCDEvent().detectors()[frame].correctedFrame()[0];
    for(int i = 0;i<rows*columns;i++){
      ret += data[i];
    }
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
					      

void cass::PostProcessor::openOutputFiles(CASSEvent &cassevent){
  if(!currentXtcFile || strcmp(currentXtcFile,cassevent.filename())){
    /* files are different. Open new file */
    currentXtcFile = cassevent.filename();    
  }
}

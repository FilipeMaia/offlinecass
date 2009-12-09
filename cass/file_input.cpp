// Copyright (C) 2009 LMF

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include "file_input.h"
#include "pdsdata/xtc/Dgram.hh"
#include "cass_event.h"

cass::FileInput::FileInput(const char *filelistname,lmf::RingBuffer<cass::CASSEvent,4> &ringbuffer, QObject *parent)
       :QThread(parent),
        _ringbuffer(ringbuffer),
        _quit(false),
        _filelistname(filelistname)
{
}

cass::FileInput::~FileInput()
{
}

void cass::FileInput::run()
{
  //create a list where all files that should be processed are in//
  std::vector<std::string> filelist;
  
  //open the file with the filenames in it//
  std::ifstream filelistfile;
  filelistfile.open(_filelistname);
  //put the names into a list of things that we want to process//
  if (filelistfile.is_open())
  {
    std::cout <<"filelist \""<<_filelistname<<"\" is open"<<std::endl;
    //go through whole file//
    while (!filelistfile.eof())
    {
      //read a line and put it into the file list//
      std::string line;
      getline(filelistfile,line);
      filelist.push_back(line);
      std::cout <<"file \""<<line<<"\" added to processing list"<<std::endl;
    }
  }
  
  //make a pointer to a buffer//
  cass::CASSEvent *cassevent;
  //go through all files in the list//
  for (std::vector<std::string>::iterator filelistiterator = filelist.begin();
       filelistiterator != filelist.end();
       ++filelistiterator)
  {
    //open the file//
    std::ifstream xtcfile;
    xtcfile.open(filelistiterator->c_str(), std::ios::binary | std::ios::in);
    //if there was such a file then we want to load it//
    if (xtcfile.is_open())
    {
      std::cout <<"processing file \""<<filelistiterator->c_str()<<"\""<<std::endl;
      //read until we are finished with the file//
      while(!xtcfile.eof() && !_quit)
      {
        //retrieve a new element from the ringbuffer//
        _ringbuffer.nextToFill(cassevent);
        //read the datagram from the file in the ringbuffer//
        Pds::Dgram& dg = *reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
        xtcfile.read(cassevent->datagrambuffer(),sizeof(dg));
        xtcfile.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
        //tell the buffer that we are done//
        _ringbuffer.doneFilling(cassevent);
      }
      //done reading.. close file//
      xtcfile.close();
    }
    else
      std::cout <<"file \""<<filelistiterator->c_str()<<"\" could not be opened"<<std::endl;

  }
  if (!_quit)
  std::cout << "done with all files"<<std::endl;
}

void cass::FileInput::end()
{
  std::cout << "input got signal that it should close"<<std::endl;
  _quit=true;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

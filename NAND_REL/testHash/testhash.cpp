// DelRM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"




//---------------------------------------------------------------------- 
//hashlib++ includes
#include <hashlibpp.h>

//----------------------------------------------------------------------	
//misc. includes
//#include <iostream>
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------	
//prototypes
void printUsage (void);

//----------------------------------------------------------------------	

int _tmain ( int argc, char **argv)
{
	bool bCreateFromText = false;
	bool bCreateFromFile = false;
	bool bTestFirst = false;
	std::string strInput = "";
	std::string strHashType = "";

	//just parse the commandline
	for( int i = 1; i < argc ; i++ )
	{
		if( strcmp(argv[i],"-s") == 0 )
		{
			bCreateFromText = true;	
			if( i != argc-1 )
			{
				strInput = argv[i+1];
			}
		}
		else if( strcmp(argv[i],"-f") == 0 )
		{
			bCreateFromFile = true;	
			if( i != argc-1 )
			{
				strInput = argv[i+1];
			}
		}
		else if( strcmp(argv[i],"-h") == 0)
		{
			if( i != argc-1 )
			{
				strHashType = argv[i+1];
			}
		}
		else if( strcmp(argv[i],"-t") == 0)
		{
			bTestFirst = true;
		}
	}

	// just print out some help if no options have been submited
	if( ((bCreateFromText == false) && (bCreateFromFile == false))  ||
	    (strHashType == "") )
	{
		printUsage();
		return -1;
	}

        //--------------------------------------------------------------
	
	//Create the wrapper using a the factory
	wrapperfactory factory;
	hashwrapper *hw = factory.create(strHashType);
	if( hw != NULL)
	{
		try
		{
			if(bTestFirst)
			{
				//Tests the implemtation. If the test fails on your 
				//system this will throw a hlException
				hw->test();
			}
			if( bCreateFromText )
			{
				//Creates a hash based on the given string
				printf("%s\n", hw->getHashFromString(strInput));
				//std::cout << hw->getHashFromString(strInput) << std::endl;
			}
			else if( bCreateFromFile )
			{
				//Create a hash based on the given file
				//This could throw a hlException if the given 
				//file does not exist
				printf("%s\n", hw->getHashFromFile(strInput));
				// std::cout << hw->getHashFromFile(strInput) << std::endl;
			}
		}
		catch(hlException &e)
		{
			printf("\n Error (%d): %s\n", e.error_number, e.error_message());
#if 0
			std::cerr << std::endl;
			std::cerr << "Error (" 
				  << e.error_number()
				  << "): " 
				  << e.error_message()
				  << std::endl;
#endif
			delete hw;
			return -2;
		}
	}
	else
	{
		printf("\nError: Unknown hashtype \"%s\"\n", strHashType);
#if 0
		std::cerr << std::endl;
		std::cerr << "Error: Unknown hashtype \""
			  << strHashType  
			  << "\"."
			  << std::endl;
#endif
		printUsage();
		return -1;
	}

	delete hw;
	return 0;
}

//----------------------------------------------------------------------	

void printUsage (void)
{
	printf("\n");
	printf("hashlib++ example program.\n");
	printf("This is a short example program which shows how to use hashlib++.\n");
	printf("It produces hash-values for a given string or file.\n");
	printf("\n");
	printf("usage: \n");
	printf("[-t] [-s STRING | -f FILENAME] [-h HASHTYPE]\n");
	printf("\n");
	printf("-t\t Tests the implementation on your system before.\n");
	printf("-s\t Creates a hash from a given string.\n");
	printf("-f\t Creates a hash from a given file.\n");
	printf("-h\t Hashfunction to use: md5,sha1,sha256,sha384,sha512.\n");
	printf("\n");
 	printf("hashlib++ - a simple hash library for C++\n");
	printf("Copyright (c) 2007-2011 Benjamin Grüdelbach\n");
	printf("\n");

#if 0
	std::cerr << std::endl;
	std::cerr << "hashlib++ example program." << std::endl;
	std::cerr << "This is a short example program which shows how to use hashlib++." << std::endl;
	std::cerr << "It produces hash-values for a given string or file." <<std::endl;
	std::cerr << std::endl;
	std::cerr << "usage: " << std::endl;
	std::cerr << "[-t] [-s STRING | -f FILENAME] [-h HASHTYPE]" << std::endl;
	std::cerr << std::endl;
	std::cerr << "-t\t Tests the implementation on your system before." << std::endl;
	std::cerr << "-s\t Creates a hash from a given string." << std::endl;
	std::cerr << "-f\t Creates a hash from a given file." << std::endl;
	std::cerr << "-h\t Hashfunction to use: md5,sha1,sha256,sha384,sha512." << std::endl;
	std::cerr << std::endl;
 	std::cerr << "hashlib++ - a simple hash library for C++" <<std::endl;
	std::cerr << "Copyright (c) 2007-2011 Benjamin Grüdelbach" <<std::endl;
	std::cerr << std::endl;
#endif
}

//----------------------------------------------------------------------	
//EOF

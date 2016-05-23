
#include <cstring>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>


#include "CJCli.h"
#include "CJTrace.h"
#include "CJConsole.h"

#include "Utils.h"
//#include "MemManager.h"
//#include "Synapse.h"
//#include "Cell.h"
#include "vRegion.h"
#include "vInputSpace.h"
#include "vClassifier.h"
#include "Pattern.h"
#include "vNetworkManager.h"

#undef CJTRACE_MODULE_ID
#define CJTRACE_MODULE_ID eTraceId_NetworkManager

// Map CLI commands to associated functions for the default command set
CJCLI_CREATE_FCNMAPPER(vCliHtmNetworkCommandSet)
CJCLI_MAP_CMD_TO_FCN(vCliHtmNetworkCommandSet, vhtm, CliCommand_htmSummary)

// Define CLI commands for the default command set
CJCLI_COMMAND_DEFINTION_START(vCliHtmNetworkCommandSet)
CJCLI_COMMAND_DESCRIPTOR(vhtm, eCliAccess_Guest, "Displays a summary of the HTM network")
CJCLI_COMMAND_DEFINTION_END


//extern MemManager mem_manager;

vNetworkManager::vNetworkManager(void)
{
   CJTRACE_SET_TRACEID_STRING(eTraceId_NetworkManager, "vHTM")
   CJTRACE_REGISTER_CLI_COMMAND_OBJ(vCliHtmNetworkCommandSet)
   CJCLI_CMDSET_OBJECT(vCliHtmNetworkCommandSet).Initialize(this);

	filename = "";
	time = 0;
	networkLoaded = false;

   lastNetworkFilename = "lastOpenNetwork.txt";

	// Delete log file if it exists.
	QFile::remove("log.txt");
}

vNetworkManager::~vNetworkManager(void)
{
	// Clear the network.
	ClearNetwork();
}

void vNetworkManager::ClearNetwork()
{
	vInputSpace *inputSpace;
	vRegion *region;

	// First clear all data from the network.
//	ClearData();

	// Delete all InputSpaces.
	while (inputSpaces.size() > 0)
	{
		inputSpace = inputSpaces.back();
		inputSpaces.pop_back();
		delete inputSpace;
	}

	// Delete all Regions.
	while (regions.size() > 0)
	{
		region = regions.back();
		regions.pop_back();
		delete region;
	}

	// Initialize members.
	filename = "";
	time = 0;
	networkLoaded = false;

	// Initialize random number generator, to have reproducibe results.
	srand(4242);
}

bool vNetworkManager::LoadLastNetwork(QString &_error_msg)
{
   QString previousFilename;

   QFile* file = new QFile(lastNetworkFilename);
   if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
   {
      _error_msg = "No last network to open in " + lastNetworkFilename;
      CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
      return false;
   }
   previousFilename = file->read(256);
   file->close();

   return LoadNetwork(previousFilename, _error_msg);
}

bool vNetworkManager::LoadNetwork(QString &_filename, QString &_error_msg)
{
	vRegion *newRegion;
	vInputSpace *newInputSpace;
	vClassifier *newClassifier;
	//bool result, proximalSynapseParamsFound = false, distalSynapseParamsFound = false;
	//SynapseParameters defaultProximalSynapseParams, defaultDistalSynapseParams;

   // Load network
   QFile* file = new QFile(_filename);

   CJTRACE(TRACE_HIGH_LEVEL, "Loading Network from file: %s", _filename);

   // If the file failed to open, display message.
   if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
   {
      _error_msg = "Couldn't open " + _filename;
      CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
      return false;
   }

   // Load the XML stream from the file.
   QXmlStreamReader xml(file);

	// Clear the network.
	ClearNetwork();
	
   while (!xml.atEnd() && !xml.hasError())
	{
		// Read next element.
      QXmlStreamReader::TokenType token = xml.readNext();

		// If token is just StartDocument, we'll go to next.
		if (token == QXmlStreamReader::StartDocument) {
				continue;
		}

		// If token is StartElement, we'll see if we can read it.
		if (token == QXmlStreamReader::StartElement) 
		{
			// If this is a ProximalSynapseParams element, read in the proximal synapse parameter information.
        /* if (xml.name() == "ProximalSynapseParams")
			{
				// Attempt to parse and store the proximal synapse parameters.
				result = ParseSynapseParams(xml, defaultProximalSynapseParams, _error_msg);

				if (result == false) 
				{
					ClearNetwork();
               CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
					return false;
				}

				proximalSynapseParamsFound = true;
			}

			// If this is a DistalSynapseParams element, read in the distal synapse parameter information.
         if (xml.name() == "DistalSynapseParams")
			{
				// Attempt to parse and store the distal synapse parameters.
				result = ParseSynapseParams(xml, defaultDistalSynapseParams, _error_msg);

				if (result == false) 
				{
					ClearNetwork();
               CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
					return false;
				}

				distalSynapseParamsFound = true;
			}*/

			// If this is a Region element, read in the Region's information.
         if (xml.name() == "Region")
			{
				// Attempt to parse and create the new Region.
            newRegion = ParseRegion(xml, _error_msg);

				if (newRegion == NULL) 
				{
					ClearNetwork();
               CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
					return false;
				}

				// Record the Region's index.
				newRegion->SetIndex((int)(regions.size()));

				// Add the new Region to the list of Regions.
				regions.push_back(newRegion);
			}

			// If this is a InputSpace element, read in the InputSpace's information.
         else if (xml.name() == "InputSpace")
			{
				// Attempt to parse and create the new InputSpace.
            newInputSpace = ParseInputSpace(xml, _error_msg);

				if (newInputSpace == NULL) 
				{
					ClearNetwork();
               CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
					return false;
				}

				// Record the InputSpace's index.
				newInputSpace->SetIndex((int)(inputSpaces.size()));

				// Add the new InputSpace to the list of InputSpaces.
				inputSpaces.push_back(newInputSpace);
			}

			// If this is a Classifier element, read in the Classifier's information.
         else if (xml.name() == "Classifier")
			{
				// Attempt to parse and create the new Classifier.
				newClassifier = ParseClassifier(xml, _error_msg);

				if (newClassifier == NULL) 
				{
					ClearNetwork();
               CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
					return false;
				}

				// Record the InputSpace's index.
				newClassifier->SetIndex((int)(classifiers.size()));

				// Add the new Classifier to the list of Classifiers.
				classifiers.push_back(newClassifier);
			}
		}
	}

	// Error handling.
   if (xml.hasError())
	{
      _error_msg = xml.errorString();
		ClearNetwork();
      CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
		return false;
	}

	// Require that the ProximalSynapseParams section has been found.
	/*if (proximalSynapseParamsFound == false)
	{
		_error_msg = "ProximalSynapseParams not found in file.";
		ClearNetwork();
      CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
		return false;
	}

	// Require that the DistalSynapseParams section has been found.
	if (distalSynapseParamsFound == false)
	{
		_error_msg = "DistalSynapseParams not found in file.";
		ClearNetwork();
      CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
		return false;
	}*/

	// Connect each Region with its input DataSpaces (InputSpaces or Regions).
	for (std::vector<vRegion*>::const_iterator region_iter = regions.begin(), end = regions.end(); region_iter != end; ++region_iter) 
	{
		for (std::vector<QString>::const_iterator input_id_iter = (*region_iter)->dConfiguredInputIDs.begin(), end = (*region_iter)->dConfiguredInputIDs.end(); input_id_iter != end; ++input_id_iter)
		{
			// Get a pointer to the DataSpace with this Region's current input's ID.
         vDataSpace *pDataSpace = GetDataSpace(*input_id_iter);

			if (pDataSpace == NULL)
			{
				_error_msg = "Region " + (*region_iter)->id + "'s input " + *input_id_iter + " is not a known Region or InputSpace.";
				ClearNetwork();
            CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
				return false;
			}

			// Add this DataSpace to this Region's list of inputs.
         (*region_iter)->AttachInputDataSpace(pDataSpace);

		}


      // Create the regions now that everything is connected
      if ((*region_iter)->CreateRegion() == false)
      {
         _error_msg = "Failed to create region (id=" + (*region_iter)->GetID() + ") -- memory issue?).";
         return false;
      }
	}

	// Connect each Classifier with its InputSpace and Region.
	for (std::vector<vClassifier*>::const_iterator classifier_iter = classifiers.begin(), end = classifiers.end(); classifier_iter != end; ++classifier_iter) 
	{
		// Get a pointer to the InputSpace with this Classifier's inputspaceID.
      vInputSpace *pInputSpace = GetInputSpace((*classifier_iter)->dInputspaceID);
      (*classifier_iter)->SetInputSpace(pInputSpace);

		// Get a pointer to the Region with this Classifier's regionID.
      vRegion* pRegion = GetRegion((*classifier_iter)->dRegionID);
      (*classifier_iter)->SetRegion(pRegion);

		if ((pInputSpace == NULL) || (pRegion == NULL))
		{
			_error_msg = "Classifier " + (*classifier_iter)->id + " must have both a valid region and inputspace.";
         CJTRACE(TRACE_HIGH_LEVEL, "ERROR: Load network failed.  %s", _error_msg);
			return false;
		}
	}

	// Removes any device() or data from the XML reader and resets its internal state to the initial state.
	xml.clear();



	// Record filename.
	filename = _filename;
   QFile* lastfile = new QFile(lastNetworkFilename);
   if (!lastfile->open(QIODevice::WriteOnly | QIODevice::Text))
   {
      CJTRACE(TRACE_HIGH_LEVEL, "WARN: Failed to open last network filename. (%s)", lastNetworkFilename);
   }
   if (lastfile->write(filename.toUtf8()) == 0)
   {
      CJTRACE(TRACE_HIGH_LEVEL, "WARN: Failed to write last network filename. (%s)", lastNetworkFilename);
   }
   lastfile->close();

	// Record that the network has been loaded.
	networkLoaded = true;

	// Success.
   CJTRACE(TRACE_HIGH_LEVEL, "Load network completed successfully.");
	return true;
}

vRegion *vNetworkManager::ParseRegion(QXmlStreamReader &_xml, QString &_error_msg)
{
   bool errorFlag = false;
   QString id;
   eRegionSize regionSize = eRegionSize_INVALID;
   vRegion *pNewRegion;

   // Look for ID attribute
   QXmlStreamAttributes attributes = _xml.attributes();
   if (attributes.hasAttribute("id")) {
      id = attributes.value("id").toString();
      pNewRegion = new vRegion(id);
   }
   else
   {
      _error_msg = "Each region must have an ID string";
      return NULL;
   }

   vRegion::SpatialPoolerConfig spConfig = pNewRegion->GetSpatialPoolerConfig();
   vRegion::TemporalPoolerConfig tpConfig = pNewRegion->GetTemporalPoolerConfig();


   // Advance to the next element.
   _xml.readNext();

   // Loop through this Region's elements (this allows the order to vary).
   // Continue the loop until we hit an EndElement named Region.
   for (;;)
   {
      // Get the current token name
      QString tokenName = _xml.name().toString().toLower();

      if ((_xml.tokenType() == QXmlStreamReader::EndElement) && (tokenName == "region")) {
         break;
      }

      if (_xml.atEnd())
      {
         _error_msg = "Unexpected end of document.";
         errorFlag = true;
         break;
      }

      if (_xml.tokenType() == QXmlStreamReader::StartElement)
      {
         // Region Size
         if (tokenName == "regionsize")
         {
            _xml.readNext();
            if (_xml.tokenType() == QXmlStreamReader::Characters) {
               regionSize = (eRegionSize)_xml.text().toString().toInt();
               
            }
         }


         // Spacial Pooler Boost Config
         else if (tokenName == "spboost")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("max")) {
               spConfig.boostMax = attributes.value("max").toString().toInt();
            }

            if (attributes.hasAttribute("min")) {
               spConfig.boostMin = attributes.value("min").toString().toInt();
            }

            if (attributes.hasAttribute("increment")) {
               spConfig.boostIncrement = attributes.value("increment").toString().toInt();
            }
         }

         // Spacial Pooler Inhibition Config 
         else if (tokenName == "spinhibition")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("type"))
            {
               /*if (attributes.value("type").toString().toLower() == "automatic")
               {
                  _error_msg = "Region " + id + " Automatic inhibition type not supported yet (TODO).";
                  inhibitionType = INHIBITION_TYPE_AUTOMATIC;
               }
               else if (attributes.value("type").toString().toLower() == "distance")
               {
                  inhibitionType = INHIBITION_TYPE_RADIUS;
               }
               else
               {
                  _error_msg = "Region " + id + " has unknown inhibition type " + attributes.value("type").toString() + ".";
                  return NULL;
               }*/
            }

            if (attributes.hasAttribute("distance")) {
               spConfig.inhibitionDistance = attributes.value("distance").toString().toInt();
            }
         }

         // Spacial Pooler Poor Overlap Config - Behavior when there is poor overlap of connected synaspes
         else if (tokenName == "spnooverlap")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("minthreshold")) {
               spConfig.noOverlapThresholdMin = attributes.value("minthreshold").toString().toInt();
            }
            if (attributes.hasAttribute("permanenceincrement")) {
               spConfig.noOverlapPermanenceIncrement = attributes.value("permanenceincrement").toString().toInt();
            }
         }



         /*
         // SpatialLearningPeriod
         else if (tokenName == "spatiallearningperiod")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("start")) {
               spatialLearningStartTime = attributes.value("start").toString().toInt();
            }

            if (attributes.hasAttribute("end")) {
               spatialLearningEndTime = attributes.value("end").toString().toInt();
            }
         }

         // TemporalLearningPeriod
         else if (tokenName == "temporallearningperiod")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("start")) {
               temporalLearningStartTime = attributes.value("start").toString().toInt();
            }

            if (attributes.hasAttribute("end")) {
               temporalLearningEndTime = attributes.value("end").toString().toInt();
            }
         }
         */


         // Input
         else if (tokenName == "input")
         {
            attributes = _xml.attributes();

            if (attributes.hasAttribute("id"))
            {
               pNewRegion->dConfiguredInputIDs.push_back(attributes.value("id").toString());
            }
         }

         else
         {
            CJTRACE(TRACE_HIGH_LEVEL, "WARN: Got an unknown XML token in vRegion config (token=%s)", tokenName.toStdString().c_str());
         }

      }

      // Advance to next token...
      _xml.readNext();
   }

   if (id.length() == 0)
   {
      _error_msg = "Region missing ID.";
      errorFlag = true;
   }

   if (pNewRegion->SetRegionSize(regionSize) == false)
   {
      _error_msg = "Region " + id + " assigned an unknown or invalid region size value";
      errorFlag = true;
   }

   
/*
   if (spatialLearningStartTime < -1)
   {
      _error_msg = "Region " + id + " has invalid SpatialLearningPeriod start time " + QString(spatialLearningStartTime) + ".";
       errorFlag = true;
   }

   if (spatialLearningEndTime < -1)
   {
      _error_msg = "Region " + id + " has invalid SpatialLearningPeriod end time " + QString(spatialLearningEndTime) + ".";
       errorFlag = true;
   }

   if (temporalLearningStartTime < -1)
   {
      _error_msg = "Region " + id + " has invalid TemporalLearningPeriod start time " + QString(temporalLearningStartTime) + ".";
       errorFlag = true;
   }

   if (temporalLearningEndTime < -1)
   {
      _error_msg = "Region " + id + " has invalid TemporalLearningPeriod end time " + QString(temporalLearningEndTime) + ".";
       errorFlag = true;
   }
   */

   if (pNewRegion->dConfiguredInputIDs.size() == 0)
   {
      _error_msg = "Region has no Inputs. ID=" + id;
      errorFlag = true;
   }

   if (errorFlag)
   {
      delete pNewRegion;
      return NULL;
   }
	return pNewRegion;
}


vInputSpace *vNetworkManager::ParseInputSpace(QXmlStreamReader &_xml, QString &_error_msg)
{
	QString id;
   vInputSpace* pInputSpace = NULL;

   bool inputSpaceParamsValid = true;
   bool encoderParamsValid = true;
   bool patternParamsValid = true;

	// Look for ID attribute
	QXmlStreamAttributes attributes = _xml.attributes();
	if (attributes.hasAttribute("id")) {
		id = attributes.value("id").toString();

      // Create an empty Input Space
      pInputSpace = new vInputSpace(id);
	}

	// Advance to the next element.
	_xml.readNext();
  
	// Loop through this InputSpace's elements (this allows the order to vary).
	// Continue the loop until we hit an EndElement named InputSpace.
	for (;;)
	{
		// Get the current token name
		QString tokenName = _xml.name().toString().toLower();

		if ((_xml.tokenType() == QXmlStreamReader::EndElement) && (tokenName == "inputspace")) {
			break;
		}

		if (_xml.atEnd()) 
		{
			_error_msg = "Unexpected end of document.";
			return NULL;
		}

		if (_xml.tokenType() == QXmlStreamReader::StartElement) 
		{
			// SizeX
			if (tokenName == "sizex") 
			{
				_xml.readNext();
				if(_xml.tokenType() == QXmlStreamReader::Characters) {
               int sizeX = _xml.text().toString().toInt();
               pInputSpace->SetSize(vPosition(sizeX, pInputSpace->GetSizeY(), pInputSpace->GetSizeZ()));
				}
			}

			// SizeY
			else if (tokenName == "sizey") 
			{
				_xml.readNext();
				if(_xml.tokenType() == QXmlStreamReader::Characters) {
               int sizeY = _xml.text().toString().toInt();
               pInputSpace->SetSize(vPosition(pInputSpace->GetSizeX(), sizeY, pInputSpace->GetSizeZ()));
				}
			}

         // SizeZ
         else if (tokenName == "sizez")
         {
            _xml.readNext();
            if (_xml.tokenType() == QXmlStreamReader::Characters) {
               int sizeZ = _xml.text().toString().toInt();
               pInputSpace->SetSize(vPosition(pInputSpace->GetSizeX(), pInputSpace->GetSizeY(), sizeZ));
            }
         }

         // Encoder
         else if (tokenName == "encoder")
         {
            encoderParamsValid = ParseEncoder(_xml, _error_msg, pInputSpace);
         }

			// Pattern
			else if (tokenName == "pattern") 
			{
            patternParamsValid = ParsePattern(_xml, _error_msg, pInputSpace);
			}
		}
		
		// Advance to next token...
		_xml.readNext();
	}

	if (id.length() == 0)
	{
      inputSpaceParamsValid = false;
		_error_msg = "InputSpace missing ID.";
		return NULL;
	}

   if ((pInputSpace->GetSizeX() <= 0) || (pInputSpace->GetSizeX() > INPUTSPACE_MAX_SIZE))
	{
      inputSpaceParamsValid = false;
		_error_msg = "InputSpace " + id + " has invalid SizeX " + QString(pInputSpace->GetSizeX()) + ".";
	}

	if ((pInputSpace->GetSizeY() <= 0) || (pInputSpace->GetSizeY() > INPUTSPACE_MAX_SIZE))
	{
      inputSpaceParamsValid = false;
		_error_msg = "InputSpace " + id + " has invalid SizeY " + QString(pInputSpace->GetSizeY()) + ".";
	}

   if ((pInputSpace->GetSizeZ() <= 0) || (pInputSpace->GetSizeZ() > INPUTSPACE_MAX_SIZE))
   {
      inputSpaceParamsValid = false;
      _error_msg = "InputSpace " + id + " has invalid SizeZ " + QString(pInputSpace->GetSizeZ()) + ".";
   }
   
	if ((inputSpaceParamsValid == false) || (encoderParamsValid == false) || (patternParamsValid == false))
	{
		return NULL;
	}
   pInputSpace->CreateBuffer();
   return pInputSpace;
}


bool vNetworkManager::ParseEncoder(QXmlStreamReader &_xml, QString &_error_msg, vInputSpace *pInputSpace)
{
   int active = 0;
   int range = 0;
   float min_value = 0;
   float max_value = 0;

   // Look for attributes
   QXmlStreamAttributes attributes = _xml.attributes();

   if (attributes.hasAttribute("type"))
   {
      QString typeString = attributes.value("type").toString().toLower();
      if (typeString != "value")
         return false;
   }

   if (attributes.hasAttribute("active"))
   {
      active = attributes.value("active").toString().toInt();
   }
   else return false;

   if (attributes.hasAttribute("range"))
   {
      range = attributes.value("range").toString().toInt();
   }
   else return false;

   if (attributes.hasAttribute("min_value"))
   {
      min_value = attributes.value("min_value").toString().toFloat();
   }
   else return false;

   if (attributes.hasAttribute("max_value"))
   {
      max_value = attributes.value("max_value").toString().toFloat();
   }
   else return false;

   pInputSpace->SetupEncoder( active, range, min_value, max_value);
   return true;
}

bool vNetworkManager::ParsePattern(QXmlStreamReader &_xml, QString &_error_msg, vInputSpace *pInputSpace)
{
   Pattern* pPattern;

	// Look for attributes
	QXmlStreamAttributes attributes = _xml.attributes();
				
   if (attributes.hasAttribute("type"))
   {
      QString typeString = attributes.value("type").toString().toLower();
      ePatternType patternType = PATTERN_NONE;

      /*if (typeString == "stripe") {
         patternType = PATTERN_STRIPE;
      } else if (typeString == "bouncingstripe") {
         patternType = PATTERN_BOUNCING_STRIPE;
      } else if (typeString == "bar") {
         patternType = PATTERN_BAR;
      } else if (typeString == "bouncingbar") {
         patternType = PATTERN_BOUNCING_BAR;
      } else if (typeString == "text") {
         patternType = PATTERN_TEXT;
      } else if (typeString == "bitmap") {
         patternType = PATTERN_BITMAP;
      } else if (typeString == "image") {
         patternType = PATTERN_IMAGE;
      } else*/ if (typeString == "encodedsequence") {
         patternType = PATTERN_ENCODED_SEQUENCE;
      }
      else if (typeString == "encodedfile") {
         patternType = PATTERN_ENCODED_FILE;
      }
      else if (typeString == "encodedrandom") {
         patternType = PATTERN_ENCODED_RANDOM;
      }

      pPattern = new Pattern(pInputSpace, patternType);
   }
   else
   {
      _error_msg = "No pattern type attribute found";
      return false;
   }

   if (attributes.hasAttribute("values"))
   {
      char stringBuffer[STRING_BUFFER_LEN], lineBuffer[LINE_BUFFER_LEN];
      strncpy(lineBuffer, attributes.value("values").toLatin1(), LINE_BUFFER_LEN);
      char* linePos = lineBuffer;
      while (ReadItem(linePos, ',', stringBuffer, STRING_BUFFER_LEN))
      {
         pPattern->InitSequence_AddValue(atof(stringBuffer));
      }
   }

	if (attributes.hasAttribute("source")) 
	{
		// Get the source file's filename.
		QString sourceFilename = attributes.value("source").toString();

		// Add the network file's path to the given source file's filename.
		QFile *networkFile = (QFile*)(_xml.device());
		QFileInfo fileInfo(*networkFile);
		sourceFilename = fileInfo.path() + QDir::separator() + sourceFilename;

		// Load source file
		QFile* file = new QFile(sourceFilename);
    
		// If the file failed to open, return error message.
		if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) 
		{
			_error_msg = "Could not open file " + sourceFilename + ".";
         delete file;
		}
      else
      {
         QTextStream in(file);
         while (!in.atEnd())
         {
            char stringBuffer[STRING_BUFFER_LEN], lineBuffer[LINE_BUFFER_LEN];
            QString line = in.readLine();
            strncpy(lineBuffer, line.toStdString().data(), LINE_BUFFER_LEN);
            char* linePos = lineBuffer;

            // Read the next item on the line into the stringBuffer.
            while (ReadItem(linePos, ',', stringBuffer, STRING_BUFFER_LEN))
            {
               pPattern->InitSequence_AddValue(atof(stringBuffer));
            }
            if (stringBuffer[0] != 0)
            {
               pPattern->InitSequence_AddValue(atof(stringBuffer));
            }
         }
      }					
		// Delete the source file object.
		delete file;
	}

	/*if (attributes.hasAttribute("start")) 
	{
		startTime = attributes.value("start").toString().toInt();
	}

	if (attributes.hasAttribute("end")) 
	{
		endTime = attributes.value("end").toString().toInt();
	}

   if (attributes.hasAttribute("string")) {
      patternString = attributes.value("string").toString();
   }


	// Advance to the next element.
	_xml.readNext();
  
	// Loop through this Pattern's elements (this allows the order to vary).
	// Continue the loop until we hit an EndElement named Pattern.
	while (!((_xml.tokenType() == QXmlStreamReader::EndElement) && (_xml.name() == "Pattern"))) 
	{
		if (_xml.tokenType() == QXmlStreamReader::StartElement) 
		{
			// BitMap
			if (_xml.name() == "BitMap") 
			{
				_xml.readNext();
				if(_xml.tokenType() == QXmlStreamReader::Characters) 
				{
					QString bitmap_string = _xml.text().toString();

					// Remove all whitespace from the string.
					bitmap_string = bitmap_string.simplified();
					bitmap_string.replace(" ", "");

					// Create an int array for the current bitmap.
					int *bitmap_array = new int[_width * _height];
					memset(bitmap_array, 0, _width * _height * sizeof(int));
					bitmaps.push_back(bitmap_array);

					for (int i = 0; i < Min(_width * _height, bitmap_string.length()); i++)
					{
						if (bitmap_string[i] != '0') {
							bitmap_array[i] = 1;
						}
					}
				}
			}
		}

		// Advance to next token...
		_xml.readNext();
	}*/

   pInputSpace->AddPattern(pPattern);
   return true;
}

vClassifier *vNetworkManager::ParseClassifier(QXmlStreamReader &_xml, QString &_error_msg)
{
	QString inputspaceID(""), regionID(""), id("");

	// Look for attributes
	QXmlStreamAttributes attributes = _xml.attributes();

	if (attributes.hasAttribute("id")) 
	{
		id = attributes.value("id").toString();
	}

	if (attributes.hasAttribute("inputspace")) 
	{
		inputspaceID = attributes.value("inputspace").toString();
	}

	if (attributes.hasAttribute("region")) 
	{
		regionID = attributes.value("region").toString();
	}

	if (id == "") 
	{
		_error_msg = QString("Classifier must have 'id' attribute.");
		return NULL;
	}

	if (regionID == "") 
	{
		_error_msg = QString("Classifier '" + id + "' must have 'region' ID.");
		return NULL;
	}

	if (inputspaceID == "") 
	{
		_error_msg = QString("Classifier '" + id + "' must have 'inputspace' ID.");
		return NULL;
	}

	// Advance to the next element.
	_xml.readNext();
  
	// Loop through this Pattern's elements (this allows the order to vary).
	// Continue the loop until we hit an EndElement named Pattern.
	while (!((_xml.tokenType() == QXmlStreamReader::EndElement) && (_xml.name() == "Classifier"))) 
	{
		// Advance to next token...
		_xml.readNext();
	}	

	return new vClassifier(id, regionID, inputspaceID);
}

bool vNetworkManager::ReadItem(char* &_linePos, char _separator, char *_stringBuffer, int _stringBufferLen)
{
	char *separator_pos = strchr(_linePos, _separator);

	if (separator_pos == NULL)
	{
      strncpy(_stringBuffer, _linePos,_stringBufferLen - 2);
		return FALSE;
	}

	// Copy the isolated item into the _stringBuffer.
	strncpy(_stringBuffer, _linePos, Min(separator_pos - _linePos, _stringBufferLen - 2));
	_stringBuffer[Min(separator_pos - _linePos, _stringBufferLen - 1)] = '\0';

	// Advance the _linePos past the isolated item and the separator.
	_linePos = separator_pos + 1;
   return TRUE;
}


/*
void NetworkManager::ClearData()
{
	Region *region;
	Column *column;
	Cell *cell;

	// Iterate through all regions...
	for (int regionIndex = 0; regionIndex < regions.size(); regionIndex++)
	{
		region = regions[regionIndex];

		// Iterate through each column.
		for (int colIndex = 0; colIndex < (region->GetSizeX() * region->GetSizeY()); colIndex++)
		{
			column = region->Columns[colIndex];

			// Clear the column's proximal segment.
			ClearData_ProximalSegment(column->ProximalSegment);

			// Iterate through all cells...
			for (int cellIndex = 0; cellIndex < region->GetCellsPerCol(); cellIndex++)
			{
				cell = column->GetCellByIndex(cellIndex);

				// Clear all of this cell's distal segments...
				while (cell->Segments.Count() > 0)
				{
					// Clear the column's distal segment.
					ClearData_DistalSegment((Segment*)(cell->Segments.GetFirst()));

					// Remove and release the current distal segment.
					mem_manager.ReleaseObject((Segment*)(cell->Segments.GetFirst()));
					cell->Segments.RemoveFirst();
				}				
			}
		}
	}
}


void NetworkManager::ClearData_ProximalSegment(Segment *_segment)
{
	while (_segment->Synapses.Count() > 0)
	{
		// Remove and release the current proximal synapse.
		mem_manager.ReleaseObject((ProximalSynapse*)(_segment->Synapses.GetFirst()));
		_segment->Synapses.RemoveFirst();
	}
}



void NetworkManager::ClearData_DistalSegment(Segment *_segment)
{
	while (_segment->Synapses.Count() > 0)
	{
		// Remove and release the current distal synapse.
		mem_manager.ReleaseObject((DistalSynapse*)(_segment->Synapses.GetFirst()));
		_segment->Synapses.RemoveFirst();
	}
}

bool NetworkManager::LoadData(QString &_filename, QFile *_file, QString &_error_msg)
{
	int numRegions, width, height, cellsPerCol, numDistalSegments;
	Region *region;
	Column *column;
	Cell *cell;
	Segment *segment;
	bool result;

	// Clear the existing data.
	ClearData();

	QDataStream stream(_file);

	// Read the number of regions.
	stream >> numRegions;

	if (numRegions < 0) 
	{
		_error_msg = QString("Invalid number of Regions.");
		return false;
	}

	// Iterate through all regions...
	for (int regionIndex = 0; regionIndex < Min(numRegions, regions.size()); regionIndex++)
	{
		// Get a pointer to the current Region.
		region = regions[regionIndex];

		stream >> width;
		stream >> height;
		stream >> cellsPerCol;

		if ((width != region->GetSizeX()) || (height != region->GetSizeY()) || (cellsPerCol != region->GetCellsPerCol())) 
		{
			_error_msg = QString("Dimensions of Region do not match network.");
			ClearData();
			return false;
		}

		// Iterate through each column.
		for (int colIndex = 0; colIndex < (region->GetSizeX() * region->GetSizeY()); colIndex++)
		{
			column = region->Columns[colIndex];
					
			// Read the column's data.
			stream >> column->_overlapDutyCycle;
			stream >> column->ActiveDutyCycle;
			stream >> column->FastActiveDutyCycle;
			stream >> column->MinBoost;
			stream >> column->MaxBoost;
			stream >> column->Boost;

			// Read the column's proximal segment data.
			result = LoadData_ProximalSegment(stream, region, column->ProximalSegment, _error_msg);

			if (result == false) 
			{
				ClearData();
				return false;
			}
			
			for (int cellIndex = 0; cellIndex < region->GetCellsPerCol(); cellIndex++)
			{
				cell = column->GetCellByIndex(cellIndex);

				// Read the number of distal segments.
				stream >> numDistalSegments;

				// Iterate through each of this cell's distal segments...
				for (int segIndex = 0; segIndex < numDistalSegments; segIndex++)
				{
					// Create the new distal segment.
               segment = (Segment*)(mem_manager.GetMemObject(MOT_SEGMENT));
					cell->Segments.InsertAtEnd(segment);

					// Read the current distal segment's data.
					result = LoadData_DistalSegment(stream, region, segment, _error_msg);

					if (result == false) 
					{
						ClearData();
						return false;
					}
				}				
			}
		}
	}

	return true;
}

bool NetworkManager::LoadData_ProximalSegment(QDataStream &_stream, Region *_region, Segment *_segment, QString &_error_msg)
{
	// Read attributes of this segment.
	_stream >> _segment->_numPredictionSteps;
	_stream >> _segment->ConnectedSynapsesCount;
	_stream >> _segment->PrevConnectedSynapsesCount;
	_stream >> _segment->ActiveThreshold;

	// Record the number of synapses.
	int numSynapses;
	_stream >> numSynapses;

	ProximalSynapse *syn;
	float perm;
	DataSpaceType dataSpaceType;
	int dataSpaceIndex;
	DataSpace *dataSpace;
	for (int i = 0; i < numSynapses; i++)
	{
      syn = (ProximalSynapse*)(mem_manager.GetMemObject(MOT_PROXIMAL_SYNAPSE));
		syn->Initialize(&(_region->DistalSynapseParams));
		_segment->Synapses.InsertAtEnd(syn);

		_stream >> perm;
		syn->SetPermanence(perm);

		_stream >> dataSpaceType;
		_stream >> dataSpaceIndex;

		dataSpace = (dataSpaceType == DATASPACE_TYPE_INPUTSPACE) ? ((DataSpace*)(inputSpaces[dataSpaceIndex])) : ((DataSpace*)(regions[dataSpaceIndex]));

		if (dataSpace == NULL) 
		{
			_error_msg = QString("Proximal synapse connects to Region or InputSpace that does not exist in network.");
			return false;
		}

		// Record this Synapse's input source.
		syn->InputSource = dataSpace;

		// Read this Synapse's input coordinates.
		_stream >> syn->InputPoint.X;
		_stream >> syn->InputPoint.Y;
		_stream >> syn->InputPoint.Index;

		if ((syn->InputPoint.X < 0) || (syn->InputPoint.X >= dataSpace->GetSizeX()) ||
			  (syn->InputPoint.Y < 0) || (syn->InputPoint.Y >= dataSpace->GetSizeY()) ||
				(syn->InputPoint.Index < 0) || (syn->InputPoint.Index >= dataSpace->GetNumValues()))
		{
			_error_msg = QString("Proximal synapse connects to input that is beyond the dimensions of its input Region or InputSpace.");
			return false;
		}

		// Read this Synapse's DistanceToInput.
		_stream >> syn->DistanceToInput;
	}

	return true;
}

bool NetworkManager::LoadData_DistalSegment(QDataStream &_stream, Region *_region, Segment *_segment, QString &_error_msg)
{
	// Read attributes of this segment.
	_stream >> _segment->_numPredictionSteps;
	_stream >> _segment->ConnectedSynapsesCount;
	_stream >> _segment->PrevConnectedSynapsesCount;
	_stream >> _segment->ActiveThreshold;

	// Record the number of synapses.
	int numSynapses;
	_stream >> numSynapses;

	DistalSynapse *syn;
	float perm;
	int inputX, inputY, inputIndex;
	for (int i = 0; i < numSynapses; i++)
	{
      syn = (DistalSynapse*)(mem_manager.GetMemObject(MOT_DISTAL_SYNAPSE));
		syn->Initialize(&(_region->DistalSynapseParams));
		_segment->Synapses.InsertAtEnd(syn);

		_stream >> perm;
		syn->SetPermanence(perm);

		// Read this Synapse's input coordinates.
		_stream >> inputX;
		_stream >> inputY;
		_stream >> inputIndex;

		if ((inputX < 0) || (inputX >= _region->GetSizeX()) ||
			  (inputY < 0) || (inputY >= _region->GetSizeY()) ||
				(inputIndex < 0) || (inputIndex >= _region->GetNumValues()))
		{
			_error_msg = QString("Distal synapse connects to input that is beyond the dimensions of its Region.");
			return false;
		}

		// Store pointer to this Synapse's input source Cell.
		syn->InputSource = _region->GetColumn(inputX, inputY)->GetCellByIndex(inputIndex);
	}

	return true;
}

bool NetworkManager::SaveData(QString &_filename, QFile *_file, QString &_error_msg)
{
	Region *region;
	Column *column;
	Cell *cell;

	QDataStream stream(_file);

	// Write number of Regions
	stream << (int)(regions.size());

	// Iterate through all regions...
	for (int regionIndex = 0; regionIndex < regions.size(); regionIndex++)
	{
		region = regions[regionIndex];

		// Record this region's dimensions.
		stream << (int)(region->GetSizeX());
		stream << (int)(region->GetSizeY());
		stream << (int)(region->GetCellsPerCol());

		// Iterate through each column.
		for (int colIndex = 0; colIndex < (region->GetSizeX() * region->GetSizeY()); colIndex++)
		{
			column = region->Columns[colIndex];

			// Record the column's data.
			stream << column->GetOverlapDutyCycle();
			stream << column->GetActiveDutyCycle();
			stream << column->GetFastActiveDutyCycle();
			stream << column->GetMinBoost();
			stream << column->GetMaxBoost();
			stream << column->GetBoost();

			// Record the column's proximal segment.
			SaveData_ProximalSegment(stream, column->ProximalSegment);

			for (int cellIndex = 0; cellIndex < region->GetCellsPerCol(); cellIndex++)
			{
				cell = column->GetCellByIndex(cellIndex);

				// Record the number of distal segments.
				stream << cell->Segments.Count();

				// Iterate through each of this cell's distal segments...
				for (int segIndex = 0; segIndex < cell->Segments.Count(); segIndex++)
				{
					// Record the current distal segment.
					SaveData_DistalSegment(stream, (Segment*)(cell->Segments.GetByIndex(segIndex)));
				}				
			}
		}
	}

	return true;
}

bool NetworkManager::SaveData_ProximalSegment(QDataStream &_stream, Segment *_segment)
{
	// Record attributes of this segment.
	_stream << _segment->GetNumPredictionSteps();
	_stream << _segment->GetConnectedSynapseCount();
	_stream << _segment->GetPrevConnectedSynapseCount();
	_stream << _segment->GetActiveThreshold();

	// Record the number of synapses.
	_stream << _segment->Synapses.Count();

	FastListIter synIter(_segment->Synapses);
	ProximalSynapse *syn;
	for (syn = (ProximalSynapse*)(synIter.Reset()); syn != NULL; syn = (ProximalSynapse*)(synIter.Advance()))
	{
		_stream << syn->GetPermanence();
		_stream << syn->InputSource->GetDataSpaceType();
		_stream << syn->InputSource->GetIndex();
		_stream << syn->InputPoint.X;
		_stream << syn->InputPoint.Y;
		_stream << syn->InputPoint.Index;
		_stream << syn->DistanceToInput;

	}

	return true;
}

bool NetworkManager::SaveData_DistalSegment(QDataStream &_stream, Segment *_segment)
{
		// Record attributes of this segment.
	_stream << _segment->GetNumPredictionSteps();
	_stream << _segment->GetConnectedSynapseCount();
	_stream << _segment->GetPrevConnectedSynapseCount();
	_stream << _segment->GetActiveThreshold();

	// Record the number of synapses.
	_stream << _segment->Synapses.Count();

	FastListIter synIter(_segment->Synapses);
	DistalSynapse *syn;
	for (syn = (DistalSynapse*)(synIter.Reset()); syn != NULL; syn = (DistalSynapse*)(synIter.Advance()))
	{
		_stream << syn->GetPermanence();
		_stream << syn->GetInputSource()->GetColumn()->Position.X;
		_stream << syn->GetInputSource()->GetColumn()->Position.Y;
		_stream << syn->GetInputSource()->GetIndex();
	}

	return true;
}
*/

vDataSpace *vNetworkManager::GetDataSpace(const QString _id)
{
	vInputSpace *inputspace = GetInputSpace(_id);

	if (inputspace != NULL) {
		return inputspace;
	}

	vRegion *region = GetRegion(_id);

	return region;
}

vInputSpace *vNetworkManager::GetInputSpace(const QString _id)
{
	// Look for a match to the given _id among the InputSpaces.
	for (std::vector<vInputSpace*>::const_iterator input_iter = inputSpaces.begin(), end = inputSpaces.end(); input_iter != end; ++input_iter) 
	{
		if ((*input_iter)->GetID() == _id) {
			return (*input_iter);
		}
	}

	return NULL;
}

vRegion *vNetworkManager::GetRegion(const QString _id)
{
	// Look for a match to the given _id among the Regions.
	for (std::vector<vRegion*>::const_iterator region_iter = regions.begin(), end = regions.end(); region_iter != end; ++region_iter) 
	{
		if ((*region_iter)->GetID() == _id) {
			return (*region_iter);
		}
	}

	return NULL;
}

int vNetworkManager::Step()
{
	// Increment time.
	time++;

	// Apply any test patterns to the InputSpaces.
	for (std::vector<vInputSpace*>::const_iterator input_iter = inputSpaces.begin(), end = inputSpaces.end(); input_iter != end; ++input_iter) {
		(*input_iter)->Step(time);
	}

	// Run a time step for each Region, in the order they were defined.
	for (std::vector<vRegion*>::const_iterator region_iter = regions.begin(), end = regions.end(); region_iter != end; ++region_iter) {
		(*region_iter)->Step();
	}

   // Run the classifier on the output data for the last time step
   for (std::vector<vClassifier*>::const_iterator classifier_iter = classifiers.begin(), end = classifiers.end(); classifier_iter != end; ++classifier_iter) {
      (*classifier_iter)->Classify();
   }

   return time;
}

void vNetworkManager::WriteToLog(QString _text)
{
	QFile file("log.txt");
	if ( file.open(QIODevice::Append) )
	{
			QTextStream stream( &file );
			stream << _text << endl;
	}
}



CliReturnCode vCliHtmNetworkCommandSet::CliCommand_htmSummary(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{

return eCliReturn_Success;
}
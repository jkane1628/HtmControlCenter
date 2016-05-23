#pragma once

#include <qstring.h>
#include <vector>

const int INPUTSPACE_MAX_SIZE = 1000000;
const int INPUTSPACE_MAX_NUM_VALUES = 1000;

const int STRING_BUFFER_LEN = 100;
const int LINE_BUFFER_LEN = 10000;

class QXmlStreamReader;
class QFile;
class SDR;
class vDataSpace;
class vInputSpace;
class vRegion;
class vClassifier;


class vNetworkManager 
{
public:
	vNetworkManager(void);
	~vNetworkManager(void);

	void ClearNetwork();

	bool LoadNetwork(QString &_filename, QString &_error_msg);
   bool LoadLastNetwork(QString &_error_msg);

	vRegion *ParseRegion(QXmlStreamReader &_xml, QString &_error_msg);

	vInputSpace *ParseInputSpace(QXmlStreamReader &_xml, QString &_error_msg);
   bool ParsePattern(QXmlStreamReader &_xml, QString &_error_msg, vInputSpace *pInputSpace);
   bool ParseEncoder(QXmlStreamReader &_xml, QString &_error_msg, vInputSpace *pInputSpace);

   vClassifier *ParseClassifier(QXmlStreamReader &_xml, QString &_error_msg);


	bool ReadItem(char* &_linePos, char _separator, char *_stringBuffer, int _stringBufferLen);

   /*void ClearData();
	void ClearData_ProximalSegment(Segment *_segment);
	void ClearData_DistalSegment(Segment *_segment);

	bool LoadData(QString &_filename, QFile *_file, QString &_error_msg);
	bool LoadData_ProximalSegment(QDataStream &_stream, Region *_region, Segment *_segment, QString &_error_msg);
	bool LoadData_DistalSegment(QDataStream &_stream, Region *_region, Segment *_segment, QString &_error_msg);

	bool SaveData(QString &_filename, QFile *_file, QString &_error_msg);
	bool SaveData_ProximalSegment(QDataStream &_stream, Segment *_segment);
	bool SaveData_DistalSegment(QDataStream &_stream, Segment *_segment);
   */
   
	const QString &GetFilename() {return filename;}
	int GetTime() {return time;}
	bool IsNetworkLoaded() {return networkLoaded;}

	vDataSpace *GetDataSpace(const QString _id);
	vInputSpace *GetInputSpace(const QString _id);
	vRegion *GetRegion(const QString _id);

   int Step();

	void WriteToLog(QString _text);

	std::vector<vInputSpace*> inputSpaces;
	std::vector<vRegion*> regions;
	std::vector<vClassifier*> classifiers;

   QString lastNetworkFilename;
	QString filename;
	int time;
	bool networkLoaded;
};

#include "CJCli.h"
class vCliHtmNetworkCommandSet
{
public:
   vCliHtmNetworkCommandSet(){}
   void Initialize(vNetworkManager* pNetworkManager) { dpNetworkManager = pNetworkManager; }
   static CliCommand CommandDescriptorArray[];
   
   // CLI Command Functions
   CliReturnCode CliCommand_htmSummary(CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);

private:
   vNetworkManager* dpNetworkManager;
};


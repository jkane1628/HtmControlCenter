#pragma once
#include "DataSpace.h"
#include <QtGui/QImage>
#include <QtGui/QPainter>

enum PatternType
{
	PATTERN_NONE,
	PATTERN_STRIPE,
	PATTERN_BOUNCING_STRIPE,
	PATTERN_BAR,
	PATTERN_BOUNCING_BAR,
	PATTERN_TEXT,
	PATTERN_BITMAP,
	PATTERN_IMAGE,
   PATTERN_ENCODED_SEQUENCE,
   PATTERN_ENCODED_FILE,
   PATTERN_ENCODED_RANDOM

};

enum PatternFileFormat
{
	PATTERN_FILE_FORMAT_UNDEF,
	PATTERN_FILE_FORMAT_SPREADSHEET,
   PATTERN_FILE_FORMAT_CSV
};

enum PatternImageMotion
{
	PATTERN_IMAGE_MOTION_NONE,
	PATTERN_IMAGE_MOTION_ACROSS,
	PATTERN_IMAGE_MOTION_ACROSS2
};

class ImageInfo
{
public:
	QString label;
	int width, height, contentX, contentY, contentWidth, contentHeight;
	float *data;
};

class PatternInfo
{
public:
   PatternInfo() : type(PATTERN_NONE), startTime(-1), endTime(-1), string(""), imageMotion(PATTERN_IMAGE_MOTION_NONE), dFcnValueHistoryMaxSize(150), dFcnValueHistoryStartTime(0) {};
   PatternInfo(PatternType _type, int _startTime, int _endTime, int _seed, QString &_string, PatternImageMotion _imageMotion, std::vector<int*> &_bitmaps, std::vector<ImageInfo*> &_images, std::vector<float> &_values) : type(_type), startTime(_startTime), endTime(_endTime), seed(_seed), string(_string), imageMotion(_imageMotion), bitmaps(_bitmaps), images(_images), values(_values), dFcnValueHistoryMaxSize(150), dFcnValueHistoryStartTime(0) {};
   PatternInfo(PatternInfo &_original) { type = _original.type; startTime = _original.startTime, endTime = _original.endTime; string = _original.string; imageMotion = _original.imageMotion; dFcnValueHistoryMaxSize = _original.dFcnValueHistoryMaxSize; dFcnValueHistoryStartTime = _original.dFcnValueHistoryStartTime; }

	PatternType type;
   PatternFileFormat fileFormat;
	PatternImageMotion imageMotion;
	int startTime, endTime;
   int seed;
	int startX, startY, endX, endY;
	QString string;
	std::vector<int*> bitmaps;
	std::vector<ImageInfo*> images;
   std::vector<float> values;
	int *buffer;
   
   int dFcnValueHistoryMaxSize;
   int dFcnValueHistoryStartTime;
   QVector<double> dFcnValueHistory;

};

class SDR;


class InputSpace
   : public DataSpace
{
public:
   InputSpace(QString &_id, int _sizeX, int _sizeY, int _numValues, std::vector<PatternInfo*> &_patterns, SDR* _sdrencoder);
   ~InputSpace(void);


   float GenerateSDR(int active_cells, int active_range, float min_value, float max_value, float value);

   // Properties

   int sizeX, sizeY, numValues, testPatterns;
   int rowSize;
   int *data, *buffer;

   std::vector<PatternInfo*> patterns;

   QImage *image;
   QPainter *painter;

   // Methods
   DataSpaceType GetDataSpaceType() { return DATASPACE_TYPE_INPUTSPACE; }

   int GetSizeX();
   int GetSizeY();
   int GetNumValues();
   int GetHypercolumnDiameter();

   bool GetIsActive(int _x, int _y, int _index);
   void SetIsActive(int _x, int _y, int _index, bool _active);
   void DeactivateAll();

   void ApplyPatterns(int _time);
   void ApplyPattern(PatternInfo *_pattern, int _time);

   //QVector<double>* GetPatternValueHistoryVector() { return &patterns[0]->dFcnValueHistory; }
   //int              GetPatternValueStartTime() { return patterns[0]->dFcnValueHistoryStartTime;}

   SDR* dpSdrEncoder;
   float dCurrentInputValue;
};

#include "camProp.h"

// Define global variables
std::vector<camPropInfo> camProps; // Array to store camera properties

camPropInfo get_camPropInfo(HDCAM hdcam, int32 propID)
{
	camPropInfo propInfo;
	DCAMERR err;

	propInfo.propID = propID;
	memset(&propInfo.propAttr, 0, sizeof(propInfo.propAttr));
	propInfo.propAttr.cbSize = sizeof(propInfo.propAttr);
	propInfo.propAttr.iProp = propID;

	dcamprop_getname(hdcam, propID, propInfo.propName, propNameSize);	// get property name
	err = dcamprop_getattr(hdcam, &propInfo.propAttr); // get property attribute

	// Collect prop attribute info
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASCHANNEL)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "HASCHANNEL");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_AUTOROUNDING)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "AUTOROUNDING");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_STEPPING_INCONSISTENT)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "STEPPING_INCONSISTENT");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_DATASTREAM)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "DATASTREAM");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASRATIO)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "HASRATIO");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_VOLATILE)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "VOLATILE");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_WRITABLE)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "WRITABLE");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_READABLE)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "READABLE");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_ACCESSREADY)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "ACCESSREADY");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_ACCESSBUSY)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "ACCESSBUSY");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR_EFFECTIVE)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "EFFECTIVE");

	// Collect prop attribute2 info
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR2_ARRAYBASE)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "ARRAYBASE");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR2_ARRAYELEMENT)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "ARRAYELEMENT");
	if (propInfo.propAttr.attribute & DCAMPROP_ATTR2_INITIALIZEIMPROPER)
		strcpy_s(propInfo.attrNames[propInfo.nAttr++], attrNameSize, "INITIALIZEIMPROPER");

	// Get the datatype of the property
	switch (propInfo.propAttr.attribute & DCAMPROP_TYPE_MASK)
	{
	case DCAMPROP_TYPE_MODE:	strcpy_s(propInfo.dataType, datatypeNameSize, "MODE");	break;
	case DCAMPROP_TYPE_LONG:	strcpy_s(propInfo.dataType, datatypeNameSize, "LONG");	break;
	case DCAMPROP_TYPE_REAL:	strcpy_s(propInfo.dataType, datatypeNameSize, "REAL");	break;
	default:					strcpy_s(propInfo.dataType, datatypeNameSize, "NONE");	break;
	}

	// Extract the range of property values
	propInfo.min = (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASRANGE) ? propInfo.propAttr.valuemin : std::numeric_limits<double>::quiet_NaN();
	propInfo.max = (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASRANGE) ? propInfo.propAttr.valuemax : std::numeric_limits<double>::quiet_NaN();

	// Extract the step size of property value
	propInfo.step = (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASSTEP) ? propInfo.propAttr.valuestep : std::numeric_limits<double>::quiet_NaN();

	// Extract the default value of the property
	propInfo.defaultVal = (propInfo.propAttr.attribute & DCAMPROP_ATTR_HASDEFAULT) ? propInfo.propAttr.valuedefault : std::numeric_limits<double>::quiet_NaN();

	// Extract the current value of the property
	dcamprop_getvalue(hdcam, propID, &propInfo.currentVal);

	// Get all possible values if the datatype is MODE
	if ((propInfo.propAttr.attribute & DCAMPROP_TYPE_MASK) == DCAMPROP_TYPE_MODE) // List possible values for the property
	{
		double lastValue = propInfo.min;
		++propInfo.nSuppVals;
		do {
			DCAMPROP_VALUETEXT pvt;
			memset(&pvt, 0, sizeof(pvt));
			pvt.cbSize = sizeof(pvt);
			pvt.iProp = propID;
			pvt.value = lastValue;
			propInfo.suppVals[propInfo.nSuppVals] = int32(lastValue);
			pvt.text = propInfo.suppValNames[propInfo.nSuppVals++];
			pvt.textbytes = suppValNameSize;
			dcamprop_getvaluetext(hdcam, &pvt);
		} while (!failed(dcamprop_queryvalue(hdcam, propID, &lastValue, DCAMPROP_OPTION_NEXT)));
	}

	// Get the unit of property value
	switch (propInfo.propAttr.iUnit)
	{
	case DCAMPROP_UNIT_SECOND:			strcpy_s(propInfo.unit, unitNameSize, "SECOND");			break;
	case DCAMPROP_UNIT_CELSIUS:			strcpy_s(propInfo.unit, unitNameSize, "CELSIUS");			break;
	case DCAMPROP_UNIT_KELVIN:			strcpy_s(propInfo.unit, unitNameSize, "KELVIN");			break;
	case DCAMPROP_UNIT_METERPERSECOND:	strcpy_s(propInfo.unit, unitNameSize, "METERPERSECOND");	break;
	case DCAMPROP_UNIT_PERSECOND:		strcpy_s(propInfo.unit, unitNameSize, "PERSECOND");			break;
	case DCAMPROP_UNIT_DEGREE:			strcpy_s(propInfo.unit, unitNameSize, "DEGREE");			break;
	case DCAMPROP_UNIT_MICROMETER:		strcpy_s(propInfo.unit, unitNameSize, "MICROMETER");		break;
	default:							strcpy_s(propInfo.unit, unitNameSize, "NONE");				break;
	}

	return propInfo;
}

void fillCamProps(HDCAM hdcam)
{
	camProps.clear(); // Empty out the array
	int32 iProp = 0;	// property IDs
	DCAMPROPOPTION opt = DCAMPROP_OPTION_SUPPORT;
	while (!failed(dcamprop_getnextid(hdcam, &iProp, opt)))
	{
		camPropInfo propInfo = get_camPropInfo(hdcam, iProp);
		camProps.push_back(propInfo);
		if (propInfo.propAttr.attribute2 & DCAMPROP_ATTR2_ARRAYBASE)
		{
			double nElem;
			int32 iPropElemStep = propInfo.propAttr.iPropStep_Element;
			dcamprop_getvalue(hdcam, propInfo.propAttr.iProp_NumberOfElement, &nElem);
			for (int32 i = 1; i < int32(nElem); ++i)
			{
				camPropInfo propElemInfo = get_camPropInfo(hdcam, iProp + i * iPropElemStep);
				camProps.push_back(propElemInfo);
			}
		}
	}
}

void printCamPropsArray(std::ostream& out)
{
	out << std::setw(102) << std::setfill('-') << "" << std::endl << std::setfill(' ');
	out << "| " << std::right << std::setw(11) << "Property ID" << " | ";
	out << std::left << std::setw(propNameSize) << "Property Name" << " | ";
	out << std::right << std::setw(datatypeNameSize) << "Datatype" << " | ";
	out << std::right << std::setw(11) << "Value" << " | ";
	out << std::right << std::setw(unitNameSize) << "Unit" << " |" << std::endl;
	out << std::setw(102) << std::setfill('-') << "" << std::endl << std::setfill(' ');

	for (size_t i = 0; i < camProps.size(); ++i)
	{
		out << "| " << std::right << std::setw(11) << camProps[i].propID << " | ";
		out << std::left << std::setw(propNameSize) << camProps[i].propName << " | ";
		out << std::right << std::setw(datatypeNameSize) << camProps[i].dataType << " | ";
		if (trunc(camProps[i].currentVal) == camProps[i].currentVal)
			out << std::right << std::setw(11) << camProps[i].currentVal << " | ";
		else
			out << std::right << std::setw(11) << std::setprecision(4) << std::scientific << camProps[i].currentVal << std::defaultfloat << " | ";
		out << std::right << std::setw(unitNameSize) << camProps[i].unit << " |" << std::endl;
	}
	out << std::setw(102) << std::setfill('-') << "" << std::endl << std::setfill(' ');
}

void printCamPropInfo(std::ostream& out, size_t camPropsIdx)
{
	// Make sure that the idx is within the array size
	if (camPropsIdx < camProps.size())
	{
		out << "\tID: " << camProps[camPropsIdx].propID << std::endl; // Print out the ID of the property
		out << "\tName: " << camProps[camPropsIdx].propName << std::endl; // Print out the name of the property
		out << "\tAttributes: " << std::endl; // Print out the property attributes
		for (int i = 0; i < camProps[camPropsIdx].nAttr; ++i)
			out << "\t\t" << camProps[camPropsIdx].attrNames[i] << std::endl;
		out << "\tDatatype: " << camProps[camPropsIdx].dataType << std::endl; // Print out the property datatype
		if (camProps[camPropsIdx].nSuppVals >= 0) // List possible values for the property
		{
			out << "\tSupported Values: " << std::endl;
			for (int i = 0; i < camProps[camPropsIdx].nSuppVals; ++i)
				out << "\t\t" << std::setw(8) << std::left << camProps[camPropsIdx].suppVals[i] << " " << camProps[camPropsIdx].suppValNames[i] << std::endl;
		}
		if (!std::isnan(camProps[camPropsIdx].min)) // Print out the minimum possible property values
			out << "\tMinimum: " << camProps[camPropsIdx].min << std::endl;
		if (!std::isnan(camProps[camPropsIdx].max)) // Print out the maximum possible property values
			out << "\tMaximum: " << camProps[camPropsIdx].max << std::endl;
		if (!std::isnan(camProps[camPropsIdx].step)) // Print out the step size of property values
			out << "\tStep: " << camProps[camPropsIdx].step << std::endl;
		if (!std::isnan(camProps[camPropsIdx].defaultVal)) // Print out the default property value
			out << "\tDefault Value: " << camProps[camPropsIdx].defaultVal << std::endl;
		if (!std::isnan(camProps[camPropsIdx].currentVal)) // Print out current property value
			out << "\tCurrent Value: " << camProps[camPropsIdx].currentVal << std::endl;
	}
	else
		out << "Invalid Camera Property Index. Update the camera property list to make sure that the property exists." << std::endl;
}

int getCamPropsIdxByName(std::string& propName)
{
	int propListIdx = -1;
	for (int i = 0; i < camProps.size(); ++i)
		if (propName.compare(camProps[i].propName) == 0)
		{
			propListIdx = i;
			break;
		}
	return propListIdx;
}

int getCamPropsIdxByID(int32 propID)
{
	int propListIdx = -1;
	for (int i = 0; i < camProps.size(); ++i)
		if (camProps[i].propID == propID)
		{
			propListIdx = i;
			break;
		}
	return propListIdx;
}

void setCamPropValue(HDCAM hdcam, std::ostream& out, size_t camPropsIdx, double val)
{
	DCAMERR err = DCAMERR_SUCCESS;
	double reqVal = val;
	do
	{
		val = reqVal;
		err = dcamprop_setgetvalue(hdcam, camProps[camPropsIdx].propID, &val);
		fillCamProps(hdcam);
	} while (err == DCAMERR_SUCCESS && camProps[camPropsIdx].currentVal != val);

	if (failed(err))
	{
		out << "Failed setting " << camProps[camPropsIdx].propName << " = " << reqVal << std::endl
			<< "Make sure that the value provided is valid." << std::endl;
		camProps[camPropsIdx] = get_camPropInfo(hdcam, camProps[camPropsIdx].propID);
		printCamPropInfo(out, camPropsIdx);
		return;
	}
}
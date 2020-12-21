/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#include "glimBatch.h"

namespace NS_GLIM
{

	void GLIM_BATCH::Attribute1f (GLIM_ATTRIBUTE Data, float a1)
	{
		Data.m_CurrentValue[0].Float = a1;

		const GLIM_ENUM Type = GLIM_1F;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::glimAttribute1f: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute1f: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;

	}

	void GLIM_BATCH::Attribute2f (GLIM_ATTRIBUTE Data, float a1, float a2)
	{
		Data.m_CurrentValue[0].Float = a1;
		Data.m_CurrentValue[1].Float = a2;

		const GLIM_ENUM Type = GLIM_2F;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute2f: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute2f: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}

	void GLIM_BATCH::Attribute3f (GLIM_ATTRIBUTE Data, float a1, float a2, float a3)
	{
		Data.m_CurrentValue[0].Float = a1;
		Data.m_CurrentValue[1].Float = a2;
		Data.m_CurrentValue[2].Float = a3;

		const GLIM_ENUM Type = GLIM_3F;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute3f: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute3f: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}

	void GLIM_BATCH::Attribute4f (GLIM_ATTRIBUTE Data, float a1, float a2, float a3, float a4)
	{
		Data.m_CurrentValue[0].Float = a1;
		Data.m_CurrentValue[1].Float = a2;
		Data.m_CurrentValue[2].Float = a3;
		Data.m_CurrentValue[3].Float = a4;

		const GLIM_ENUM Type = GLIM_4F;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute4f: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute4f: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}


	void GLIM_BATCH::Attribute1i (GLIM_ATTRIBUTE Data, int a1)
	{
		Data.m_CurrentValue[0].Int = a1;

		const GLIM_ENUM Type = GLIM_1I;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute1i: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute1i: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}

	void GLIM_BATCH::Attribute2i (GLIM_ATTRIBUTE Data, int a1, int a2)
	{
		Data.m_CurrentValue[0].Int = a1;
		Data.m_CurrentValue[1].Int = a2;

		const GLIM_ENUM Type = GLIM_2I;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute2i: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute2i: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}

	void GLIM_BATCH::Attribute3i (GLIM_ATTRIBUTE Data, int a1, int a2, int a3)
	{
		Data.m_CurrentValue[0].Int = a1;
		Data.m_CurrentValue[1].Int = a2;
		Data.m_CurrentValue[2].Int = a3;

		const GLIM_ENUM Type = GLIM_3I;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute3i: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute3i: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}

	void GLIM_BATCH::Attribute4i (GLIM_ATTRIBUTE Data, int a1, int a2, int a3, int a4)
	{
		Data.m_CurrentValue[0].Int = a1;
		Data.m_CurrentValue[1].Int = a2;
		Data.m_CurrentValue[2].Int = a3;
		Data.m_CurrentValue[3].Int = a4;

		const GLIM_ENUM Type = GLIM_4I;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute4i: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute4i: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}


	void GLIM_BATCH::Attribute4ub (GLIM_ATTRIBUTE Data, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4)
	{
		Data.m_CurrentValue[0].Bytes[0] = a1;
		Data.m_CurrentValue[0].Bytes[1] = a2;
		Data.m_CurrentValue[0].Bytes[2] = a3;
		Data.m_CurrentValue[0].Bytes[3] = a4;

		const GLIM_ENUM Type = GLIM_4UB;

		// Check that either the batch is just about to be created, or the attribute has already been mentioned before.
		GLIM_CHECK ((m_Data->m_State == STATE_BEGINNING_BATCH) || (Data.m_DataType != GLIM_NODATA), "GLIM::Attribute4ub: All to-be-used attributes need to be set to default values, after BeginBatch and before Begin is called.");

		// check that the type matches
		GLIM_CHECK ((Data.m_DataType == Type) || (Data.m_DataType == GLIM_NODATA), "GLIM::glimAttribute4ub: An attribute was used with different type than previously.");
		// set the type
		Data.m_DataType = Type;
	}


	GLIM_ATTRIBUTE GLIM_BATCH::Attribute1f (const char* szAttribute, float a1)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute1f (Data, a1);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute2f (const char* szAttribute, float a1, float a2)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute2f (Data, a1, a2);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute3f (const char* szAttribute, float a1, float a2, float a3)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute3f (Data, a1, a2, a3);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute4f (const char* szAttribute, float a1, float a2, float a3, float a4)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute4f (Data, a1, a2, a3, a4);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute1i (const char* szAttribute, int a1)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute1i (Data, a1);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute2i (const char* szAttribute, int a1, int a2)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute2i (Data, a1, a2);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute3i (const char* szAttribute, int a1, int a2, int a3)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute3i (Data, a1, a2, a3);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute4i (const char* szAttribute, int a1, int a2, int a3, int a4)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute4i (Data, a1, a2, a3, a4);

		return (Data);
	}

	GLIM_ATTRIBUTE GLIM_BATCH::Attribute4ub (const char* szAttribute, unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4)
	{
		GlimArrayData& Data = m_Data->m_Attributes[szAttribute];

		Attribute4ub (Data, a1, a2, a3, a4);

		return (Data);
	}


}


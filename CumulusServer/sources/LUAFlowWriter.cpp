/* 
	Copyright 2010 OpenRTMFP
 
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License received along this program for more
	details (or else see http://www.gnu.org/licenses/).

	This file is a part of Cumulus.
*/

#include "LUAFlowWriter.h"
#include "FlowWriter.h"
#include "Service.h"

using namespace std;
using namespace Cumulus;
using namespace Poco;

class NewWriter : public FlowWriter {
public:
	NewWriter(const string& signature,BandWriter& band) : FlowWriter(signature,band),pState(NULL) {
	}
	virtual ~NewWriter(){
		Script::ClearPersistentObject<NewWriter,LUAFlowWriter>(pState,*this);
	}
	void manage(Invoker& invoker){
		if(!closed()) {
			SCRIPT_BEGIN(pState)
				SCRIPT_MEMBER_FUNCTION_BEGIN(NewWriter,LUAFlowWriter,*this,"onManage")
					SCRIPT_FUNCTION_CALL
				SCRIPT_FUNCTION_END
			SCRIPT_END
		}
		FlowWriter::manage(invoker);
	}
	lua_State*			pState;
};

const char*		LUAFlowWriter::Name="Cumulus::FlowWriter";


int LUAFlowWriter::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(NewWriter,LUAFlowWriter,writer)
		writer.close();
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::Close(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		writer.close();
	SCRIPT_CALLBACK_RETURN
}


int LUAFlowWriter::Get(lua_State *pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		string name = SCRIPT_READ_STRING("");
		if(name=="reliable") {
			SCRIPT_WRITE_BOOL(writer.reliable)
		} else if(name=="flush") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::Flush)
		} else if(name=="writeAMFResult") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::WriteAMFResult)
		} else if(name=="writeAMFMessage") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::WriteAMFMessage)
		} else if(name=="writeStatusResponse") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::WriteStatusResponse)
		} else if(name=="newFlowWriter") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::NewFlowWriter)
		} else if(name=="close") {
			SCRIPT_WRITE_FUNCTION(&LUAFlowWriter::Close)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::Set(lua_State *pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		string name = SCRIPT_READ_STRING("");
		if(name=="reliable")
			writer.reliable = lua_toboolean(pState,-1)==0 ? false : true;
		else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::Flush(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		writer.flush(SCRIPT_READ_BOOL(false));
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::WriteAMFResult(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		AMFWriter& amf = writer.writeAMFResult();
		SCRIPT_READ_AMF(amf)
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::WriteAMFMessage(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		string name = SCRIPT_READ_STRING("");
		AMFWriter& amf = writer.writeAMFMessage(name);
		SCRIPT_READ_AMF(amf)
	SCRIPT_CALLBACK_RETURN
}

int LUAFlowWriter::WriteStatusResponse(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		string code = SCRIPT_READ_STRING("");
		AMFObjectWriter response = writer.writeStatusResponse(code,SCRIPT_READ_STRING(""));
		while(SCRIPT_CAN_READ) {
			response.writer.writePropertyName(SCRIPT_READ_STRING(""));
			Script::ReadAMF(pState,response.writer,1);
		}
	SCRIPT_CALLBACK_RETURN
}


int LUAFlowWriter::NewFlowWriter(lua_State* pState) {
	SCRIPT_CALLBACK(FlowWriter,LUAFlowWriter,writer)
		NewWriter& newWriter = writer.newFlowWriter<NewWriter>();
		newWriter.pState = pState;
		SCRIPT_WRITE_PERSISTENT_OBJECT(NewWriter,LUAFlowWriter,newWriter)
		SCRIPT_ADD_DESTRUCTOR(&LUAFlowWriter::Destroy)
	SCRIPT_CALLBACK_RETURN
}

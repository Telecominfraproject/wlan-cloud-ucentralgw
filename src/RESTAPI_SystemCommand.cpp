//
// Created by stephane bourque on 2021-04-11.
//

#include "RESTAPI_SystemCommand.h"

#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "uCentral.h"

/*

    SystemCommandDetails:
      type: object
      properties:
        command:
          type: string
          enum:
            - setloglevel
            - getloglevel
            - stats
        parameters:
          type: array
          items:
            properties:
              name:
                type: string
              value:
                type: string

    SystemCommandResults:
      type: object
      properties:
        command:
          type: string
        result:
          type: integer
        resultTxt:
          type: array
          items:
            type: string

 */
void RESTAPI_SystemCommand::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {

	if(!ContinueProcessing(Request,Response))
		return;

	if(!IsAuthorized(Request,Response))
		return;

	try {

		if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_POST) {
			Poco::JSON::Parser parser;
			auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *Obj;

			if(ds.contains("command")) {
				auto Command = ds["command"].toString();
				if(Command=="setloglevel") {
					if(ds.contains("parameters")) {
						auto ParametersBlock = ds["parameters"];
						if(ParametersBlock.isArray())
						{
							for(const auto &i : ParametersBlock)
							{
								if(i.isStruct()) {
									auto O = i.toString();
									Poco::JSON::Parser	pp;

									auto TLV = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
									Poco::DynamicStruct Vars = *TLV;
									if (Vars.contains("name") && Vars.contains("value")) {
										auto Name = Vars["name"].toString();
										auto Value = Vars["value"].toString();
										uCentral::Daemon::SetSubsystemLogLevel(Name,Value);
										Logger_.information(Poco::format("Setting log level for %s at %s",Name,Value));
									}
								}
							}
							OK(Response);
							return;
						}
					}
				} else if (Command=="getloglevel") {

				} else if (Command=="stats") {

				}
			}
		}
	} catch ( const Poco::Exception & E ) {
		Logger_.log(E);
	}

	BadRequest(Response);
}

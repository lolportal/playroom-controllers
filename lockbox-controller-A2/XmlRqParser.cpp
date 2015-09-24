// 
// 
// 

#include "XmlRqParser.h"
#include "lockbox-controller-rq-base-protocol.h"
#include "lockbox-controller-rq-valid-protocol-values.h"

XmlRqParser::XmlRqParser()
{
}

XmlRqParser::ErrorType XmlRqParser::process_stream(Stream& s, XmlRqParsingOutput* out)
{
	if (!out)
		return ERROR_OUTPUT_NULLPTR;

	bool ok = token_parser.expect_opening_tag(s, RQ_TAG_NAME);

	if (!ok)
		return ERROR_RQ_TAG_NAME;

	ok = token_parser.find_attribute(s, TYPE_ATTR_NAME, MAX_RQ_TYPE_ATTR_VALUE_LENGTH);
	if (!ok)
		return ERROR_TYPE_ATTR_VALUE;

	out->type_attr_found = true;
	token_parser.copy_buf_to_attr_buf(out->type_attr_buf, sizeof(out->type_attr_buf));
	
	ok = token_parser.find_attribute(s, PROTOVER_ATTR_NAME, MAX_PROTO_ATTR_VALUE_LENGTH);
	if (!ok)
		return ERROR_PROTOVER_ATTR_VALUE;

	out->proto_attr_found = true;
	token_parser.copy_buf_to_attr_buf(out->proto_attr_buf, sizeof(out->proto_attr_buf));

	ok = token_parser.find_attribute(s, SERIAL_ATTR_NAME, MAX_SERIAL_ATTR_VALUE_LENGTH);
	if (!ok)
		return ERROR_SERIAL_ATTR_VALUE;

	out->serial_attr_found = true;
	token_parser.copy_buf_to_attr_buf(out->serial_attr_buf, sizeof(out->serial_attr_buf));

	ok = token_parser.expect_right_simple_bracket(s);
	if (!ok)
		return ERROR_CLOSE_TAG;

	ok = token_parser.expect_opening_tag(s, DATA_TAG_NAME);
	if (!ok)
		return ERROR_DATA_TAG_NAME;

	ok = token_parser.find_attribute(s, LOCKBOX_ATTR_NAME, MAX_LOCKBOX_ATTR_VALUE_LENGTH);
	if (!ok)
		return ERROR_LOCKBOX_ATTR_VALUE;
	out->lockbox_attr_found = true;
	token_parser.copy_buf_to_attr_buf(out->lockbox_attr_buf, sizeof(out->lockbox_attr_buf));

	ok = token_parser.expect_right_closing_bracket(s);
	if (!ok)
		return ERROR_CLOSE_TAG;

	ok = token_parser.expect_closing_tag(s, RQ_TAG_NAME);
	if (!ok)
		return ERROR_RQ_CLOSING_TAG_NAME;
	
	return ERROR_NONE;
}


